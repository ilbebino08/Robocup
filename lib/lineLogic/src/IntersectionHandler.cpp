#include "IntersectionHandler.h"
#include "../config.h"
#include <sensorBoard.h>
#include <motori.h>
#include <followLine.h>
#include <debug.h>
#include <Arduino.h>

extern BottomSensor IR_board;
extern Motori motori;
extern Debug debug;

bool intersectionDetected(RobotContext& ctx, uint32_t now) {
    const int16_t pos = IR_board.line();

    // line()==0 + checkLinea()==true → possibile incrocio
    // line()==0 + checkLinea()==false → following normale centrato
    if (pos == 0 && IR_board.checkLinea()) {
        if (ctx.crossZeroStartMs == 0) {
            ctx.crossZeroStartMs = now;
        }
        return (int32_t)(now - ctx.crossZeroStartMs) >= (int32_t)CROSS_CONFIRM_MS;
    }

    // Non più a zero → reset contatore
    ctx.crossZeroStartMs = 0;
    return false;
}

void intersectionHandler_enter(RobotContext& ctx) {
    ctx.state = STATE_INTERSECTION;
    ctx.stateStartMs = millis();
    ctx.crossZeroStartMs = 0;
    debug.println("-> INTERSECTION");
}

void intersectionHandler_update(RobotContext& ctx) {
    const int16_t pos = IR_board.line();

    // Attraversa l'incrocio a velocità ridotta usando il PID
    pidLineFollowing(CROSS_VEL);

    // Uscita: line() != 0 stabilmente → linea ritrovata
    if (pos != 0) {
        resetPID();
        ctx.state = STATE_FOLLOWING;
        ctx.stateStartMs = millis();
        debug.println("-> FOLLOWING (cross ok)");
    }
}
