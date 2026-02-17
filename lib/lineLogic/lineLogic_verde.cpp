#include "lineLogic_verde.h"
#include <sensorBoard.h>
#include <followLine.h>
#include <motori.h>

// Parametri per verdi
#define VELOCITA_STERZATA       1000
#define ANGOLO_STERZATA         1000
#define TEMPO_STERZATA          300
#define SOGLIA_SCOSTAMENTO      1500
#define TEMPO_VERIFICA          250
#define CONTATORE_CONFERMA      3
#define TIMEOUT_FALSO_VERDE     500
#define TEMPO_IGNORA_VERDE      2000
#define TEMPO_IGNORA_DOPPIO_VERDE 500
#define VELOCITA_180            0
#define ANGOLO_180              1750
#define TEMPO_180               2000
#define VELOCITA_AVANZA_CIECO   800
#define TEMPO_AVANZA_CIECO      1000
#define VELOCITA_RALLENTA       400
#define VELOCITA_VERIFICA       300
#define TEMPO_VERIFICA_AVANTI   200
#define TEMPO_VERIFICA_INDIETRO 200

// Istanze di stato
StatoVerde statoVerdeDx;
StatoVerde statoVerdeSx;
StatoDoppioVerde statoDoppioVerde;
StatoVerdeZero statoVerdeZero;
uint16_t ultimoTempoVerde = 0;
uint16_t ultimoTempoDoppioVerde = 0;

// Riferimenti agli oggetti globali
extern Motori motori;
extern BottomSensor IR_board;
extern int statoLinea();
extern void resetPID();

void gestisciDoppioVerde() {
    switch (statoDoppioVerde.stato) {
        case StatoDoppioVerde::S_NORMALE:
            if (millis() - ultimoTempoDoppioVerde < TEMPO_IGNORA_DOPPIO_VERDE) {
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
            motori.muovi(VELOCITA_180, ANGOLO_180);
            unsigned long tempoRotazione = millis() - statoDoppioVerde.tempoInizio;

            if (tempoRotazione >= TEMPO_180) {
                debug.print("Rotazione 180 completata dopo ");
                debug.print((int)tempoRotazione);
                debug.println("ms: avanzamento cieco.");
                ultimoTempoDoppioVerde = millis();
                statoDoppioVerde.stato = StatoDoppioVerde::S_AVANZA_CIECO;
                statoDoppioVerde.tempoInizio = millis();
                resetPID();
            }
        }
            break;

        case StatoDoppioVerde::S_AVANZA_CIECO:
        {
            motori.muovi(VELOCITA_AVANZA_CIECO, 0);
            unsigned long tempoAvanzamento = millis() - statoDoppioVerde.tempoInizio;

            if (tempoAvanzamento >= TEMPO_AVANZA_CIECO) {
                debug.println("Avanzamento cieco completato: ripristino normale.");
                statoDoppioVerde.reset();
                resetPID();
            }
        }
            break;
    }
}

void gestisciVerdeGenerico(StatoVerde& stato, int statoCorrente, int velocita, int angolo, const char* nomeDir) {
    switch (stato.stato) {
        case StatoVerde::S_NORMALE:
            if (millis() - ultimoTempoVerde < TEMPO_IGNORA_VERDE) {
                debug.println("Verde ignorato: troppo vicino all'ultimo");
                return;
            }

            debug.print("Verde ");
            debug.print(nomeDir);
            debug.println(" rilevato: inizio verifica.");
            motori.stop();
            stato.stato = StatoVerde::S_RILEVATO;
            stato.tempoRilevazione = millis();
            stato.contatoreConsecutivo = 0;
            stato.doppioVerdeRilevato = false;
            stato.lineaIniziale = IR_board.line();
            break;

        case StatoVerde::S_RILEVATO:
            motori.stop();

            if (statoLinea() == DOPPIO_VERDE) {
                stato.doppioVerdeRilevato = true;
                debug.println("DOPPIO VERDE rilevato durante conferma!");
            }

            if (statoLinea() == statoCorrente || stato.doppioVerdeRilevato) {
                stato.contatoreConsecutivo++;
                debug.print("Contatore verde: ");
                debug.println(stato.contatoreConsecutivo);

                if (stato.contatoreConsecutivo >= CONTATORE_CONFERMA) {
                    debug.println("Verde confermato: inizio verifica avanti-indietro.");
                    stato.stato = StatoVerde::S_VERIFICA_AVANTI;
                    stato.tempoAvanzamento = millis();
                }
            } else {
                unsigned long tempoTrascorso = millis() - stato.tempoRilevazione;
                debug.print("Verde perso, tempo trascorso: ");
                debug.println((int)tempoTrascorso);

                if (tempoTrascorso > TIMEOUT_FALSO_VERDE) {
                    debug.println("Falso verde: ripristino normale.");
                    stato.reset();
                }
            }
            break;

        case StatoVerde::S_VERIFICA_AVANTI:
            motori.muovi(VELOCITA_VERIFICA, 0);

            if (statoLinea() == DOPPIO_VERDE) {
                stato.doppioVerdeRilevato = true;
                debug.println("DOPPIO VERDE rilevato durante verifica avanti!");
            }

            if (millis() - stato.tempoAvanzamento >= TEMPO_VERIFICA_AVANTI) {
                debug.println("Verifica avanti completata: indietreggio.");
                stato.stato = StatoVerde::S_VERIFICA_INDIETRO;
                stato.tempoAvanzamento = millis();
            }
            break;

        case StatoVerde::S_VERIFICA_INDIETRO:
            motori.muovi(-VELOCITA_VERIFICA, 0);

            if (statoLinea() == DOPPIO_VERDE) {
                stato.doppioVerdeRilevato = true;
                debug.println("DOPPIO VERDE rilevato durante verifica indietro!");
            }

            if (millis() - stato.tempoAvanzamento >= TEMPO_VERIFICA_INDIETRO) {
                debug.println("Verifica indietro completata.");
                motori.stop();

                if (stato.doppioVerdeRilevato) {
                    debug.println("Doppio verde confermato: passo al gestore doppio verde.");
                    stato.reset();
                    statoDoppioVerde.stato = StatoDoppioVerde::S_RILEVATO;
                    statoDoppioVerde.tempoInizio = millis();
                    statoDoppioVerde.contatoreConsecutivo = 0;
                } else {
                    debug.print("Verde ");
                    debug.print(nomeDir);
                    debug.println(" singolo: eseguo manovra.");
                    resetPID();
                    motori.muovi(velocita, angolo);
                    stato.stato = StatoVerde::S_IN_MANOVRA;
                    stato.tempoAvanzamento = millis();
                }
            }
            break;

        case StatoVerde::S_IN_MANOVRA:
        {
            motori.muovi(velocita, angolo);
            unsigned long tempoManovra = millis() - stato.tempoAvanzamento;

            if (tempoManovra >= TEMPO_STERZATA) {
                debug.print("Manovra completata dopo ");
                debug.print((int)tempoManovra);
                debug.println("ms: ripristino normale.");
                ultimoTempoVerde = millis();
                stato.reset();
                resetPID();
            }
        }
            break;

        case StatoVerde::S_AVANZA_DOPO:
            pidLineFollowing(DEFAULT_VELOCITY);
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
            if (millis() - statoVerdeZero.tempoInizio >= 200) {
                debug.println("Vado indietro lentamente per verificare tipo verde");
                statoVerdeZero.stato = StatoVerdeZero::S_INDIETRO_VERIFICA;
                statoVerdeZero.tempoInizio = millis();
            }
            break;

        case StatoVerdeZero::S_INDIETRO_VERIFICA:
            motori.muovi(-300, 0);

            int statoCorrente = statoLinea();
            int line_pos = IR_board.line();

            if (statoCorrente == DOPPIO_VERDE) {
                debug.println("Confermato DOPPIO VERDE - eseguo manovra diretta");
                statoVerdeZero.reset();
                resetPID();
                statoDoppioVerde.stato = StatoDoppioVerde::S_IN_ROTAZIONE;
                statoDoppioVerde.tempoInizio = millis();
                motori.muovi(VELOCITA_180, ANGOLO_180);
            } else if (statoCorrente == VERDE_SX) {
                debug.println("Confermato VERDE SINISTRA - eseguo manovra diretta");
                statoVerdeZero.reset();
                resetPID();
                statoVerdeSx.stato = StatoVerde::S_IN_MANOVRA;
                statoVerdeSx.tempoAvanzamento = millis();
                motori.muovi(VELOCITA_STERZATA, -ANGOLO_STERZATA);
            } else if (statoCorrente == VERDE_DX) {
                debug.println("Confermato VERDE DESTRA - eseguo manovra diretta");
                statoVerdeZero.reset();
                resetPID();
                statoVerdeDx.stato = StatoVerde::S_IN_MANOVRA;
                statoVerdeDx.tempoAvanzamento = millis();
                motori.muovi(VELOCITA_STERZATA, ANGOLO_STERZATA);
            } else if (line_pos != 0 || millis() - statoVerdeZero.tempoInizio >= 500) {
                debug.println("Era solo linea dritta, riprendo inseguimento");
                statoVerdeZero.reset();
            }
            break;
    }
}
