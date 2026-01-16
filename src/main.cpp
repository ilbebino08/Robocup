#include <Arduino.h>
#include <sensorBoard.h>
#include <motori.h>
#include "robot.h"
#include "followLine.h"
#include "MultiClickButton.h"
#include "Funzioni.h"
#include <braccio.h>

BottomSensor IR_board;
Motori motori;
MultiClickButton button(BUTTON);
Braccio braccio;

long timer = 0;
long lastNoLineaTime = 0;

void setup() {
    Serial.begin(115200);
    Serial1.begin(57600);
    
    // Inizializza il sensorBoard
    IR_board.start();
    
    // Resetta lo stato PID
    resetPID();

    // Inizializza il button
    button.begin();

    // Configura il callback per il singolo click
    button.onSingleClick(singoloClick);
    button.onDoubleClick(doppioClick);
    button.onTripleClick(triploClick);

    braccio.init();

    motori.init();

    braccio.riposo();

    motori.stop();
}

void loop() {
    button.update();
    
    if(!button.isPaused()) {
        switch (statoLinea())
        {
        case LINEA:
            pidLineFollowing(DEFAULT_VELOCITY);
            break;
        case COL_RILEVATO:
            motori.stop();
            break;

        case DOPPIO_VERDE:
            
            break;
        case VERDE_SX:
            
            break;
        case VERDE_DX:
            
            break;
        case NO_LINEA:
            motori.muovi(-400, 0);
            lastNoLineaTime = millis();
            if(millis() - lastNoLineaTime > 1000){
                motori.stop();
                button.setPaused(true);
            } else {
                motori.muovi(-400, 0);
                timer = millis();
                while (millis() - timer < 1000){
                    button.update();
                    IR_board.utils.val_sensor();
                }
            }


            break;
        default:
            break;
        }
        
    }
}

    
