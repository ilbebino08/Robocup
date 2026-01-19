#include "lineLogic.h"

// Parametri configurabili per le manovre
#define VELOCITA_STERZATA   600      // Velocità durante la sterzata (-1023 a +1023)
#define ANGOLO_STERZATA     1750     // Angolo durante la sterzata (-1750 a +1750)
#define TEMPO_STERZATA      300      // Tempo di sterzata in millisecondi
#define SOGLIA_SCOSTAMENTO  1500     // Soglia per rilevare scostamento linea
#define TEMPO_VERIFICA      250      // Tempo di verifica prima della decisione
#define CONTATORE_CONFERMA  3        // Numero di letture consecutive per conferma verde
#define TIMEOUT_FALSO_VERDE 500      // Timeout per rilevare un falso verde
#define VELOCITA_180        0        // Velocità durante la rotazione di 180 gradi (-1023 a +1023)
#define ANGOLO_180          1750     // Angolo per rotazione sul posto (massimo)
#define TEMPO_180           1000     // Tempo per completare la rotazione di 180 gradi

// Riferimenti agli oggetti globali definiti in main.cpp
extern Motori motori;
extern BottomSensor IR_board;
extern MultiClickButton button;

// Struct per lo stato della gestione del verde
struct StatoVerde {
    enum Stato { S_NORMALE = 0, S_RILEVATO = 1, S_VERIFICA = 2, S_IN_MANOVRA = 3, S_AVANZA_DOPO = 4 };
    Stato stato;
    unsigned long tempoRilevazione;
    int contatoreConsecutivo;
    unsigned long tempoAvanzamento;
    int lineaIniziale;  // Valore della linea al momento del rilevamento

    StatoVerde() : stato(S_NORMALE), tempoRilevazione(0), contatoreConsecutivo(0), tempoAvanzamento(0), lineaIniziale(0) {}
    
    void reset() {
        stato = S_NORMALE;
        tempoRilevazione = 0;
        contatoreConsecutivo = 0;
        tempoAvanzamento = 0;
        lineaIniziale = 0;
    }
};

static StatoVerde statoVerdeDx;
static StatoVerde statoVerdeSx;

// Stato per la gestione del doppio verde (rotazione 180)
struct StatoDoppioVerde {
    enum Stato { S_NORMALE = 0, S_IN_ROTAZIONE = 1 };
    Stato stato;
    unsigned long tempoInizio;
    
    StatoDoppioVerde() : stato(S_NORMALE), tempoInizio(0) {}
    
    void reset() {
        stato = S_NORMALE;
        tempoInizio = 0;
    }
};

static StatoDoppioVerde statoDoppioVerde;

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
    statoVerdeDx.reset();
    statoVerdeSx.reset();
    statoDoppioVerde.reset();
}

void gestisciLinea(int stato) {
    switch (stato) {
        case LINEA:
            debug.println("Seguendo la linea.");
            if (statoVerdeDx.stato != StatoVerde::S_NORMALE && statoVerdeDx.stato != StatoVerde::S_IN_MANOVRA) 
                statoVerdeDx.reset();
            if (statoVerdeSx.stato != StatoVerde::S_NORMALE && statoVerdeSx.stato != StatoVerde::S_IN_MANOVRA) 
                statoVerdeSx.reset();
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
            motori.muovi(-400, 0);  // Retromarcia dritta
            // comportamento di recupero: lascio che il loop principale gestisca pause
            break;

        default:
            break;
    }
}

void gestisciDoppioVerde() {
    switch (statoDoppioVerde.stato) {
        case StatoDoppioVerde::S_NORMALE:
            debug.println("Doppio verde rilevato: inizio rotazione 180 gradi.");
            statoDoppioVerde.stato = StatoDoppioVerde::S_IN_ROTAZIONE;
            statoDoppioVerde.tempoInizio = millis();
            resetPID();  // Reset del PID prima di usare i motori direttamente
            motori.muovi(VELOCITA_180, ANGOLO_180);  // Rotazione sul posto
            break;
            
        case StatoDoppioVerde::S_IN_ROTAZIONE:
            // Continua la rotazione
            motori.muovi(VELOCITA_180, ANGOLO_180);
            
            if (millis() - statoDoppioVerde.tempoInizio > TEMPO_180) {
                debug.println("Rotazione 180 completata: ripristino normale.");
                statoDoppioVerde.reset();
                resetPID();  // Reset del PID dopo aver usato i motori direttamente
            }
            break;
    }
}

/**
 * @brief Gestisce il verde in modo generico (sia destra che sinistra)
 * @param stato Riferimento allo stato verde (dx o sx)
 * @param statoCorrente Stato della linea attuale (VERDE_DX o VERDE_SX)
 * @param velocita Velocità durante la sterzata (-1023 a +1023)
 * @param angolo Angolo durante la sterzata (-1750 a +1750)
 * @param nomeDir Nome della direzione per i messaggi di debug
 */
void gestisciVerdeGenerico(StatoVerde& stato, int statoCorrente, int velocita, int angolo, const char* nomeDir) {
    switch (stato.stato) {
        case StatoVerde::S_NORMALE:
            debug.print("Verde ");
            debug.print(nomeDir);
            debug.println(" rilevato: rallento per verificare.");
            stato.stato = StatoVerde::S_RILEVATO;
            stato.tempoRilevazione = millis();
            stato.contatoreConsecutivo = 0;
            stato.lineaIniziale = IR_board.line();
            pidLineFollowing(DEFAULT_VELOCITY / 2);
            break;

        case StatoVerde::S_RILEVATO:
            stato.contatoreConsecutivo++;
            pidLineFollowing(DEFAULT_VELOCITY / 2);
            if (stato.contatoreConsecutivo >= CONTATORE_CONFERMA) {
                debug.print("Verde ");
                debug.print(nomeDir);
                debug.println(" confermato: verifico posizione.");
                stato.stato = StatoVerde::S_VERIFICA;
                stato.tempoAvanzamento = millis();
            } else if (millis() - stato.tempoRilevazione > TIMEOUT_FALSO_VERDE) {
                debug.println("Falso verde: ripristino normale.");
                stato.reset();
            }
            break;

        case StatoVerde::S_VERIFICA:
            pidLineFollowing(DEFAULT_VELOCITY / 2);
            if (millis() - stato.tempoAvanzamento > TEMPO_VERIFICA) {
                int lineaAttuale = IR_board.line();
                int scostamento = abs(lineaAttuale - stato.lineaIniziale);
                
                // Se la linea si discosta molto, è probabilmente prima della svolta
                if (scostamento > SOGLIA_SCOSTAMENTO) {
                    debug.print("Scostamento rilevato: ");
                    debug.print(scostamento);
                    debug.print(" - Verde prima della svolta a ");
                    debug.println(nomeDir);
                    resetPID();
                    motori.muovi(velocita, angolo);
                    stato.stato = StatoVerde::S_IN_MANOVRA;
                    stato.tempoAvanzamento = millis();
                } else if (statoLinea() == statoCorrente) {
                    // Ancora vedo verde e senza scostamento = dopo la curva
                    debug.print("Verde dopo la svolta a ");
                    debug.print(nomeDir);
                    debug.println(": ignoro e continuo dritto.");
                    stato.stato = StatoVerde::S_AVANZA_DOPO;
                    stato.tempoAvanzamento = millis();
                } else {
                    // Non vedo più verde e nessuno scostamento = eseguo svolta
                    debug.print("Verde prima della svolta a ");
                    debug.println(nomeDir);
                    resetPID();
                    motori.muovi(velocita, angolo);
                    stato.stato = StatoVerde::S_IN_MANOVRA;
                    stato.tempoAvanzamento = millis();
                }
            }
            break;

        case StatoVerde::S_IN_MANOVRA:
            // Continuo la manovra per il tempo stabilito
            motori.muovi(velocita, angolo);
            if (millis() - stato.tempoAvanzamento > TEMPO_STERZATA) {
                debug.println("Manovra completata: ripristino normale.");
                stato.reset();
                resetPID();
            }
            break;

        case StatoVerde::S_AVANZA_DOPO:
            pidLineFollowing(DEFAULT_VELOCITY);
            // Se non vedo più verde o è passato abbastanza tempo, torno normale
            if (statoLinea() != statoCorrente || millis() - stato.tempoAvanzamento > 500) {
                debug.println("Verde post-curva superato: ripristino normale.");
                stato.reset();
            }
            break;
    }
}

void gestisciVerdeSinistra() {
    gestisciVerdeGenerico(statoVerdeSx, VERDE_SX, VELOCITA_STERZATA, -ANGOLO_STERZATA, "sinistra");
}

void gestisciVerdeDestra() {
    gestisciVerdeGenerico(statoVerdeDx, VERDE_DX, VELOCITA_STERZATA, ANGOLO_STERZATA, "destra");
}
