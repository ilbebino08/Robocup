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
#include <debug.h>

extern BottomSensor IR_board;
extern Motori motori;
extern Braccio braccio;


/**
 * @brief Funzione chiamata al singolo click del pulsante.
 */
void singoloClick() {
    debug.println("Pressione singola rilevata!");
    motori.stop();
    braccio.riposo();
    resetPID();
}

/**
 * @brief Funzione chiamata al doppio click del pulsante.
 */
void doppioClick() {
    // Azione da eseguire al doppio click del pulsante
    debug.println("Pulsante premuto due volte!");
    IR_board.utils.calibration();
    resetPID();
}

/**
 * @brief Funzione chiamata al triplo click del pulsante.
 */
void triploClick() {
    // Azione da eseguire al triplo click del pulsante
    debug.println("Pulsante premuto tre volte!");
    IR_board.utils.greenDxCalibration();
    resetPID();
}

