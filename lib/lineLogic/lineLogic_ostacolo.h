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
        S_VERIFICA_LINEA = 7,
        S_RIENTRO = 8
    };
    uint8_t stato;
    uint16_t tempoInizio;
    uint8_t latoCosteggiamento; // 0=destra, 1=sinistra
    uint16_t minDistPericolo;   // Minima distanza rilevata sul lato opposto
    uint8_t contatoreLetture;   // Contatore per lettura sensore opposto
    uint8_t verificaStep;       // Step interno per verifica linea (avanti/indietro)

    StatoOstacolo() : stato(0), tempoInizio(0), latoCosteggiamento(0), minDistPericolo(2000), contatoreLetture(0), verificaStep(0) {}

    void reset() {
        stato = 0;
        tempoInizio = 0;
        latoCosteggiamento = 0;
        minDistPericolo = 2000;
        contatoreLetture = 0;
        verificaStep = 0;
    }
};

// Dichiarazioni esterne
extern StatoOstacolo statoOstacolo;
extern int statoLinea();
extern void resetPID();

// API pubbliche gestione ostacoli
void gestisciOstacolo();

#endif
