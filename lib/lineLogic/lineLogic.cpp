#include "lineLogic.h"

// Riferimenti agli oggetti globali definiti in main.cpp
extern Motori motori;
extern BottomSensor IR_board;
extern MultiClickButton button;

// Stato interno per la gestione del verde a destra
enum StatoVerdeDx { S_NORMALE = 0, S_RILEVATO = 1, S_VERIFICA = 2, S_AVANZA_DOPO = 3 };
static StatoVerdeDx statoVerdeDx = S_NORMALE;
static unsigned long tempoRilevazioneVerde = 0;
static int contatoreVerdeConsecutivo = 0;
static unsigned long tempoAvanzamento = 0;

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

void initLineLogic() {
    statoVerdeDx = S_NORMALE;
    tempoRilevazioneVerde = 0;
    contatoreVerdeConsecutivo = 0;
    tempoAvanzamento = 0;
}

void gestisciLinea(int stato) {
    switch (stato) {
        case LINEA:
            debug.println("Seguendo la linea.");
            if (statoVerdeDx != S_NORMALE) statoVerdeDx = S_NORMALE;
            pidLineFollowing(DEFAULT_VELOCITY);
            break;

        case COL_RILEVATO:
            motori.stop();
            break;

        case DOPPIO_VERDE:
            gestisciDoppioVerde();
            break;

        case VERDE_SX:
            gestisciVerdeSinistra();
            break;

        case VERDE_DX:
            gestisciVerdeDestra();
            break;

        case NO_LINEA:
            motori.muovi(-400, 0);
            // comportamento di recupero: lascio che il loop principale gestisca pause
            break;

        default:
            break;
    }
}

void gestisciDoppioVerde() {
    motori.stop();
    debug.println("Doppio verde rilevato: fermo in attesa di istruzioni.");
    button.setPaused(true);
}

void gestisciVerdeSinistra() {
    debug.println("Curva a sinistra rilevata.");
    // azione non bloccante: avvio manovra e torno al controllo
    motori.muovi(-200, 200);
}

void gestisciVerdeDestra() {
    switch (statoVerdeDx) {
        case S_NORMALE:
            debug.println("Verde destra rilevato: rallento per verificare.");
            statoVerdeDx = S_RILEVATO;
            tempoRilevazioneVerde = millis();
            contatoreVerdeConsecutivo = 0;
            pidLineFollowing(DEFAULT_VELOCITY / 2);
            break;

        case S_RILEVATO:
            contatoreVerdeConsecutivo++;
            pidLineFollowing(DEFAULT_VELOCITY / 2);
            if (contatoreVerdeConsecutivo >= 3) {
                debug.println("Verde confermato: avanzando per verificare posizione.");
                statoVerdeDx = S_VERIFICA;
                tempoAvanzamento = millis();
            } else if (millis() - tempoRilevazioneVerde > 500) {
                debug.println("Falso verde: ripristino normale.");
                statoVerdeDx = S_NORMALE;
            }
            break;

        case S_VERIFICA:
            pidLineFollowing(DEFAULT_VELOCITY / 2);
            if (millis() - tempoAvanzamento > 250) {
                // se ancora vedo verde probabilmente è dopo la curva
                if (statoLinea() == VERDE_DX) {
                    debug.println("Verde dopo la svolta: ignoro e continuo dritto.");
                    statoVerdeDx = S_AVANZA_DOPO;
                    tempoAvanzamento = millis();
                } else {
                    debug.println("Verde prima della svolta: eseguo curva a destra.");
                    motori.muovi(200, -200);
                    // non blocchiamo: la successiva iterazione continuerà
                    statoVerdeDx = S_NORMALE;
                }
            }
            break;

        case S_AVANZA_DOPO:
            pidLineFollowing(DEFAULT_VELOCITY);
            if (statoLinea() != VERDE_DX || millis() - tempoAvanzamento > 500) {
                debug.println("Verde post-curva superato: ripristino normale.");
                statoVerdeDx = S_NORMALE;
            }
            break;
    }
}
