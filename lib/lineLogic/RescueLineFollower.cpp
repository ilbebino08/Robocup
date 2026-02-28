#include "RescueLineFollower.h"
#include <sensorBoard.h>
#include <motori.h>
#include <tofManager.h>
#include <followLine.h>
#include <debug.h>
#include <Arduino.h>
#include <string.h>  // memset

extern BottomSensor IR_board;
extern Motori motori;
extern tofManager tof_manager;
extern Debug debug;

// ═══════════════════════════════════════════════════════════════════
//  Inizializzazione
// ═══════════════════════════════════════════════════════════════════

void RescueLineFollower::begin() {
    memset(&_ctx, 0, sizeof(_ctx));
    _ctx.state = STATE_FOLLOWING;
    resetPID();
}

// ═══════════════════════════════════════════════════════════════════
//  Transizione di stato unificata — logga la label
// ═══════════════════════════════════════════════════════════════════

void RescueLineFollower::_transitionTo(FSMState newState, const char* label) {
    debug.print("-> ");
    debug.println(label);
    _ctx.state = newState;
    _ctx.stateStartMs = millis();
    if (newState == STATE_FOLLOWING) {
        resetPID();
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Gestione svolta 90° (verde singolo)
// ═══════════════════════════════════════════════════════════════════

void RescueLineFollower::_handleTurn(bool isLeft) {
    _ctx.state = isLeft ? STATE_TURN_LEFT : STATE_TURN_RIGHT;
    _ctx.stateStartMs = millis();
    _ctx.lastTurnWasLeft = isLeft;
    greenManager_reset(_ctx);
    greenManager_setIgnoreAfterTurn(_ctx, isLeft);
    debug.println(isLeft ? "-> TURN_LEFT" : "-> TURN_RIGHT");
}

// ═══════════════════════════════════════════════════════════════════
//  Logica del STATE_FOLLOWING — valuta priorità eventi
// ═══════════════════════════════════════════════════════════════════

void RescueLineFollower::_handleFollowing(int16_t pos, uint32_t now) {
    const GreenEvent ge = greenManager_getEvent(_ctx);

    // 1. Ostacolo (massima priorità)
    if (obstacleDetected()) {
        obstacleHandler_enter(_ctx);
        return;
    }

    // 2. Doppio verde → U-turn
    if (ge == GREEN_DOUBLE) {
        uTurnHandler_enter(_ctx);
        return;
    }

    // 3. Verde singolo SX
    if (ge == GREEN_SINGLE_SX) {
        _handleTurn(true);
        return;
    }

    // 4. Verde singolo DX
    if (ge == GREEN_SINGLE_DX) {
        _handleTurn(false);
        return;
    }

    // 5. Verde WAITING → non fare nulla di speciale, continua PID
    //    (la finestra doppio-verde è ancora aperta)

    // 6. Incrocio (solo se nessun verde attivo)
    if (ge == GREEN_NONE && intersectionDetected(_ctx, now)) {
        intersectionHandler_enter(_ctx);
        return;
    }

    // 7. Linea persa — abs(pos)==1750 per LINE_LOST_CONFIRM_MS
    const int16_t absPos = (pos < 0) ? -pos : pos;
    if (absPos >= 1750) {
        if (_ctx.lineLostStartMs == 0) {
            _ctx.lineLostStartMs = now;
        } else if ((now - _ctx.lineLostStartMs) >= LINE_LOST_CONFIRM_MS) {
            lineRecovery_enter(_ctx);
            return;
        }
    } else {
        _ctx.lineLostStartMs = 0;
    }

    // 8. Following normale
    const short ang = pidLineFollowing(BASE_VEL, pos);
    motori.muovi(BASE_VEL, ang);
}

// ═══════════════════════════════════════════════════════════════════
//  Update principale — chiamare una volta per loop()
// ═══════════════════════════════════════════════════════════════════

void RescueLineFollower::update() {
    const uint32_t now = millis();

    // 1. Aggiorna TOF
    tof_manager.refreshAll();

    // 2. Aggiorna verde (legge checkGreenSx/Dx UNA volta)
    greenManager_update(_ctx);

    // 3. Leggi line() UNA sola volta
    const int16_t pos = IR_board.line();

    // Aggiorna lastLinePos solo se la scheda non ha congelato il valore
    if (pos > -1750 && pos < 1750) {
        _ctx.lastLinePos = pos;
    }

    // 4-5. Smista al gestore corretto
    switch (_ctx.state) {

    case STATE_FOLLOWING:
        _handleFollowing(pos, now);
        break;

    // ── Svolte 90° ──────────────────────────────────────────────
    case STATE_TURN_LEFT:
    case STATE_TURN_RIGHT: {
        const bool isLeft = (_ctx.state == STATE_TURN_LEFT);
        const short ang = isLeft ? TURN_LEFT_ANG : TURN_RIGHT_ANG;
        motori.muovi(TURN_SLOW_VEL, ang);

        // Completamento: line() centrata dopo che il segno è cambiato
        const int16_t absP = (pos < 0) ? -pos : pos;
        if (absP < TURN_CENTER_THRESHOLD &&
            (now - _ctx.stateStartMs) > 200) {  // min 200ms per evitare exit prematura
            _transitionTo(STATE_FOLLOWING, "FOLLOWING (turn ok)");
        }

        // Timeout
        if ((now - _ctx.stateStartMs) >= TURN_TIMEOUT_MS) {
            _transitionTo(STATE_FOLLOWING, "FOLLOWING (turn timeout)");
        }
        break;
    }

    // ── U-Turn ──────────────────────────────────────────────────
    case STATE_UTURN:
        uTurnHandler_update(_ctx);
        break;

    // ── Incrocio ────────────────────────────────────────────────
    case STATE_INTERSECTION:
        intersectionHandler_update(_ctx);
        break;

    // ── Line lost (3 sotto-stati) ───────────────────────────────
    case STATE_LINE_LOST_REVERSE:
    case STATE_LINE_LOST_CENTER:
    case STATE_LINE_LOST_FORWARD:
        lineRecovery_update(_ctx);
        break;

    // ── Ostacolo (4 sotto-stati) ────────────────────────────────
    case STATE_OBSTACLE_BACK:
    case STATE_OBSTACLE_CHOOSE_SIDE:
    case STATE_OBSTACLE_BYPASS:
    case STATE_OBSTACLE_REALIGN:
        obstacleHandler_update(_ctx);
        break;

    default:
        break;
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Robot rimesso in pista dall'arbitro — reset totale
// ═══════════════════════════════════════════════════════════════════

void RescueLineFollower::onRobotReplaced() {
    memset(&_ctx, 0, sizeof(_ctx));
    _ctx.state = STATE_FOLLOWING;
    resetPID();
    motori.stop();
    debug.println("ROBOT REPLACED - RESET");
}
