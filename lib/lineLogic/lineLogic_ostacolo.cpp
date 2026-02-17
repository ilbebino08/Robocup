#include "lineLogic_ostacolo.h"
#include <sensorBoard.h>
#include <motori.h>
#include <followLine.h>

#define SOGLIA_OSTACOLO         100
#define VELOCITA_OSTACOLO       600

StatoOstacolo statoOstacolo;

extern Motori motori;
extern BottomSensor IR_board;

void gestisciOstacolo() {
    int line_pos = IR_board.line();
    unsigned long tempoTrascorso = millis() - statoOstacolo.tempoInizio;

    switch (statoOstacolo.stato) {
        case StatoOstacolo::S_NORMALE:
            debug.println("OSTACOLO RILEVATO: Inizio centramento");
            motori.stop();
            statoOstacolo.stato = StatoOstacolo::S_CENTRAMENTO;
            statoOstacolo.tempoInizio = millis();
            break;

        case StatoOstacolo::S_CENTRAMENTO:
            if (abs(line_pos) > 200) {
                pidLineFollowing(300);
                if (tempoTrascorso > 1000) {
                    debug.println("Timeout centramento: Inizio aggiramento comunque");
                    motori.muovi(VELOCITA_OSTACOLO, 1750);
                    statoOstacolo.stato = StatoOstacolo::S_STERZATA_FUORI;
                    statoOstacolo.tempoInizio = millis();
                }
            } else {
                debug.println("In asse: Inizio aggiramento");
                motori.muovi(VELOCITA_OSTACOLO, 1750);
                statoOstacolo.stato = StatoOstacolo::S_STERZATA_FUORI;
                statoOstacolo.tempoInizio = millis();
            }
            break;

        case StatoOstacolo::S_STERZATA_FUORI:
            motori.muovi(VELOCITA_OSTACOLO, 1750);
            if (tempoTrascorso >= 600) {
                debug.println("Avanza fuori linea");
                motori.muovi(VELOCITA_OSTACOLO, 0);
                statoOstacolo.stato = StatoOstacolo::S_AVANZA_FUORI;
                statoOstacolo.tempoInizio = millis();
            }
            break;

        case StatoOstacolo::S_AVANZA_FUORI:
            motori.muovi(VELOCITA_OSTACOLO, 0);
            if (tempoTrascorso >= 800) {
                debug.println("Sterza parallelo");
                motori.muovi(VELOCITA_OSTACOLO, -1750);
                statoOstacolo.stato = StatoOstacolo::S_STERZATA_DENTRO;
                statoOstacolo.tempoInizio = millis();
            }
            break;

        case StatoOstacolo::S_STERZATA_DENTRO:
            motori.muovi(VELOCITA_OSTACOLO, -1750);
            if (tempoTrascorso >= 700) {
                debug.println("Avanza laterale");
                motori.muovi(VELOCITA_OSTACOLO, 0);
                statoOstacolo.stato = StatoOstacolo::S_AVANZA_LATERALE;
                statoOstacolo.tempoInizio = millis();
            }
            break;

        case StatoOstacolo::S_AVANZA_LATERALE:
            motori.muovi(VELOCITA_OSTACOLO, 0);
            if (tempoTrascorso >= 1200) {
                debug.println("Ricerca linea...");
                motori.muovi(VELOCITA_OSTACOLO, -1200);
                statoOstacolo.stato = StatoOstacolo::S_RICERCA_LINEA;
                statoOstacolo.tempoInizio = millis();
            }
            break;

        case StatoOstacolo::S_RICERCA_LINEA:
            motori.muovi(VELOCITA_OSTACOLO, -1200);
            if (line_pos > -1700 && line_pos < 1700) {
                debug.print("Linea rivista a: ");
                debug.println(line_pos);
                debug.println("Correzione asse");
                motori.muovi(VELOCITA_OSTACOLO, 1750);
                statoOstacolo.stato = StatoOstacolo::S_RIENTRO;
                statoOstacolo.tempoInizio = millis();
            }
            if (tempoTrascorso > 3000) {
                debug.println("Timeout ricerca linea");
                statoOstacolo.reset();
                resetPID();
            }
            break;

        case StatoOstacolo::S_RIENTRO:
            if (abs(line_pos) < 500 || tempoTrascorso > 500) {
                debug.println("Rientro completato");
                statoOstacolo.reset();
                resetPID();
            }
            break;
    }
}
