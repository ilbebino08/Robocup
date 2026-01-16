#include <Arduino.h>
#include <motori.h>
#include <followLine.h>
#include <braccio.h>
#include <sensorBoard.h>
#include <robot.h>

extern BottomSensor IR_board;
extern Motori motori;
extern Braccio braccio;


/**
 * @brief Funzione chiamata al singolo click del pulsante.
 */
void singoloClick() {
    Serial.println("Pressione singola rilevata!");
    motori.stop();
    braccio.riposo();
    resetPID();
}

/**
 * @brief Funzione chiamata al doppio click del pulsante.
 */
void doppioClick() {
    // Azione da eseguire al doppio click del pulsante
    Serial.println("Pulsante premuto due volte!");
    IR_board.utils.calibration();
    resetPID();
}

/**
 * @brief Funzione chiamata al triplo click del pulsante.
 */
void triploClick() {
    // Azione da eseguire al triplo click del pulsante
    Serial.println("Pulsante premuto tre volte!");
    resetPID();
}


int statoLinea(){
    if(IR_board.checkColor())                               return COL_RILEVATO;
    if(IR_board.checkGreenDx() and IR_board.checkGreenSx()) return DOPPIO_VERDE;
    if(IR_board.checkGreenSx())                             return VERDE_SX;
    if(IR_board.checkGreenDx())                             return VERDE_DX;
    if(IR_board.checkLinea() != 1)                          return NO_LINEA;
    return LINEA;
}