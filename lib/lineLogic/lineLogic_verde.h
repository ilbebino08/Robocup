#ifndef LINELOGIC_VERDE_H
#define LINELOGIC_VERDE_H

#include <Arduino.h>
#include <debug.h>

// Struct per lo stato della gestione del verde singolo
struct StatoVerde {
    enum Stato { S_NORMALE = 0, S_RILEVATO = 1, S_VERIFICA_AVANTI = 2, S_VERIFICA_INDIETRO = 3, S_IN_MANOVRA = 4, S_AVANZA_DOPO = 5 };
    uint8_t stato;
    uint16_t tempoRilevazione;
    uint8_t contatoreConsecutivo;
    uint16_t tempoAvanzamento;
    int16_t lineaIniziale;
    bool doppioVerdeRilevato;

    StatoVerde() : stato(0), tempoRilevazione(0), contatoreConsecutivo(0), tempoAvanzamento(0), lineaIniziale(0), doppioVerdeRilevato(false) {}

    void reset() {
        stato = 0;
        tempoRilevazione = 0;
        contatoreConsecutivo = 0;
        tempoAvanzamento = 0;
        lineaIniziale = 0;
        doppioVerdeRilevato = false;
    }
};

// Struct per lo stato della gestione del doppio verde (rotazione 180)
struct StatoDoppioVerde {
    enum Stato { S_NORMALE = 0, S_RILEVATO = 1, S_IN_ROTAZIONE = 2, S_AVANZA_CIECO = 3 };
    uint8_t stato;
    uint16_t tempoInizio;
    uint8_t contatoreConsecutivo;

    StatoDoppioVerde() : stato(0), tempoInizio(0), contatoreConsecutivo(0) {}

    void reset() {
        stato = 0;
        tempoInizio = 0;
        contatoreConsecutivo = 0;
    }
};

// Struct per gestione linea a 0
struct StatoVerdeZero {
    enum Stato { S_NORMALE = 0, S_FERMO = 1, S_INDIETRO_VERIFICA = 2 };
    uint8_t stato;
    uint16_t tempoInizio;

    StatoVerdeZero() : stato(0), tempoInizio(0) {}

    void reset() {
        stato = 0;
        tempoInizio = 0;
    }
};

// Dichiarazioni esterne per lo stato verde
extern StatoVerde statoVerdeDx;
extern StatoVerde statoVerdeSx;
extern StatoDoppioVerde statoDoppioVerde;
extern StatoVerdeZero statoVerdeZero;
extern uint16_t ultimoTempoVerde;
extern uint16_t ultimoTempoDoppioVerde;

// API pubbliche gestione verdi
void gestisciDoppioVerde();
void gestisciVerdeGenerico(StatoVerde& stato, int statoCorrente, int velocita, int angolo, const char* nomeDir);
void gestisciVerdeSinistra();
void gestisciVerdeDestra();
void gestisciVerdeConLineaZero(int statoIniziale);

#endif
