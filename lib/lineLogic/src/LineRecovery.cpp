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
//  Interruzione di linea — nuova logica:
//  FERMATO → INDIETRO_SCAN (retromarcia + controlla linea)
//    → se |pos| > 1000: CORREGGI (sterza al centro)
//    → se |pos| ≤ 1000: PID_ALIGN (breve PID av/ind) → AVANZA
//    → timeout senza linea: AVANZA dritto
// ═══════════════════════════════════════════════════════════════════
enum LocalRecoveryState {
    LR_FERMATO = 0,
    LR_INDIETRO_SCAN,   // retromarcia mentre cerca la linea
    LR_INDIETRO_EXTRA,  // extra retromarcia prima di correggere
    LR_CORREGGI,        // sterza sul posto verso il centro
    LR_PID_ALIGN,       // breve PID per allinearsi prima di avanzare
    LR_AVANZA           // avanza dritto finché trova la linea
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
    ctx.state         = STATE_LINE_LOST_REVERSE;
    ctx.stateStartMs  = millis();
    _recoveryOriginMs = millis();
    _lrPhaseStartMs   = millis();
    _lrState          = LR_FERMATO;
    motori.stop();
    LL_LOG("[RV] enter -> FERMATO");
}

void lineRecovery_update(RobotContext& ctx, int16_t pos) {
    const uint32_t now = millis();

    // ── Timeout assoluto globale ────────────────────────────────
    if ((int32_t)(now - _recoveryOriginMs) >= (int32_t)SEARCH_TOTAL_MS) {
        motori.stop();
        LL_LOG("RECOVERY TIMEOUT");
        return;
    }

    switch (_lrState) {

    // ── FERMATO: stop breve prima di provare retromarcia ──────────────────────────────────────
    case LR_FERMATO:
        motori.stop(); // Ci ri-assicuriamo che i motori siano forzati a 0 per bloccare l'inerzia
        if ((int32_t)(now - _lrPhaseStartMs) >= 500) { // Aumentato a 500 per far smorzare l'inerzia del robot
            LL_LOG("[RV] FERMATO -> INDIETRO_SCAN");
            _lrState        = LR_INDIETRO_SCAN;
            _lrPhaseStartMs = now;
        }
        break;

    // ── INDIETRO_SCAN: retromarcia, controlla la linea ───────────
    case LR_INDIETRO_SCAN: {
        // Log ridotto per non intasare, traccia solo ad ogni secondo o cambio di stato
        static uint32_t lastLogScan = 0;
        if ((int32_t)(now - lastLogScan) > 300) {
            LL_LOG2("[SC] pos=", (int)pos);
            lastLogScan = now;
        }

        if (pos != -1750 && pos != 1750) {
            // -- LOGICA RITROVATA DURANTE RETROMARCIA --
            // Assicuriamoci che l'abbia vista per davvero e non sia un glitch:
            // reset della condizione e switch a state appropriato.
            const int16_t absPos = pos < 0 ? -pos : pos;
            if (absPos > 1000) {
                // Linea trovata ma spostata: vai indietro extra e poi correggi
                LL_LOG2("[RV] linea laterale pos=", (int)pos);
                LL_LOG("[RV] -> INDIETRO_EXTRA");
                _lrState        = LR_INDIETRO_EXTRA;
                _lrPhaseStartMs = now;
            } else {
                // Linea trovata e abbastanza centrata: allinea col PID
                LL_LOG("[RV] linea centrata -> PID_ALIGN");
                _lrState        = LR_PID_ALIGN;
                _lrPhaseStartMs = now;
            }
        } 
        else if ((int32_t)(now - _lrPhaseStartMs) >= (int32_t)GAP_CHECK_BACK_MS) {
            // Nessuna linea trovata andando indietro per tutto GAP_CHECK_BACK_MS.
            // Siamo su un gap. Avanziamo verso l'altra sponda.
            LL_LOG("[RV] nessuna linea indietro in tempo -> AVANZA");
            _lrState        = LR_AVANZA;
            _lrPhaseStartMs = now;
        } 
        else {
            // Ancora non abbiamo trovato la linea e siamo entro il tempo limite:
            // Continuiamo ad applicare i motori per andare indietro
            motori.muovi(REVERSE_VEL, 0);
        }
        break;
    }

    // ── INDIETRO_EXTRA: retromarcia extra prima di correggere lo sterzo ──
    case LR_INDIETRO_EXTRA:
        if ((int32_t)(now - _lrPhaseStartMs) >= 400) { // Tempo di extra-retromarcia (aumentato per coprire i delay del loop)
            LL_LOG("[RV] fine indietro extra -> CORREGGI");
            _lrState        = LR_CORREGGI;
            _lrPhaseStartMs = now;
        } else {
            motori.muovi(REVERSE_VEL, 0); // Continua ad andare indietro
        }
        break;

    // ── CORREGGI: sterza sul posto finché |pos| ≤ 1000 ──────────
    case LR_CORREGGI: {
        LL_LOG2("[CG] pos=", (int)pos);
        if (pos == -1750 || pos == 1750) {
            // Linea persa durante correzione: torna a scansionare
            _lrState = LR_INDIETRO_SCAN;
            _lrPhaseStartMs = now;
            break;
        }
        const int16_t absPos = pos < 0 ? -pos : pos;
        if (absPos <= 1000) {
            LL_LOG("[RV] corretto -> PID_ALIGN");
            _lrState        = LR_PID_ALIGN;
            _lrPhaseStartMs = now;
        } else {
            // Correggi la traiettoria sul posto tramite la funzione PID
            pidLineFollowing(0, pos);
        }
        break;
    }

    // ── PID_ALIGN: breve PID per centrare, poi avanza ────────────
    case LR_PID_ALIGN:
        LL_LOG2("[PA] pos=", (int)pos);
        if (pos == -1750 || pos == 1750) {
            // Linea persa di nuovo: interrompi il PID e vai dritto
            _lrState        = LR_AVANZA;
            _lrPhaseStartMs = now;
            break;
        }
        
        if ((int32_t)(now - _lrPhaseStartMs) >= 600) { // Un po' di PID prima di andare dritto
            LL_LOG("[RV] PID_ALIGN done -> AVANZA");
            _lrState        = LR_AVANZA;
            _lrPhaseStartMs = now;
        } else {
            // Fa un po' avanti e indietro con il PID se necessario o semplicemente usa il PID
            if ((int32_t)(now - _lrPhaseStartMs) < 300) {
                pidLineFollowing(REVERSE_VEL, pos); // indietro con pid
            } else {
                pidLineFollowing(SEARCH_VEL, pos);  // avanti con pid
            }
        }
        break;

    // ── AVANZA: dritto finché pos != ±1750 ───────────────────────
    case LR_AVANZA:
        LL_LOG2("[AV] pos=", (int)pos);
        if (pos != -1750 && pos != 1750) {
            _exitToFollowing(ctx, now, "[RV] linea ritrovata -> FOLLOWING");
        } else {
            motori.muovi(SEARCH_VEL, 0); // Vai dritto
        }
        break;

    default:
        break;
    }
}
