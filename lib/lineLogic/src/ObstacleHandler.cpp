#include "ObstacleHandler.h"
#include "../config.h"
#include <tofManager.h>
#include <sensorBoard.h>
#include <motori.h>
#include <followLine.h>
#include <debug.h>
#include <Arduino.h>

extern tofManager tof_manager;
extern BottomSensor IR_board;
extern Motori motori;
extern Debug debug;

// ── Accumulatori per media distanze laterali durante retromarcia ──
static uint32_t _sumAntSX;
static uint32_t _sumAntDX;
static uint16_t _sampleCount;
static uint8_t  _obstacleConsecutive = 0;  // contatore debounce

// ═════════════════════════════════════════════════════════════════

bool obstacleDetected() {
    const uint16_t d = tof_manager.front.getDistance();
    // Scarta letture invalide: 0 = errore sensore, >8000 = fuori range
    if (d == 0 || d > 8000) {
        _obstacleConsecutive = 0;
        return false;
    }
    if (d < OBSTACLE_DETECT_MM) {
        if (_obstacleConsecutive < 255) _obstacleConsecutive++;
        return _obstacleConsecutive >= OBSTACLE_CONFIRM_READS;
    }
    _obstacleConsecutive = 0;
    return false;
}

void obstacleHandler_enter(RobotContext& ctx) {
    ctx.state = STATE_OBSTACLE_BACK;
    ctx.stateStartMs = millis();
    ctx.obstacleBackStartMs = millis();
    _sumAntSX   = 0;
    _sumAntDX   = 0;
    _sampleCount = 0;
    debug.println("-> OBSTACLE_BACK");
}

// ── Conta sensori attivi (stessa logica di LineRecovery) ─────────
static uint8_t _countActive() {
    const uint16_t* vals = IR_board.utils.val_sensor();
    uint8_t c = 0;
    for (uint8_t i = 0; i < 8; i++) {
        if (vals[i] > 500) c++;
    }
    return c;
}

void obstacleHandler_update(RobotContext& ctx) {
    const uint32_t now = millis();

    switch (ctx.state) {

    // ── BACK: retromarcia leggendo laterali ─────────────────────
    case STATE_OBSTACLE_BACK: {
        motori.muovi(OBSTACLE_BACK_VEL, 0);

        // Accumulo letture laterali per media
        _sumAntSX += tof_manager.antSX.getDistance();
        _sumAntDX += tof_manager.antDX.getDistance();
        _sampleCount++;

        if ((int32_t)(now - ctx.stateStartMs) >= (int32_t)OBSTACLE_BACK_MS) {
            ctx.state = STATE_OBSTACLE_CHOOSE_SIDE;
            ctx.stateStartMs = now;
            debug.println("-> OBSTACLE_CHOOSE_SIDE");
        }
        break;
    }

    // ── CHOOSE_SIDE: decidi da che lato aggirare ────────────────
    case STATE_OBSTACLE_CHOOSE_SIDE: {
        motori.stop();

        uint16_t avgSX = (_sampleCount > 0) ? (uint16_t)(_sumAntSX / _sampleCount) : 0;
        uint16_t avgDX = (_sampleCount > 0) ? (uint16_t)(_sumAntDX / _sampleCount) : 0;

        const bool sxClear = (avgSX > SIDE_CLEAR_MM);
        const bool dxClear = (avgDX > SIDE_CLEAR_MM);

        if (sxClear && dxClear) {
            ctx.obstacleBypassLeft = OBSTACLE_DEFAULT_LEFT;
        } else if (sxClear) {
            ctx.obstacleBypassLeft = true;
        } else if (dxClear) {
            ctx.obstacleBypassLeft = false;
        } else {
            // Nessun lato chiaro, scegli il maggiore
            ctx.obstacleBypassLeft = (avgSX > avgDX);
        }

        ctx.state = STATE_OBSTACLE_BYPASS;
        ctx.stateStartMs = now;
        debug.print("-> OBSTACLE_BYPASS ");
        debug.println(ctx.obstacleBypassLeft ? "SX" : "DX");
        break;
    }

    // ── BYPASS: aggira l'ostacolo ───────────────────────────────
    case STATE_OBSTACLE_BYPASS: {
        // Steer nel verso scelto e avanza
        const short steerSign = ctx.obstacleBypassLeft ? -1 : 1;
        motori.muovi(OBSTACLE_BYPASS_VEL,
                     (short)(steerSign * OBSTACLE_STEER_ANG));

        // Se i sensori IR vedono linea → vai a REALIGN
        if (_countActive() >= CENTER_MIN_SENSORS) {
            ctx.state = STATE_OBSTACLE_REALIGN;
            ctx.stateStartMs = now;
            debug.println("-> OBSTACLE_REALIGN");
            break;
        }

        // Timeout
        if ((int32_t)(now - ctx.stateStartMs) >= (int32_t)OBSTACLE_BYPASS_MS) {
            ctx.state = STATE_OBSTACLE_REALIGN;
            ctx.stateStartMs = now;
            debug.println("-> OBSTACLE_REALIGN (timeout)");
        }
        break;
    }

    // ── REALIGN: riallinea sulla linea ──────────────────────────
    case STATE_OBSTACLE_REALIGN: {
        // Sterza nella direzione che NON inverte il senso di marcia
        const short realignAng = ctx.lastTurnWasLeft ? TURN_LEFT_ANG
                                                     : TURN_RIGHT_ANG;
        motori.muovi(SEARCH_VEL, realignAng);

        if (_countActive() >= CENTER_MIN_SENSORS) {
            const int16_t pos = IR_board.line();
            const int16_t absPos = (pos < 0) ? -pos : pos;
            if (absPos < CENTER_THRESHOLD) {
                resetPID();
                ctx.state = STATE_FOLLOWING;
                ctx.stateStartMs = now;
                debug.println("-> FOLLOWING (realign ok)");
            }
        }
        break;
    }

    default:
        break;
    }
}
