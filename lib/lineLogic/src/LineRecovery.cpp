#include "LineRecovery.h"
#include "../config.h"
#include <sensorBoard.h>
#include <motori.h>
#include <followLine.h>
#include <debug.h>
#include <Arduino.h>

extern BottomSensor IR_board;
extern Motori motori;
extern Debug debug;

// ═══════════════════════════════════════════════════════════════════
//  Variabili locali al modulo
// ═══════════════════════════════════════════════════════════════════
static uint32_t _recoveryOriginMs;  // timestamp ingresso nel primo stato REVERSE

void lineRecovery_enter(RobotContext& ctx) {
    ctx.state = STATE_LINE_LOST_REVERSE;
    ctx.stateStartMs = millis();
    _recoveryOriginMs = millis();
    debug.println("-> LINE_LOST_REVERSE");
}

// ── Conta quanti sensori laterali dell'array vedono qualcosa ─────
static uint8_t _countActiveSensors() {
    const uint16_t* vals = IR_board.utils.val_sensor();
    uint8_t count = 0;
    for (uint8_t i = 0; i < 8; i++) {
        // Sensori calibrati: valore alto = linea rilevata
        if (vals[i] > 500) count++;
    }
    return count;
}

void lineRecovery_update(RobotContext& ctx) {
    const uint32_t now = millis();

    // ── Timeout assoluto globale ────────────────────────────────
    if ((now - _recoveryOriginMs) >= SEARCH_TOTAL_MS) {
        motori.stop();
        debug.println("RECOVERY TIMEOUT");
        return;  // resta fermo nello stato corrente
    }

    switch (ctx.state) {

    // ── REVERSE: retromarcia cercando la linea ──────────────────
    case STATE_LINE_LOST_REVERSE: {
        motori.muovi(REVERSE_VEL, 0);

        // Se almeno CENTER_MIN_SENSORS sensori vedono linea → trovata
        if (_countActiveSensors() >= CENTER_MIN_SENSORS) {
            resetPID();
            ctx.state = STATE_FOLLOWING;
            ctx.stateStartMs = now;
            debug.println("-> FOLLOWING (reverse ok)");
            return;
        }

        // Timeout fase REVERSE → passa a CENTER
        if ((now - ctx.stateStartMs) >= REVERSE_SEARCH_MS) {
            ctx.state = STATE_LINE_LOST_CENTER;
            ctx.stateStartMs = now;
            debug.println("-> LINE_LOST_CENTER");
        }
        break;
    }

    // ── CENTER: sterza verso il lato opposto all'ultima posizione
    case STATE_LINE_LOST_CENTER: {
        // Sterza nella direzione opposta all'ultima posizione nota
        const short steerAng = (ctx.lastLinePos < 0) ? TURN_RIGHT_ANG
                                                       : TURN_LEFT_ANG;
        motori.muovi(0, steerAng);

        const int16_t pos = IR_board.line();
        if (pos > -1750 && pos < 1750 &&
            ((pos < 0 ? -pos : pos) < CENTER_THRESHOLD)) {
            ctx.state = STATE_LINE_LOST_FORWARD;
            ctx.stateStartMs = now;
            debug.println("-> LINE_LOST_FORWARD");
        }
        break;
    }

    // ── FORWARD: avanzamento lento fino a conferma linea ────────
    case STATE_LINE_LOST_FORWARD: {
        motori.muovi(SEARCH_VEL, 0);

        if (_countActiveSensors() >= CENTER_MIN_SENSORS) {
            resetPID();
            ctx.state = STATE_FOLLOWING;
            ctx.stateStartMs = now;
            debug.println("-> FOLLOWING (forward ok)");
        }
        break;
    }

    default:
        break;
    }
}
