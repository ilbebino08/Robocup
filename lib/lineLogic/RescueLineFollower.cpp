#include "RescueLineFollower.h"
#include <sensorBoard.h>
#include <motori.h>
#include <tofManager.h>
#include <followLine.h>
#include <debug.h>
#include <Arduino.h>
#include <string.h>  // memset
#include <MultiClickButton.h>

extern BottomSensor IR_board;
extern Motori motori;
extern tofManager tof_manager;
extern MultiClickButton button;
extern Debug debug;

// ═══════════════════════════════════════════════════════════════════
//  Inizializzazione
// ═══════════════════════════════════════════════════════════════════

void RescueLineFollower::begin() {
    memset(&_ctx, 0, sizeof(_ctx));
    _ctx.state = STATE_FOLLOWING;
    resetPID();
    LL_LOG("[LL] begin() -> FOLLOWING");
}

// ═══════════════════════════════════════════════════════════════════
//  Transizione di stato unificata — logga la label
// ═══════════════════════════════════════════════════════════════════

void RescueLineFollower::_transitionTo(FSMState newState, const char* label) {
    LL_LOG2("[LL] -> ", label);
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
    if (_ctx.state == STATE_TURN_LEFT || _ctx.state == STATE_TURN_RIGHT) return; // Non retriggerare
    _ctx.state = isLeft ? STATE_TURN_LEFT : STATE_TURN_RIGHT;
    _ctx.stateStartMs = millis();
    _ctx.lastTurnWasLeft = isLeft;
    greenManager_reset(_ctx);
    LL_LOG(isLeft ? "[LL] -> TURN_LEFT" : "[LL] -> TURN_RIGHT");
}

// ═══════════════════════════════════════════════════════════════════
//  Logica del STATE_FOLLOWING — valuta priorità eventi
// ═══════════════════════════════════════════════════════════════════

void RescueLineFollower::_handleFollowing(int16_t pos, uint32_t now) {
    const GreenEvent ge = greenManager_getEvent(_ctx);

    // Log periodico ogni ~500ms per non floodare
    static uint32_t _lastLogMs = 0;
    if (LL_DEBUG && (int32_t)(now - _lastLogMs) >= 500) {
        _lastLogMs = now;
        LL_LOG6("[FL] pos=", (int)pos, " ge=", (int)ge,
                " tof=", (int)tof_manager.front.getDistance());
    }

    // Nota: Ostacolo e Verdi (GREEN_DOUBLE, GREEN_SINGLE, GREEN_WAITING) 
    // sono ora gestiti a monte in update() come priorità assoluta.

    // 6. Incrocio (solo se nessun verde attivo)
    if (ge == GREEN_NONE && intersectionDetected(_ctx, now)) {
        LL_LOG("[FL] INTERSECTION detected");
        intersectionHandler_enter(_ctx);
        return;
    }

    // 7. Linea persa — abs(pos)==1750 per LINE_LOST_CONFIRM_MS
    const int16_t absPos = (pos < 0) ? -pos : pos;
    if (absPos >= 1750) {
        if (_ctx.lineLostStartMs == 0) {
            _ctx.lineLostStartMs = now;
            LL_LOG2("[FL] line frozen pos=", (int)pos);
        } else if ((int32_t)(now - _ctx.lineLostStartMs) >= (int32_t)LINE_LOST_CONFIRM_MS) {
            LL_LOG("[FL] LINE LOST -> RECOVERY");
            lineRecovery_enter(_ctx);
            return;
        }
        // Ancora nel debounce: va dritto invece di sterzare a fondo
        motori.muovi(BASE_VEL, 0);
        return;
    } else {
        _ctx.lineLostStartMs = 0;
    }

    // 8. Following normale — legge pos fresca e la passa direttamente al PID
    {
        const int16_t freshPos = IR_board.line();
        if (freshPos == -1750 || freshPos == 1750) {
            // Linea sparita nell'ultimo istante → entra subito in recovery
            LL_LOG("[FL] LINE LOST (fresh) -> RECOVERY");
            lineRecovery_enter(_ctx);
            return;
        }
        pidLineFollowing(BASE_VEL, freshPos);
    }
    
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
    // e NON siamo in recovery (per non sovrascrivere la direzione)
    if (pos > -1750 && pos < 1750 &&
        _ctx.state != STATE_LINE_LOST_REVERSE &&
        _ctx.state != STATE_LINE_LOST_CENTER &&
        _ctx.state != STATE_LINE_LOST_FORWARD) {
        _ctx.lastLinePos = pos;
    }

    // Log stato corrente ogni ~1s
    static uint32_t _lastStateLog = 0;
    if (LL_DEBUG && (int32_t)(now - _lastStateLog) >= 1000) {
        _lastStateLog = now;
        LL_LOG4("[LL] state=", (int)_ctx.state, " pos=", (int)pos);
    }

    // 4. Gestione prioritaria Ostacoli e Verdi
    if (_ctx.state == STATE_FOLLOWING || 
        _ctx.state == STATE_LINE_LOST_REVERSE || 
        _ctx.state == STATE_LINE_LOST_CENTER || 
        _ctx.state == STATE_LINE_LOST_FORWARD) {
        
        // 4.1 Ostacolo (massima priorità)
        if (obstacleDetected()) {
            LL_LOG2("[FL] OBSTACLE tof=", (int)tof_manager.front.getDistance());
            obstacleHandler_enter(_ctx);
            return;
        }

        const GreenEvent ge = greenManager_getEvent(_ctx);
        
        // 4.2 Doppio verde → U-turn
        if (ge == GREEN_DOUBLE) {
            LL_LOG("[FL] GREEN_DOUBLE -> UTURN");
            uTurnHandler_enter(_ctx);
            return;
        }

        // 4.3 Verde singolo SX
        if (ge == GREEN_SINGLE_SX) {
            LL_LOG("[FL] GREEN_SX -> TURN_LEFT");
            // Se eravamo in recovery o ostacolo, va resettato lo stato
            if (_ctx.state != STATE_FOLLOWING) resetPID();
            _handleTurn(true);
            return;
        }

        // 4.4 Verde singolo DX
        if (ge == GREEN_SINGLE_DX) {
            LL_LOG("[FL] GREEN_DX -> TURN_RIGHT");
            // Se eravamo in recovery o ostacolo, va resettato lo stato
            if (_ctx.state != STATE_FOLLOWING) resetPID();
            _handleTurn(false);
            return;
        }

        // 4.5 Verde WAITING → ferma i motori per attendere eventuale secondo verde senza superarlo
        if (ge == GREEN_WAITING) {
            LL_LOG("[FL] GREEN_WAITING -> motori fermi attesa");
            motori.stop();
            return;
        }
    }

    // 5. Smista al gestore corretto
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

        LL_LOG2("[TN] BLOCKING TURN START ", isLeft ? "L" : "R");

        while(true) {
            button.update();
            if (button.isPaused()) {
                motori.stop();
                _transitionTo(STATE_FOLLOWING, "INTERRUPT");
                return; 
            }

            uint32_t current_now = millis();
            
            // Lettura velocissima (bloccante e dedicata solo alla curva)
            IR_board.line();
            bool lineDetected = IR_board.checkLinea();
            
            // Fuga dalla linea originale
            if (lineDetected && (int32_t)(current_now - _ctx.stateStartMs) > 150) {
                greenManager_setIgnoreAfterTurn(_ctx, isLeft);
                LL_LOG("[TN] turn ok line found!");
                _transitionTo(STATE_FOLLOWING, "FOLLOWING (turn ok)");
                break;
            }

            // Timeout d'emergenza
            if ((int32_t)(current_now - _ctx.stateStartMs) >= (int32_t)TURN_TIMEOUT_MS) {
                greenManager_setIgnoreAfterTurn(_ctx, isLeft);
                LL_LOG("[TN] turn timeout!");
                _transitionTo(STATE_FOLLOWING, "FOLLOWING (turn timeout)");
                break;
            }
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
        lineRecovery_update(_ctx, pos);
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
