#include "UTurnHandler.h"
#include "GreenManager.h"
#include "../config.h"
#include <sensorBoard.h>
#include <motori.h>
#include <followLine.h>
#include <debug.h>
#include <Arduino.h>

extern BottomSensor IR_board;
extern Motori motori;
extern Debug debug;

// Segno della posizione linea all'ingresso (locale allo stato, non in ctx)
static int8_t _entrySign;

void uTurnHandler_enter(RobotContext& ctx) {
    ctx.state = STATE_UTURN;
    ctx.stateStartMs = millis();

    const int16_t pos = IR_board.line();
    if (pos > 0)      _entrySign =  1;
    else if (pos < 0) _entrySign = -1;
    else              _entrySign =  1;  // default se centrato

    debug.println("-> UTURN");
}

void uTurnHandler_update(RobotContext& ctx) {
    const uint32_t now = millis();

    // Timeout
    if ((now - ctx.stateStartMs) >= UTURN_TIMEOUT_MS) {
        motori.stop();
        greenManager_reset(ctx);
        resetPID();
        ctx.state = STATE_FOLLOWING;
        ctx.stateStartMs = now;
        debug.println("-> FOLLOWING (uturn timeout)");
        return;
    }

    // Ruota sul posto
    motori.muovi(0, UTURN_ANG);

    // Monitora line(): fermati quando il segno si è invertito
    // E il robot è sufficientemente centrato
    const int16_t pos = IR_board.line();
    int8_t curSign = 0;
    if (pos > 0)      curSign =  1;
    else if (pos < 0) curSign = -1;

    const int16_t absPos = (pos < 0) ? -pos : pos;

    if (curSign != 0 && curSign != _entrySign &&
        absPos < TURN_CENTER_THRESHOLD) {
        // U-turn completata
        greenManager_reset(ctx);
        resetPID();
        ctx.state = STATE_FOLLOWING;
        ctx.stateStartMs = now;
        debug.println("-> FOLLOWING (uturn ok)");
    }
}
