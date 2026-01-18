#ifndef LINE_LOGIC_H
#define LINE_LOGIC_H

#include <Arduino.h>
#include <motori.h>
#include <sensorBoard.h>
#include <MultiClickButton.h>
#include <debug.h>
#include "followLine.h"

/**
 * @brief Inizializza lo stato interno della libreria lineLogic.
 * 
 * Azzera tutti i contatori, timer e imposta la macchina a stati per il verde
 * in stato NORMALE. Chiamare dal setup() prima di iniziare il tracciamento.
 */
void initLineLogic();

/**
 * @brief Gestisce lo stato corrente della linea e coordina le azioni del robot.
 * 
 * Funzione principale che implementa la logica di controllo basata sullo stato
 * rilevato dai sensori. Delega l'attuazione alle librerie motori e followLine.
 * Chiamare ad ogni iterazione del loop: gestisciLinea(statoLinea()).
 * 
 * @param stato Stato della linea ritornato da statoLinea() (LINEA, VERDE_SX, VERDE_DX, etc.)
 */
void gestisciLinea(int stato);

/**
 * @brief Gestisce l'intersezione con doppio verde.
 * 
 * Ferma il robot e lo mette in pausa. Tipicamente usato per fine percorso
 * o punti di decisione che richiedono input esterno.
 */
void gestisciDoppioVerde();

/**
 * @brief Gestisce la curva a sinistra quando rileva verde.
 * 
 * Avvia la manovra di rotazione a sinistra in modo non bloccante.
 * La funzione ritorna immediatamente dopo aver comandato i motori.
 */
void gestisciVerdeSinistra();

/**
 * @brief Gestisce la curva a destra con verifica intelligente della posizione del verde.
 * 
 * Implementa una macchina a stati (4 stati) per distinguere il verde prima della curva
 * (da seguire) dal verde dopo la curva (da ignorare). Include verifica di consistenza
 * e timeout per falsi positivi. Comportamento non bloccante.
 */
void gestisciVerdeDestra();

#endif
