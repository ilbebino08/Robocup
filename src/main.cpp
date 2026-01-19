#include <Arduino.h>
#include <sensorBoard.h>
#include <motori.h>
#include "robot.h"
#include "followLine.h"
#include "MultiClickButton.h"
#include "Funzioni.h"
#include <braccio.h>
#include <debug.h>
#include <lineLogic.h>

BottomSensor IR_board;
Motori motori;
MultiClickButton button(BUTTON);
Braccio braccio;

long timer = 0;
long lastNoLineaTime = 0;

void setup() {
    Serial.begin(115200);
    Serial1.begin(57600);
    Serial3.begin(9600);
    
    // Inizializza il sistema di debug
    debug.begin(DEBUG_USB | DEBUG_BLUETOOTH); // Cambia a DEBUG_USB | DEBUG_BLUETOOTH per usare anche Bluetooth
    debug.setBluetoothSerial(&Serial3);
    
    // Inizializza il sensorBoard
    IR_board.start();
    debug.println("Sensor board inizializzata.");
    
    // Resetta lo stato PID
    resetPID();

    debug.println("Button inizializzato.");

    // Inizializza il button
    button.begin();

    // Configura il callback per il singolo click
    button.onSingleClick(singoloClick);
    button.onDoubleClick(doppioClick);
    button.onTripleClick(triploClick);

    braccio.init();
    debug.println("Braccio inizializzato.");

    motori.init();
    debug.println("Motori inizializzati.");

    braccio.riposo();

    motori.stop();
}

void loop() {
    button.update();
    
    if(!button.isPaused()) {
        gestisciLinea(statoLinea());
    }
    
}