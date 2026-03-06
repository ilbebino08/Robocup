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
//  Interruzione di linea (gap) — regolamento RoboCup Rescue Line
//  Il robot avanza dritto per attraversare il salto. Se non trova
//  la linea entro GAP_CROSS_MS, torna indietro finché la rivede.
// ═══════════════════════════════════════════════════════════════════
enum LocalRecoveryState {
    LR_CONTROLLA_INDIETRO = 0, // retromarcia breve: svolta non completata?
    LR_AVANZA,               // attraversa il salto andando avanti
    LR_CERCA_LINEA,          // torna indietro finché non rivede la linea
    LR_CENTRA                // sterza sul posto finché la linea è centrata
};

static LocalRecoveryState _lrState;
static uint32_t           _lrPhaseStartMs;
static uint32_t           _recoveryOriginMs;

static void _exitToFollowing(RobotContext& ctx, uint32_t now, const char* msg) {
    resetPID();
    ctx.lineLostStartMs = 0;
    ctx.state        = STATE_FOLLOWING;
    ctx.stateStartMs = now;
    LL_LOG(msg);
}

void lineRecovery_enter(RobotContext& ctx) {
    ctx.state         = STATE_LINE_LOST_REVERSE;  // segnala recovery in corso
    ctx.stateStartMs  = millis();
    _recoveryOriginMs = millis();
    _lrPhaseStartMs   = millis();
    _lrState          = LR_CONTROLLA_INDIETRO;
    LL_LOG("[RV] enter -> CONTROLLA_INDIETRO");
}

void lineRecovery_update(RobotContext& ctx, int16_t pos) {
    const uint32_t now = millis();

    // ── Timeout assoluto globale ────────────────────────────────
    if ((now - _recoveryOriginMs) >= SEARCH_TOTAL_MS) {
        motori.stop();
        LL_LOG("RECOVERY TIMEOUT");
        return;
    }

    switch (_lrState) {

    // ── CONTROLLA_INDIETRO: retromarcia breve, poi verifica se c'è la linea ──
    case LR_CONTROLLA_INDIETRO: {
        LL_LOG2("[CI] pos=", (int)pos);
        if ((now - _lrPhaseStartMs) < GAP_CHECK_BACK_MS) {
            // Muovi sempre indietro per l'intera durata, senza controllare pos
            motori.muovi(REVERSE_VEL, 0);
        } else {
            // Solo dopo aver arretrato: controlla se c'è la linea
            if (pos > -TURN_CENTER_THRESHOLD && pos < TURN_CENTER_THRESHOLD) {
                _exitToFollowing(ctx, now, "[RV] linea dietro (svolta) -> FOLLOWING");
                return;
            }
            LL_LOG("[RV] nessuna linea dietro -> AVANZA");
            _lrState        = LR_AVANZA;
            _lrPhaseStartMs = now;
        }
        break;
    }

    // ── AVANZA: attraversa il salto dritto ──────────────────────
    case LR_AVANZA: {
        LL_LOG2("[AV] pos=", (int)pos);
        // pos != ±1750 → linea reale trovata (come il branch main)
        if (pos != -1750 && pos != 1750) {
            _exitToFollowing(ctx, now, "[RV] linea ritrovata -> FOLLOWING");
            return;
        }
        // Timeout avanzamento → torna indietro a cercare la linea
        if ((now - _lrPhaseStartMs) >= GAP_CROSS_MS) {
            LL_LOG("[RV] gap timeout -> CERCA_LINEA");
            _lrState        = LR_CERCA_LINEA;
            _lrPhaseStartMs = now;
        } else {
            motori.muovi(BASE_VEL, 0);
        }
        break;
    }

    // ── CERCA_LINEA: retromarcia finché non rivede la linea ──────
    case LR_CERCA_LINEA: {
        LL_LOG2("[CL] pos=", (int)pos);
        // pos != ±1750 → linea trovata
        if (pos != -1750 && pos != 1750) {
            if (pos >= -TURN_CENTER_THRESHOLD && pos <= TURN_CENTER_THRESHOLD) {
                _exitToFollowing(ctx, now, "[RV] linea centrata -> FOLLOWING");
                return;
            } else {
                LL_LOG2("[RV] linea trovata pos=", (int)pos);
                LL_LOG("[RV] -> CENTRA");
                _lrState = LR_CENTRA;
                _lrPhaseStartMs = now;
            }
        } else {
            motori.muovi(REVERSE_VEL, 0);
        }
        break;
    }

    // ── CENTRA: sterza sul posto finché la linea è centrata ───────
    case LR_CENTRA: {
        LL_LOG2("[CT] pos=", (int)pos);
        // Linea persa durante centraggio: torna a cercarla
        if (pos == -1750 || pos == 1750) {
            _lrState = LR_CERCA_LINEA;
            break;
        }
        if (pos >= -TURN_CENTER_THRESHOLD && pos <= TURN_CENTER_THRESHOLD) {
            _exitToFollowing(ctx, now, "[CT] centrato -> FOLLOWING");
            return;
        }
        // Sterza verso il centro senza avanzare
        const short steer = (pos < 0) ? TURN_RIGHT_ANG : TURN_LEFT_ANG;
        motori.muovi(0, steer);
        break;
    }

    default:
        break;
    }
}
