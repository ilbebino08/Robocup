/**
 * @file Funzioni.h
 * @brief Funzioni di utilità per la gestione degli eventi del pulsante e per la lettura dello stato della linea tramite i sensori.
 * 
 * - Gestione callback per singolo, doppio e triplo click del pulsante.
 * - Funzione di stato per la linea (linea, nessuna linea, colore rilevato, verde SX/DX, doppio verde).
 * 
 * Queste funzioni vengono richiamate nel main loop per gestire le azioni principali del robot in base agli input utente e ai sensori.
 */

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

/**
 * @brief Restituisce lo stato attuale della linea rilevata dai sensori.
 * 
 * @return int -> Stato della linea (LINEA, NO_LINEA, COL_RILEVATO, VERDE_SX, VERDE_DX, DOPPIO_VERDE).
 */
int statoLinea(){
    if(IR_board.checkColor())                               return COL_RILEVATO;
    if(IR_board.checkGreenDx() and IR_board.checkGreenSx()) return DOPPIO_VERDE;
    if(IR_board.checkGreenSx())                             return VERDE_SX;
    if(IR_board.checkGreenDx())                             return VERDE_DX;
    if(IR_board.checkLinea() != 1)                          return NO_LINEA;
    return LINEA;
}