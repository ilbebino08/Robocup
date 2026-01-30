#include "lineLogic.h"

// Forward declarations
void gestisciVerdeConLineaZero(int statoIniziale);

// Parametri configurabili per le manovre
#define VELOCITA_STERZATA   1000     // Velocità durante la sterzata (-1023 a +1023)
#define ANGOLO_STERZATA     1000     // Angolo durante la sterzata (-1750 a +1750)
#define TEMPO_STERZATA      300      // Tempo di sterzata in millisecondi
#define SOGLIA_SCOSTAMENTO  1500     // Soglia per rilevare scostamento linea
#define TEMPO_VERIFICA      250      // Tempo di verifica prima della decisione
#define CONTATORE_CONFERMA  3        // Numero di letture consecutive per conferma verde
#define TIMEOUT_FALSO_VERDE 500      // Timeout per rilevare un falso verde
#define TEMPO_IGNORA_VERDE  1000     // Tempo in ms per ignorare verdi dopo l'ultimo verde gestito
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
    enum Stato { S_NORMALE = 0, S_RILEVATO = 1, S_IN_ROTAZIONE = 2 };
    Stato stato;
    unsigned long tempoInizio;
    int contatoreConsecutivo;
    
    StatoDoppioVerde() : stato(S_NORMALE), tempoInizio(0), contatoreConsecutivo(0) {}
    
    void reset() {
        stato = S_NORMALE;
        tempoInizio = 0;
        contatoreConsecutivo = 0;
    }
};

static StatoDoppioVerde statoDoppioVerde;

// Stato per la gestione dell'interruzione di linea
struct StatoInterruzione {
    enum Stato { S_NORMALE = 0, S_FERMATO = 1, S_AVANTI_COLORE = 2, S_INDIETRO_COLORE = 3, S_CERCA_LINEA = 4, S_VERIFICA = 5, S_AVANZA_INTERRUZIONE = 6 };
    Stato stato;
    unsigned long tempoInizio;
    
    StatoInterruzione() : stato(S_NORMALE), tempoInizio(0) {}
    
    void reset() {
        stato = S_NORMALE;
        tempoInizio = 0;
    }
};

static StatoInterruzione statoInterruzione;

// Stato per la gestione del verde con linea a 0
struct StatoVerdeZero {
    enum Stato { S_NORMALE = 0, S_FERMO = 1, S_INDIETRO_VERIFICA = 2 };
    Stato stato;
    unsigned long tempoInizio;
    
    StatoVerdeZero() : stato(S_NORMALE), tempoInizio(0) {}
    
    void reset() {
        stato = S_NORMALE;
        tempoInizio = 0;
    }
};

static StatoVerdeZero statoVerdeZero;

// Tempo dell'ultimo verde gestito
static unsigned long ultimoTempoVerde = 0;

/**
 * @brief Restituisce lo stato attuale della linea rilevata dai sensori.
 * 
 * @return int -> Stato della linea (LINEA, NO_LINEA, COL_RILEVATO, VERDE_SX, VERDE_DX, DOPPIO_VERDE).
 */
int statoLinea(){
    // Prima controlla colori speciali
    if(IR_board.checkColor())                               return COL_RILEVATO;
    if(IR_board.checkGreenDx() and IR_board.checkGreenSx()) return DOPPIO_VERDE;
    if(IR_board.checkGreenSx())                             return VERDE_SX;
    if(IR_board.checkGreenDx())                             return VERDE_DX;
    
    // Leggi la posizione della linea
    int line_position = IR_board.line();
    
    // Se la linea è ai limiti estremi (-1750 o 1750), significa che è persa
    if(line_position == -1750 || line_position == 1750)     return NO_LINEA;
    
    // In tutti gli altri casi (incluso line_position == 0), la linea è presente
    return LINEA;
}

void initLineLogic() {
    statoVerdeDx.reset();
    statoVerdeSx.reset();
    statoDoppioVerde.reset();
    statoInterruzione.reset();
    statoVerdeZero.reset();
}

void gestisciLinea(int stato) {
    // Gestione LED in base allo stato
    if (stato == VERDE_SX || stato == VERDE_DX || stato == DOPPIO_VERDE) {
        digitalWrite(LED_V, HIGH);
        digitalWrite(LED_R, LOW);
        digitalWrite(LED_G, LOW);
    } else if (statoInterruzione.stato != StatoInterruzione::S_NORMALE) {
        digitalWrite(LED_R, HIGH);
        digitalWrite(LED_V, LOW);
        digitalWrite(LED_G, LOW);
    } else if (stato == LINEA) {
        digitalWrite(LED_G, HIGH);
        digitalWrite(LED_V, LOW);
        digitalWrite(LED_R, LOW);
    } else {
        digitalWrite(LED_V, LOW);
        digitalWrite(LED_R, LOW);
        digitalWrite(LED_G, LOW);
    }

    // Gestione speciale: se la linea è a 0 E non si è in gestione verde, ferma e verifica
    int line_position = IR_board.line();
    if (line_position == 0 && stato == LINEA && 
        statoVerdeDx.stato == StatoVerde::S_NORMALE && 
        statoVerdeSx.stato == StatoVerde::S_NORMALE) {
        gestisciVerdeConLineaZero(stato);
        return;
    }

    switch (stato) {
        case LINEA:
            // Non interrompere se c'è una manovra verde in corso
            if (statoVerdeDx.stato == StatoVerde::S_IN_MANOVRA) {
                gestisciVerdeDestra();
                return;
            }
            if (statoVerdeSx.stato == StatoVerde::S_IN_MANOVRA) {
                gestisciVerdeSinistra();
                return;
            }
            if (statoDoppioVerde.stato == StatoDoppioVerde::S_IN_ROTAZIONE) {
                gestisciDoppioVerde();
                return;
            }
            
            debug.println("Seguendo la linea.");
            if (statoVerdeDx.stato != StatoVerde::S_NORMALE && statoVerdeDx.stato != StatoVerde::S_IN_MANOVRA) 
                statoVerdeDx.reset();
            if (statoVerdeSx.stato != StatoVerde::S_NORMALE && statoVerdeSx.stato != StatoVerde::S_IN_MANOVRA) 
                statoVerdeSx.reset();
            statoVerdeZero.reset();
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
            gestisciNoLinea();
            break;

        default:
            break;
    }
}

void gestisciDoppioVerde() {
    switch (statoDoppioVerde.stato) {
        case StatoDoppioVerde::S_NORMALE:
            // Ignora il doppio verde se è passato meno di 500ms dall'ultimo verde gestito
            if (millis() - ultimoTempoVerde < TEMPO_IGNORA_VERDE) {
                debug.println("Doppio verde ignorato: troppo vicino all'ultimo");
                return;
            }
            
            debug.println("Doppio verde rilevato: mi fermo.");
            motori.stop();
            statoDoppioVerde.stato = StatoDoppioVerde::S_RILEVATO;
            statoDoppioVerde.tempoInizio = millis();
            statoDoppioVerde.contatoreConsecutivo = 0;
            break;
            
        case StatoDoppioVerde::S_RILEVATO:
            // Rimane fermo e verifica che il doppio verde sia ancora presente
            motori.stop();
            
            if (statoLinea() == DOPPIO_VERDE) {
                statoDoppioVerde.contatoreConsecutivo++;
                debug.print("Contatore doppio verde: ");
                debug.println(statoDoppioVerde.contatoreConsecutivo);
                
                if (statoDoppioVerde.contatoreConsecutivo >= CONTATORE_CONFERMA) {
                    debug.println("Doppio verde confermato: eseguo rotazione 180 gradi.");
                    resetPID();
                    motori.muovi(VELOCITA_180, ANGOLO_180);
                    statoDoppioVerde.stato = StatoDoppioVerde::S_IN_ROTAZIONE;
                    statoDoppioVerde.tempoInizio = millis();
                }
            } else {
                // Doppio verde non più presente
                unsigned long tempoTrascorso = millis() - statoDoppioVerde.tempoInizio;
                debug.print("Doppio verde perso, tempo trascorso: ");
                debug.println((int)tempoTrascorso);
                
                if (tempoTrascorso > TIMEOUT_FALSO_VERDE) {
                    debug.println("Falso doppio verde: ripristino normale.");
                    statoDoppioVerde.reset();
                }
            }
            break;
            
        case StatoDoppioVerde::S_IN_ROTAZIONE:
            {
                // Continua la rotazione
                motori.muovi(VELOCITA_180, ANGOLO_180);
                unsigned long tempoRotazione = millis() - statoDoppioVerde.tempoInizio;
                
                if (tempoRotazione > TEMPO_180) {
                    debug.print("Rotazione 180 completata dopo ");
                    debug.print((int)tempoRotazione);
                    debug.println("ms: ripristino normale.");
                    ultimoTempoVerde = millis();  // Aggiorna il tempo dell'ultimo verde gestito
                    statoDoppioVerde.reset();
                    resetPID();
                }
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
            // Ignora il verde se è passato meno di 500ms dall'ultimo verde gestito
            if (millis() - ultimoTempoVerde < TEMPO_IGNORA_VERDE) {
                debug.println("Verde ignorato: troppo vicino all'ultimo");
                return;
            }
            
            debug.print("Verde ");
            debug.print(nomeDir);
            debug.println(" rilevato: mi fermo.");
            motori.stop();
            stato.stato = StatoVerde::S_RILEVATO;
            stato.tempoRilevazione = millis();
            stato.contatoreConsecutivo = 0;
            stato.lineaIniziale = IR_board.line();
            break;

        case StatoVerde::S_RILEVATO:
            // Rimane fermo e verifica che il verde sia ancora presente
            motori.stop();
            
            if (statoLinea() == statoCorrente) {
                stato.contatoreConsecutivo++;
                debug.print("Contatore verde: ");
                debug.println(stato.contatoreConsecutivo);
                
                if (stato.contatoreConsecutivo >= CONTATORE_CONFERMA) {
                    debug.print("Verde ");
                    debug.print(nomeDir);
                    debug.println(" confermato: eseguo manovra.");
                    resetPID();
                    motori.muovi(velocita, angolo);
                    stato.stato = StatoVerde::S_IN_MANOVRA;
                    stato.tempoAvanzamento = millis();
                }
            } else {
                // Verde non più presente
                unsigned long tempoTrascorso = millis() - stato.tempoRilevazione;
                debug.print("Verde perso, tempo trascorso: ");
                debug.println((int)tempoTrascorso);
                
                if (tempoTrascorso > TIMEOUT_FALSO_VERDE) {
                    debug.println("Falso verde: ripristino normale.");
                    stato.reset();
                }
            }
            break;

        case StatoVerde::S_VERIFICA:
            // Stato non più utilizzato, passa direttamente alla manovra
            break;

        case StatoVerde::S_IN_MANOVRA:
            {
                // Continuo la manovra per il tempo stabilito
                motori.muovi(velocita, angolo);
                unsigned long tempoManovra = millis() - stato.tempoAvanzamento;
                
                if (tempoManovra > TEMPO_STERZATA) {
                    debug.print("Manovra completata dopo ");
                    debug.print((int)tempoManovra);
                    debug.println("ms: ripristino normale.");
                    ultimoTempoVerde = millis();  // Aggiorna il tempo dell'ultimo verde gestito
                    stato.reset();
                    resetPID();
                }
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

void gestisciVerdeConLineaZero(int statoIniziale) {
    switch (statoVerdeZero.stato) {
        case StatoVerdeZero::S_NORMALE:
            debug.println("Linea a 0 rilevata: fermo per verificare tipo verde");
            motori.stop();
            statoVerdeZero.stato = StatoVerdeZero::S_FERMO;
            statoVerdeZero.tempoInizio = millis();
            break;
            
        case StatoVerdeZero::S_FERMO:
            // Attesa breve
            if (millis() - statoVerdeZero.tempoInizio >= 200) {
                debug.println("Vado indietro lentamente per verificare tipo verde");
                statoVerdeZero.stato = StatoVerdeZero::S_INDIETRO_VERIFICA;
                statoVerdeZero.tempoInizio = millis();
            }
            break;
            
        case StatoVerdeZero::S_INDIETRO_VERIFICA:
            // Va indietro lentamente mentre verifica lo stato
            motori.muovi(-300, 0);
            
            int statoCorrente = statoLinea();
            int line_pos = IR_board.line();
            
            // Verifica il tipo di verde rilevato
            if (statoCorrente == DOPPIO_VERDE) {
                debug.println("Confermato DOPPIO VERDE - eseguo manovra diretta");
                statoVerdeZero.reset();
                resetPID();
                // Forza direttamente lo stato IN_ROTAZIONE
                statoDoppioVerde.stato = StatoDoppioVerde::S_IN_ROTAZIONE;
                statoDoppioVerde.tempoInizio = millis();
                motori.muovi(VELOCITA_180, ANGOLO_180);
            } else if (statoCorrente == VERDE_SX) {
                debug.println("Confermato VERDE SINISTRA - eseguo manovra diretta");
                statoVerdeZero.reset();
                resetPID();
                // Forza direttamente lo stato IN_MANOVRA
                statoVerdeSx.stato = StatoVerde::S_IN_MANOVRA;
                statoVerdeSx.tempoAvanzamento = millis();
                motori.muovi(VELOCITA_STERZATA, -ANGOLO_STERZATA);
            } else if (statoCorrente == VERDE_DX) {
                debug.println("Confermato VERDE DESTRA - eseguo manovra diretta");
                statoVerdeZero.reset();
                resetPID();
                // Forza direttamente lo stato IN_MANOVRA
                statoVerdeDx.stato = StatoVerde::S_IN_MANOVRA;
                statoVerdeDx.tempoAvanzamento = millis();
                motori.muovi(VELOCITA_STERZATA, ANGOLO_STERZATA);
            } else if (line_pos != 0 || millis() - statoVerdeZero.tempoInizio >= 500) {
                // La linea non è più a 0 o timeout: era solo linea dritta
                debug.println("Era solo linea dritta, riprendo inseguimento");
                statoVerdeZero.reset();
            }
            break;
    }
}

void gestisciNoLinea() {
    switch (statoInterruzione.stato) {
        case StatoInterruzione::S_NORMALE:
            // Prima volta che si rileva NO_LINEA: ferma e inizia la procedura
            debug.println("Linea persa: inizio procedura verifica");
            motori.stop();
            statoInterruzione.stato = StatoInterruzione::S_FERMATO;
            statoInterruzione.tempoInizio = millis();
            break;
            
        case StatoInterruzione::S_FERMATO:
            // Attesa breve dopo il fermo
            if (millis() - statoInterruzione.tempoInizio >= 300) {
                debug.println("Cerco colore avanti");
                statoInterruzione.stato = StatoInterruzione::S_AVANTI_COLORE;
                statoInterruzione.tempoInizio = millis();
                IR_board.setCheckColor(3);
            }
            break;
            
        case StatoInterruzione::S_AVANTI_COLORE:
            // Avanza per 500ms cercando un colore speciale
            if (IR_board.checkColor()) {
                debug.println("Colore rilevato!");
                motori.stop();
                statoInterruzione.reset();
                // Qui potrebbe essere necessario cambiare stato globale
            } else if (millis() - statoInterruzione.tempoInizio >= 500) {
                debug.println("Cerco colore indietro");
                statoInterruzione.stato = StatoInterruzione::S_INDIETRO_COLORE;
                statoInterruzione.tempoInizio = millis();
            } else {
                motori.muovi(400, 0);  // Avanza
            }
            break;
            
        case StatoInterruzione::S_INDIETRO_COLORE:
            // Torna indietro per 500ms cercando un colore speciale
            if (IR_board.checkColor()) {
                debug.println("Colore rilevato!");
                motori.stop();
                statoInterruzione.reset();
            } else if (millis() - statoInterruzione.tempoInizio >= 500) {
                debug.println("Torno indietro a cercare la linea");
                statoInterruzione.stato = StatoInterruzione::S_CERCA_LINEA;
            } else {
                motori.muovi(-400, 0);  // Indietro
            }
            break;
            
        case StatoInterruzione::S_CERCA_LINEA:
            // Continua indietro fino a ritrovare la linea
            if (statoLinea() == LINEA) {
                int line_pos = IR_board.line();
                // Se la linea è nel range centrale, è probabilmente un'interruzione
                if (line_pos >= -500 && line_pos <= 500) {
                    debug.println("Linea in range centrale: interruzione confermata");
                    motori.stop();
                    statoInterruzione.stato = StatoInterruzione::S_AVANZA_INTERRUZIONE;
                } else {
                    debug.println("Linea ritrovata, continuo indietro");
                    statoInterruzione.stato = StatoInterruzione::S_VERIFICA;
                    statoInterruzione.tempoInizio = millis();
                }
            } else {
                motori.muovi(-400, 0);  // Continua indietro
            }
            break;
            
        case StatoInterruzione::S_VERIFICA:
            // Continua indietro per altri 400ms dopo aver ritrovato la linea
            if (millis() - statoInterruzione.tempoInizio >= 400) {
                motori.stop();
                
                // Verifica se è un'interruzione o una rotatoria
                if (IR_board.checkLinea()) {
                    int line_pos = IR_board.line();
                    if (line_pos >= -500 && line_pos <= 500) {
                        debug.println("Interruzione confermata: inizio avanzamento");
                        statoInterruzione.stato = StatoInterruzione::S_AVANZA_INTERRUZIONE;
                    } else {
                        debug.println("Linea fuori range: ripristino");
                        statoInterruzione.reset();
                        resetPID();
                    }
                } else {
                    // Non è un'interruzione: la linea è stata persa
                    // Determina la direzione dell'errore e correggi con velocità maggiore
                    int line_position = IR_board.line();
                    
                    if (line_position <= -1690) {
                        // Errore a sinistra: ruota a sinistra
                        debug.println("Rotatoria/fuori percorso: correzione ampia a SINISTRA");
                        motori.muovi(600, -1750);  // Sterzata ampia a sinistra con velocità maggiore
                    } else if (line_position >= 1690) {
                        // Errore a destra: ruota a destra
                        debug.println("Rotatoria/fuori percorso: correzione ampia a DESTRA");
                        motori.muovi(600, 1750);  // Sterzata ampia a destra con velocità maggiore
                    } else {
                        debug.println("Rotatoria o fuori percorso: ripristino");
                    }
                    
                    statoInterruzione.reset();
                    resetPID();
                }
            } else {
                motori.muovi(-400, 0);  // Continua indietro
            }
            break;
            
        case StatoInterruzione::S_AVANZA_INTERRUZIONE:
            // Avanza fino a quando la linea non ricompare sotto i sensori laterali
            // Controlla anche se ci sono verdi durante l'avanzamento
            if (IR_board.checkGreenDx() || IR_board.checkGreenSx()) {
                debug.println("Verde rilevato durante interruzione: ripristino per gestione verde");
                statoInterruzione.reset();
                resetPID();
                break;
            }
            
            int line_position = IR_board.line();
            
            // Se la linea non è più ai limiti, l'abbiamo ritrovata
            if (line_position != -1750 && line_position != 1750) {
                debug.println("Linea ritrovata dopo interruzione: ripristino normale");
                statoInterruzione.reset();
                resetPID();
            } else {
                // Continua ad avanzare
                motori.muovi(500, 0);
            }
            break;
    }
    
    // Se siamo tornati in LINEA durante la gestione, resetta lo stato (tranne durante avanzamento interruzione)
    if (statoLinea() == LINEA && statoInterruzione.stato != StatoInterruzione::S_CERCA_LINEA 
        && statoInterruzione.stato != StatoInterruzione::S_VERIFICA
        && statoInterruzione.stato != StatoInterruzione::S_AVANZA_INTERRUZIONE) {
        statoInterruzione.reset();
    }
}
