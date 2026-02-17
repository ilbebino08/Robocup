#include "lineLogic_interruzione.h"
#include <sensorBoard.h>
#include <motori.h>
#include <followLine.h>

StatoInterruzione statoInterruzione;

extern Motori motori;
extern BottomSensor IR_board;

void gestisciNoLinea() {
    switch (statoInterruzione.stato) {
        case StatoInterruzione::S_NORMALE:
            debug.println("Linea persa: inizio procedura verifica");
            motori.stop();
            statoInterruzione.stato = StatoInterruzione::S_FERMATO;
            statoInterruzione.tempoInizio = millis();
            break;

        case StatoInterruzione::S_FERMATO:
            if (millis() - statoInterruzione.tempoInizio >= 300) {
                debug.println("Cerco colore avanti");
                statoInterruzione.stato = StatoInterruzione::S_AVANTI_COLORE;
                statoInterruzione.tempoInizio = millis();
                IR_board.setCheckColor(3);
            }
            break;

        case StatoInterruzione::S_AVANTI_COLORE:
            if (IR_board.checkColor()) {
                debug.println("Colore rilevato!");
                motori.stop();
                statoInterruzione.reset();
            } else if (millis() - statoInterruzione.tempoInizio >= 500) {
                debug.println("Cerco colore indietro");
                statoInterruzione.stato = StatoInterruzione::S_INDIETRO_COLORE;
                statoInterruzione.tempoInizio = millis();
            } else {
                motori.muovi(400, 0);
            }
            break;

        case StatoInterruzione::S_INDIETRO_COLORE:
            if (IR_board.checkColor()) {
                debug.println("Colore rilevato!");
                motori.stop();
                statoInterruzione.reset();
            } else if (millis() - statoInterruzione.tempoInizio >= 500) {
                debug.println("Torno indietro a cercare la linea");
                statoInterruzione.stato = StatoInterruzione::S_CERCA_LINEA;
            } else {
                motori.muovi(-400, 0);
            }
            break;

        case StatoInterruzione::S_CERCA_LINEA:
            if (statoLinea() == LINEA) {
                int line_pos = IR_board.line();
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
                motori.muovi(-400, 0);
            }
            break;

        case StatoInterruzione::S_VERIFICA:
            if (millis() - statoInterruzione.tempoInizio >= 400) {
                motori.stop();

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
                    int line_position = IR_board.line();

                    if (line_position <= -1690) {
                        debug.println("Rotatoria/fuori percorso: correzione ampia a SINISTRA");
                        motori.muovi(600, -1750);
                    } else if (line_position >= 1690) {
                        debug.println("Rotatoria/fuori percorso: correzione ampia a DESTRA");
                        motori.muovi(600, 1750);
                    } else {
                        debug.println("Rotatoria o fuori percorso: ripristino");
                    }

                    statoInterruzione.reset();
                    resetPID();
                }
            } else {
                motori.muovi(-400, 0);
            }
            break;

        case StatoInterruzione::S_AVANZA_INTERRUZIONE:
            if (IR_board.checkGreenDx() || IR_board.checkGreenSx()) {
                debug.println("Verde rilevato durante interruzione: ripristino per gestione verde");
                statoInterruzione.reset();
                resetPID();
                break;
            }

            int line_position = IR_board.line();

            if (line_position != -1750 && line_position != 1750) {
                debug.println("Linea ritrovata dopo interruzione: ripristino normale");
                statoInterruzione.reset();
                resetPID();
            } else {
                motori.muovi(500, 0);
            }
            break;
    }

    if (statoLinea() == LINEA && statoInterruzione.stato != StatoInterruzione::S_CERCA_LINEA
        && statoInterruzione.stato != StatoInterruzione::S_VERIFICA
        && statoInterruzione.stato != StatoInterruzione::S_AVANZA_INTERRUZIONE) {
        statoInterruzione.reset();
    }
}
