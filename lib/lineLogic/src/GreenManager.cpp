#include "GreenManager.h"
#include "../config.h"
#include <sensorBoard.h>
#include <debug.h>
#include <Arduino.h>

extern BottomSensor IR_board;
extern Debug debug;

// Risultato calcolato nell'ultimo update(), esposto via getEvent()
static GreenEvent _lastEvent = GREEN_NONE;

void greenManager_update(RobotContext& ctx) {
    // ─── 1. Lettura singola per ciclo ───────────────────────────
    const bool rawSx = IR_board.checkGreenSx();
    const bool rawDx = IR_board.checkGreenDx();

    // ─── 2. Aggiorna contatori (azzeramento immediato) ──────────
    if (rawSx) {
        if (ctx.greenCntSx < 255) ctx.greenCntSx++;
    } else {
        ctx.greenCntSx = 0;
    }

    if (rawDx) {
        if (ctx.greenCntDx < 255) ctx.greenCntDx++;
    } else {
        ctx.greenCntDx = 0;
    }

    // ─── 3. Gestisci flag ignore post-svolta ────────────────────
    //   Il flag si rimuove solo quando il verde fisico è sparito
    if (ctx.ignoreSxUntilClear && !rawSx) {
        ctx.ignoreSxUntilClear = false;
    }
    if (ctx.ignoreDxUntilClear && !rawDx) {
        ctx.ignoreDxUntilClear = false;
    }

    // ─── 4. Aggiorna greenConfirmed ─────────────────────────────
    if (!ctx.ignoreSxUntilClear && ctx.greenCntSx >= GREEN_CONFIRM_READS) {
        if (!ctx.greenConfirmedSx) {
            ctx.greenConfirmedSx = true;
            if (ctx.firstGreenConfMs == 0) {
                ctx.firstGreenConfMs = millis();
            }
        }
    }
    if (ctx.greenCntSx == 0) {
        ctx.greenConfirmedSx = false;
    }

    if (!ctx.ignoreDxUntilClear && ctx.greenCntDx >= GREEN_CONFIRM_READS) {
        if (!ctx.greenConfirmedDx) {
            ctx.greenConfirmedDx = true;
            if (ctx.firstGreenConfMs == 0) {
                ctx.firstGreenConfMs = millis();
            }
        }
    }
    if (ctx.greenCntDx == 0) {
        ctx.greenConfirmedDx = false;
    }

    // ─── 5. Classifica evento ───────────────────────────────────
    if (!ctx.greenConfirmedSx && !ctx.greenConfirmedDx) {
        _lastEvent = GREEN_NONE;
        // Se nessun lato confermato, reset timestamp finestra
        ctx.firstGreenConfMs = 0;
        return;
    }

    const bool bothConfirmed = ctx.greenConfirmedSx && ctx.greenConfirmedDx;
    const bool windowExpired =
        (millis() - ctx.firstGreenConfMs) >= GREEN_DOUBLE_WINDOW_MS;

    if (bothConfirmed) {
        _lastEvent = GREEN_DOUBLE;
        LL_LOG("[GM] event=DOUBLE");
    } else if (windowExpired) {
        _lastEvent = ctx.greenConfirmedSx ? GREEN_SINGLE_SX : GREEN_SINGLE_DX;
        LL_LOG2("[GM] event=SINGLE_", ctx.greenConfirmedSx ? "SX" : "DX");
    } else {
        _lastEvent = GREEN_WAITING;
    }
}

GreenEvent greenManager_getEvent(const RobotContext& /*ctx*/) {
    return _lastEvent;
}

void greenManager_reset(RobotContext& ctx) {
    ctx.greenCntSx        = 0;
    ctx.greenCntDx        = 0;
    ctx.greenConfirmedSx  = false;
    ctx.greenConfirmedDx  = false;
    ctx.firstGreenConfMs  = 0;
    _lastEvent            = GREEN_NONE;
}

void greenManager_setIgnoreAfterTurn(RobotContext& ctx, bool wasTurnLeft) {
    if (wasTurnLeft) {
        ctx.ignoreSxUntilClear = true;
    } else {
        ctx.ignoreDxUntilClear = true;
    }
    ctx.postTurnIgnoreMs = millis();
}
