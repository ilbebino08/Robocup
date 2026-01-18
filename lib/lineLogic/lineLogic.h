#ifndef LINE_LOGIC_H
#define LINE_LOGIC_H

#include <Arduino.h>
#include <motori.h>
#include <sensorBoard.h>
#include <MultiClickButton.h>
#include <debug.h>
#include "followLine.h"

// Inizializzazione opzionale (chiama dal setup se necessario)
void initLineLogic();

// Funzione principale che gestisce lo stato corrente della linea
// Chiamare da loop(): gestisciLinea(statoLinea());
void gestisciLinea(int stato);

// Funzioni per casi specifici (esposte per test o richieste speciali)
void gestisciDoppioVerde();
void gestisciVerdeSinistra();
void gestisciVerdeDestra();

/**
 * @brief Restituisce lo stato attuale della linea rilevata dai sensori.
 * 
 * @return int -> Stato della linea (LINEA, NO_LINEA, COL_RILEVATO, VERDE_SX, VERDE_DX, DOPPIO_VERDE).
 */
int statoLinea();

#endif
