#include "UTurnHandler.h"
#include "GreenManager.h"
#include "../config.h"
#include <sensorBoard.h>
#include <motori.h>
#include <followLine.h>
#include <debug.h>
#include <Arduino.h>
#include <MultiClickButton.h>

extern BottomSensor IR_board;
extern Motori motori;
extern Debug debug;
extern MultiClickButton button;

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
    // Ruota sul posto all'infinito finché non troviamo la linea (bloccante)
    while (true) {
        button.update();
        if (button.isPaused()) {
            motori.stop();
            ctx.state = STATE_FOLLOWING;
            debug.println("INTERRUPT! (uturn)");
            return;
        }

        uint32_t current_now = millis();
        
        // Continua a dare il comando ai motori
        motori.muovi(0, UTURN_ANG);

        // Monitora checkLinea(): fermati quando il sensore rileva fisicamente la linea
        // Necessario chiamarlo per aggiornare i bit di status I2C costantemente
        IR_board.line(); 
        const bool lineDetected = IR_board.checkLinea();

        if (lineDetected && (int32_t)(current_now - ctx.stateStartMs) > 200) {  
            // U-turn completata
            greenManager_reset(ctx);
            resetPID();
            ctx.state = STATE_FOLLOWING;
            ctx.stateStartMs = current_now;
            debug.println("-> FOLLOWING (uturn ok line found)");
            break;
        }

        // Timeout d'emergenza
        if ((int32_t)(current_now - ctx.stateStartMs) >= (int32_t)UTURN_TIMEOUT_MS) {
            motori.stop();
            greenManager_reset(ctx);
            resetPID();
            ctx.state = STATE_FOLLOWING;
            ctx.stateStartMs = current_now;
            debug.println("-> FOLLOWING (uturn timeout)");
            break;
        }
    }
}
