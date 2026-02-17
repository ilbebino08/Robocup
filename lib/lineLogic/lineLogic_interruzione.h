#ifndef LINELOGIC_INTERRUZIONE_H
#define LINELOGIC_INTERRUZIONE_H

#include <Arduino.h>
#include <debug.h>

// Struct per la gestione dell'interruzione di linea
struct StatoInterruzione {
    enum Stato { S_NORMALE = 0, S_FERMATO = 1, S_AVANTI_COLORE = 2, S_INDIETRO_COLORE = 3, S_CERCA_LINEA = 4, S_VERIFICA = 5, S_AVANZA_INTERRUZIONE = 6 };
    uint8_t stato;
    uint16_t tempoInizio;

    StatoInterruzione() : stato(0), tempoInizio(0) {}

    void reset() {
        stato = 0;
        tempoInizio = 0;
    }
};

// Dichiarazioni esterne
extern StatoInterruzione statoInterruzione;
extern int statoLinea();
extern void resetPID();

// API pubbliche gestione interruzioni
void gestisciNoLinea();

#endif
