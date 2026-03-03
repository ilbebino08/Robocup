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
static uint32_t _recoveryOriginMs;
static bool     _curveDetected;     // true se un sensore esterno ha visto la linea
static bool     _curveIsLeft;       // true = curva a sinistra

void lineRecovery_enter(RobotContext& ctx) {
    ctx.state = STATE_LINE_LOST_REVERSE;
    ctx.stateStartMs = millis();
    _recoveryOriginMs = millis();
    _curveDetected = false;
    _curveIsLeft   = false;
    LL_LOG2("[RV] enter lastPos=", (int)ctx.lastLinePos);
}

void lineRecovery_update(RobotContext& ctx) {
    const uint32_t now = millis();

    // ── Timeout assoluto globale ────────────────────────────────
    if ((now - _recoveryOriginMs) >= SEARCH_TOTAL_MS) {
        motori.stop();
        LL_LOG("RECOVERY TIMEOUT");
        return;
    }

    switch (ctx.state) {

    // ── REVERSE: retromarcia lenta, guarda sensori esterni ──────
    case STATE_LINE_LOST_REVERSE: {
        motori.muovi(REVERSE_VEL, 0);  // indietro dritto, piano

        // Leggi i sensori più esterni (0 = tutto a SX, 7 = tutto a DX)
        const uint16_t* vals = IR_board.utils.val_sensor();
        const bool leftEdge  = (vals[0] > 500);
        const bool rightEdge = (vals[7] > 500);

        LL_LOG6("[RV] L=", (int)vals[0], " R=", (int)vals[7],
                " t=", (int)(now - ctx.stateStartMs));

        if (leftEdge || rightEdge) {
            // Sensore esterno ha trovato la linea → è una curva a gomito
            _curveDetected = true;
            _curveIsLeft   = leftEdge;
            ctx.state = STATE_LINE_LOST_CENTER;
            ctx.stateStartMs = now;
            LL_LOG2("[RV] curva trovata lato=", _curveIsLeft ? "SX" : "DX");
            break;
        }

        // Timeout retromarcia → sterza nella direzione dell'ultima pos nota
        if ((now - ctx.stateStartMs) >= REVERSE_SEARCH_MS) {
            _curveDetected = true;
            _curveIsLeft   = (ctx.lastLinePos < 0);
            ctx.state = STATE_LINE_LOST_CENTER;
            ctx.stateStartMs = now;
            LL_LOG("[RV] timeout -> CENTER");
        }
        break;
    }

    // ── CENTER: sterza verso il lato della curva fino a checkLinea ──
    case STATE_LINE_LOST_CENTER: {
        // Sterza verso il lato dove il sensore esterno ha visto la linea
        const short steerAng = _curveIsLeft ? TURN_LEFT_ANG : TURN_RIGHT_ANG;
        motori.muovi(SEARCH_VEL, steerAng);

        const int16_t pos = IR_board.line();
        LL_LOG4("[CT] pos=", (int)pos, " steer=", (int)steerAng);

        // Linea ritrovata al centro → esci dal recovery
        if (IR_board.checkLinea() && pos > -1750 && pos < 1750 &&
            ((pos < 0 ? -pos : pos) < CENTER_THRESHOLD)) {
            resetPID();
            ctx.lineLostStartMs = 0;
            ctx.state = STATE_FOLLOWING;
            ctx.stateStartMs = now;
            LL_LOG("[CT] -> FOLLOWING (recovery ok)");
        }
        break;
    }

    // ── FORWARD: non usato in questo schema, fallback ───────────
    case STATE_LINE_LOST_FORWARD: {
        resetPID();
        ctx.lineLostStartMs = 0;
        ctx.state = STATE_FOLLOWING;
        ctx.stateStartMs = now;
        break;
    }

    default:
        break;
    }
}
