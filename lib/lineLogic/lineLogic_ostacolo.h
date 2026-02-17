#ifndef LINELOGIC_OSTACOLO_H
#define LINELOGIC_OSTACOLO_H

#include <Arduino.h>
#include <debug.h>

// Struct per la gestione dell'evitamento ostacoli
struct StatoOstacolo {
    enum Stato { 
        S_NORMALE = 0, 
        S_CENTRAMENTO = 1, 
        S_STERZATA_FUORI = 2, 
        S_AVANZA_FUORI = 3, 
        S_STERZATA_DENTRO = 4, 
        S_AVANZA_LATERALE = 5,
        S_RICERCA_LINEA = 6,
        S_RIENTRO = 7
    };
    uint8_t stato;
    uint16_t tempoInizio;

    StatoOstacolo() : stato(0), tempoInizio(0) {}

    void reset() {
        stato = 0;
        tempoInizio = 0;
    }
};

// Dichiarazioni esterne
extern StatoOstacolo statoOstacolo;
extern int statoLinea();
extern void resetPID();

// API pubbliche gestione ostacoli
void gestisciOstacolo();

#endif
