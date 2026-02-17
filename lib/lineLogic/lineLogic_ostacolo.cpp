#include "lineLogic_ostacolo.h"
#include <sensorBoard.h>
#include <motori.h>
#include <followLine.h>
#include <tofManager.h>

#define SOGLIA_OSTACOLO         50
#define VELOCITA_OSTACOLO       600
#define DISTANZA_COSTEGGIAMENTO 50  // Distanza da mantenere dall'ostacolo (mm)

StatoOstacolo statoOstacolo;

extern Motori motori;
extern BottomSensor IR_board;
extern tofManager tof_manager;

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
            // Torni leggermente indietro per cercare pilastri non visti prima
            if (tempoTrascorso < 400) {
                motori.muovi(-450, 0); // Arretramento per scansionare
                tof_manager.antSX.refresh();
                tof_manager.antDX.refresh();
                return;
            }

            if (abs(line_pos) > 200) {
                pidLineFollowing(300);
                if (tempoTrascorso > 1200) {
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
                // Leggi i sensori posteriori per determinare il lato più libero
                tof_manager.posDX.refresh();
                tof_manager.posSX.refresh();
                
                if (tof_manager.posDX.getDistance() > tof_manager.posSX.getDistance()) {
                    statoOstacolo.latoCosteggiamento = 0; // Costeggia a destra
                    debug.println("Costeggiamento DESTRA");
                } else {
                    statoOstacolo.latoCosteggiamento = 1; // Costeggia a sinistra
                    debug.println("Costeggiamento SINISTRA");
                }
                statoOstacolo.minDistPericolo = 2000;
                statoOstacolo.contatoreLetture = 0;
                debug.println("Avanza fuori linea");
                motori.muovi(VELOCITA_OSTACOLO, 0);
                statoOstacolo.stato = StatoOstacolo::S_AVANZA_FUORI;
                statoOstacolo.tempoInizio = millis();
            }
            break;

        case StatoOstacolo::S_AVANZA_FUORI:
            {
                motori.muovi(VELOCITA_OSTACOLO, 0);
                
                // Monitoraggio dei pilastri laterali mentre si avanza
                // Leggiamo solo il sensore del lato opposto a quello scelto
                if (statoOstacolo.latoCosteggiamento == 0) tof_manager.posSX.refresh();
                else tof_manager.posDX.refresh();
                
                uint16_t distPill = (statoOstacolo.latoCosteggiamento == 0) ? 
                                    tof_manager.posSX.getDistance() : 
                                    tof_manager.posDX.getDistance();
                
                if (distPill < statoOstacolo.minDistPericolo) {
                    statoOstacolo.minDistPericolo = distPill;
                }

                if (tempoTrascorso >= 800) {
                    debug.print("Sterza parallelo. Minimo visto: ");
                    debug.println((int)statoOstacolo.minDistPericolo);
                    
                    motori.muovi(VELOCITA_OSTACOLO, -1750);
                    statoOstacolo.stato = StatoOstacolo::S_STERZATA_DENTRO;
                    statoOstacolo.tempoInizio = millis();
                }
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
            {
                // Scelta sensore principale
                if (statoOstacolo.latoCosteggiamento == 0) tof_manager.posDX.refresh();
                else tof_manager.posSX.refresh();

                uint16_t distGuida = (statoOstacolo.latoCosteggiamento == 0) ? 
                                    tof_manager.posDX.getDistance() : 
                                    tof_manager.posSX.getDistance();

                // Ogni 10 letture (cicli) controlla quello opposto (distanza pericolo)
                statoOstacolo.contatoreLetture++;
                if (statoOstacolo.contatoreLetture >= 10) {
                    if (statoOstacolo.latoCosteggiamento == 0) tof_manager.posSX.refresh();
                    else tof_manager.posDX.refresh();
                    statoOstacolo.contatoreLetture = 0;
                }

                uint16_t distOppposta = (statoOstacolo.latoCosteggiamento == 0) ? 
                                         tof_manager.posSX.getDistance() : 
                                         tof_manager.posDX.getDistance();

                short sterzata = 0;
                
                // Monitora pericoli sul lato opposto (pilastri non visti prima)
                if (distOppposta < statoOstacolo.minDistPericolo) {
                    statoOstacolo.minDistPericolo = distOppposta;
                }

                // GESTIONE COSTEGGIAMENTO INTELLIGENTE
                if (distOppposta < 120) {
                    // Passaggio stretto/Pillastro rilevato dal lato opposto: cerchiamo il centro
                    sterzata = (distGuida - distOppposta) * 4; 
                    if (abs(sterzata) > 1300) sterzata = (sterzata > 0 ? 1300 : -1300);
                    debug.println("CENTRAMENTO ATTIVO (Tunnel/Pilastro)");
                } else {
                    // Costeggiamento normale: manteniamo DISTANZA_COSTEGGIAMENTO
                    int errore = (int)distGuida - DISTANZA_COSTEGGIAMENTO;
                    sterzata = errore * 6; 
                    if (statoOstacolo.latoCosteggiamento == 1) sterzata = -sterzata;
                }

                motori.muovi(450, sterzata); // Velocità ridotta per maggiore precisione

                if (tempoTrascorso >= 1600) { 
                    debug.println("Ricerca linea...");
                    motori.muovi(VELOCITA_OSTACOLO, -1200);
                    statoOstacolo.stato = StatoOstacolo::S_RICERCA_LINEA;
                    statoOstacolo.tempoInizio = millis();
                }
            }
            break;

        case StatoOstacolo::S_RICERCA_LINEA:
            {
                // Calcola direzione di rientro in base al lato costeggiato
                short dirRicerca = (statoOstacolo.latoCosteggiamento == 0) ? -1450 : 1450;
                
                // Sterziamo per incrociare la linea con un angolo di circa 50-60 gradi
                motori.muovi(VELOCITA_OSTACOLO - 100, dirRicerca); 

                // Se rileviamo qualcosa che somiglia a una linea
                if (line_pos > -1750 && line_pos < 1750) {
                    // Se la troviamo troppo presto (es. < 300ms) probabilmente è una linea parallela falsa
                    if (tempoTrascorso < 300) {
                        debug.println("Linea rilevata troppo presto (probabile parallela falsa). Ignoro.");
                    } else {
                        debug.println("Incrocio linea rilevato: inizio verifica...");
                        motori.stop();
                        statoOstacolo.stato = StatoOstacolo::S_VERIFICA_LINEA;
                        statoOstacolo.tempoInizio = millis();
                        statoOstacolo.verificaStep = 0;
                    }
                }
            }
            if (tempoTrascorso > 4500) {
                debug.println("Timeout ricerca linea profonda");
                statoOstacolo.reset();
                resetPID();
            }
            break;

        case StatoOstacolo::S_VERIFICA_LINEA:
            {
                // Logica "Avanti-Indietro" con analisi spaziale e angolare
                if (statoOstacolo.verificaStep == 0) {
                    motori.stop();
                    if (tempoTrascorso > 150) {
                        uint8_t count = 0;
                        for (int i = 0; i < 8; i++) if (IR_board.utils.val_sensorCal(i) > 600) count++;
                        
                        // Se troppi sensori sono attivi (>5), siamo quasi paralleli alla linea
                        // Solitamente dovremmo arrivare a 50-90 gradi (2-4 sensori)
                        if (count > 5) {
                            debug.print("ERRORE: Angolo troppo piatto (Sensori: ");
                            debug.print((int)count);
                            debug.println("). Probabile linea falsa parallela.");
                            statoOstacolo.stato = StatoOstacolo::S_RICERCA_LINEA;
                            statoOstacolo.tempoInizio = millis(); 
                            return;
                        }
                        
                        statoOstacolo.verificaStep = 1;
                        statoOstacolo.tempoInizio = millis();
                    }
                } else if (statoOstacolo.verificaStep == 1) {
                    // Avanza lentamente per "tagliare" la linea
                    motori.muovi(300, 0); 
                    if (tempoTrascorso > 200) {
                        statoOstacolo.verificaStep = 2;
                        statoOstacolo.tempoInizio = millis();
                    }
                } else if (statoOstacolo.verificaStep == 2) {
                    // Arretra per confermare la traccia
                    motori.muovi(-300, 0);
                    if (tempoTrascorso > 300) {
                        statoOstacolo.verificaStep = 3;
                    }
                } else {
                    int sensoriFinali = 0;
                    for (int i = 0; i < 8; i++) {
                        if (IR_board.utils.val_sensorCal(i) > 550) sensoriFinali++;
                    }

                    // Una linea incrociata correttamente a 50-90 gradi attiva solitamente 2-4 sensori
                    if (sensoriFinali >= 2 && sensoriFinali <= 5) {
                        debug.print("Linea CERTIFICATA (Angolo OK): ");
                        debug.println(sensoriFinali);
                        
                        // Manovra di rientro: sterza nel verso opposto alla ricerca per allinearsi
                        short dirRientro = (statoOstacolo.latoCosteggiamento == 0) ? 1750 : -1750;
                        motori.muovi(VELOCITA_OSTACOLO, dirRientro);
                        
                        statoOstacolo.stato = StatoOstacolo::S_RIENTRO;
                        statoOstacolo.tempoInizio = millis();
                    } else {
                        debug.println("Verifica fallita: linea non coerente o angolo errato.");
                        statoOstacolo.stato = StatoOstacolo::S_RICERCA_LINEA;
                        statoOstacolo.tempoInizio = millis();
                    }
                }
            }
            break;

        case StatoOstacolo::S_RIENTRO:
            if (abs(line_pos) < 500 || tempoTrascorso > 700) {
                debug.println("Rientro completato");
                statoOstacolo.reset();
                resetPID();
            }
            break;
    }
}
