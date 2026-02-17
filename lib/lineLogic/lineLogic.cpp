#include "lineLogic.h"
#include <tofManager.h>
#include "lineLogic_verde.h"
#include "lineLogic_interruzione.h"
#include "lineLogic_ostacolo.h"

// Parametri configurabili per le manovre
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
#define SOGLIA_OSTACOLO         100
#define VELOCITA_OSTACOLO       600

// Riferimenti agli oggetti globali definiti in main.cpp
extern Motori motori;
extern BottomSensor IR_board;
extern MultiClickButton button;
extern tofManager tof_manager;

/**
 * @brief Restituisce lo stato attuale della linea rilevata dai sensori.
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
    statoOstacolo.reset();
}

void gestisciLinea(int stato) {
    // Leggi il sensore frontale per ostacoli
    tof_manager.front.refresh();
    int distanza = tof_manager.front.distance;

    // Se c'è un ostacolo o manovra in corso, ha la priorità assoluta
    if ((distanza > 0 && distanza < SOGLIA_OSTACOLO) || statoOstacolo.stato != StatoOstacolo::S_NORMALE) {
        gestisciOstacolo();
        return;
    }

    // Leggi valori sensori IR
    int line_position = IR_board.line();
    bool verde_dx = IR_board.checkGreenDx();
    bool verde_sx = IR_board.checkGreenSx();
    
    // Flag per sensori verdi con debug RAW
    static bool flag_verde_dx = false;
    static bool flag_verde_sx = false;
    
    if (verde_dx && !flag_verde_dx) {
        debug.println("*** VERDE DX ***");
        flag_verde_dx = true;
    } else if (!verde_dx && flag_verde_dx) {
        flag_verde_dx = false;
    }
    
    if (verde_sx && !flag_verde_sx) {
        debug.println("*** VERDE SX ***");
        flag_verde_sx = true;
    } else if (!verde_sx && flag_verde_sx) {
        flag_verde_sx = false;
    }

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

    // PRIORITÀ ASSOLUTA: Manovre già in corso devono essere completate
    if (statoDoppioVerde.stato == StatoDoppioVerde::S_IN_ROTAZIONE || 
        statoDoppioVerde.stato == StatoDoppioVerde::S_AVANZA_CIECO) {
        gestisciDoppioVerde();
        return;
    }
    
    // PRIORITÀ ALTA: Doppio verde in fase di conferma (S_RILEVATO)
    if (statoDoppioVerde.stato == StatoDoppioVerde::S_RILEVATO) {
        gestisciDoppioVerde();
        return;
    }
    
    if (statoVerdeDx.stato == StatoVerde::S_IN_MANOVRA) {
        gestisciVerdeDestra();
        return;
    }
    
    if (statoVerdeSx.stato == StatoVerde::S_IN_MANOVRA) {
        gestisciVerdeSinistra();
        return;
    }
    
    // PRIORITÀ MASSIMA: Doppio verde ha precedenza su tutto
    if (stato == DOPPIO_VERDE && 
        statoDoppioVerde.stato != StatoDoppioVerde::S_IN_ROTAZIONE && 
        statoDoppioVerde.stato != StatoDoppioVerde::S_AVANZA_CIECO) {
        debug.println("*** DOPPIO VERDE ***");
        // Resetta eventuali stati di verde singolo in corso
        if (statoVerdeDx.stato != StatoVerde::S_NORMALE) {
            statoVerdeDx.reset();
        }
        if (statoVerdeSx.stato != StatoVerde::S_NORMALE) {
            statoVerdeSx.reset();
        }
        gestisciDoppioVerde();
        return;
    }
    
    // Gestione speciale: se la linea è a 0 E non si è in gestione verde, ferma e verifica
    if (line_position == 0 && stato == LINEA && 
        statoVerdeDx.stato == StatoVerde::S_NORMALE && 
        statoVerdeSx.stato == StatoVerde::S_NORMALE) {
        gestisciVerdeConLineaZero(stato);
        return;
    }

    switch (stato) {
        case LINEA:
            // Rallenta se sta confermando un verde o doppio verde
            if (statoVerdeDx.stato == StatoVerde::S_RILEVATO || 
                statoVerdeSx.stato == StatoVerde::S_RILEVATO || 
                statoDoppioVerde.stato == StatoDoppioVerde::S_RILEVATO) {
                pidLineFollowing(VELOCITA_RALLENTA);
            } else {
                if (statoVerdeDx.stato != StatoVerde::S_NORMALE && statoVerdeDx.stato != StatoVerde::S_IN_MANOVRA) 
                    statoVerdeDx.reset();
                if (statoVerdeSx.stato != StatoVerde::S_NORMALE && statoVerdeSx.stato != StatoVerde::S_IN_MANOVRA) 
                    statoVerdeSx.reset();
                statoVerdeZero.reset();
                pidLineFollowing(DEFAULT_VELOCITY);
            }
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
