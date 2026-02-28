#ifndef LINELOGIC_ROBOTCONTEXT_H
#define LINELOGIC_ROBOTCONTEXT_H

#include <stdint.h>
#include "FSMState.h"

// ═══════════════════════════════════════════════════════════════════
//  RobotContext — unico stato condiviso tra tutti i moduli.
//
//  sizeof(RobotContext) stima:
//    1 (state) + 4 (stateStartMs) + 2 (lastLinePos)
//    + 1 (lastTurnWasLeft) + 1 (obstacleBypassLeft)
//    + 2 (greenCntSx+Dx) + 2 (greenConfirmedSx+Dx)
//    + 4 (firstGreenConfMs)
//    + 2 (ignoreSx+Dx) + 4 (postTurnIgnoreMs)
//    + 4 (crossZeroStartMs) + 4 (obstacleBackStartMs)
//    + 4 (lineLostStartMs)
//    = ~35 byte (con padding AVR reale <= 36 byte)
// ═══════════════════════════════════════════════════════════════════

struct RobotContext {
    FSMState  state;              // stato corrente FSM
    uint32_t  stateStartMs;       // millis() all'ingresso nello stato

    int16_t   lastLinePos;        // ultima posizione linea valida (non frozen ±1750)
    bool      lastTurnWasLeft;    // direzione dell'ultima svolta (per riallineamento)
    bool      obstacleBypassLeft; // lato scelto per aggirare l'ostacolo

    // ── Debounce verde (comportamento A e B della scheda slave) ──
    uint8_t   greenCntSx;         // contatore cicli true consecutivi verde SX
    uint8_t   greenCntDx;         // contatore cicli true consecutivi verde DX
    bool      greenConfirmedSx;   // SX ha raggiunto GREEN_CONFIRM_READS
    bool      greenConfirmedDx;   // DX ha raggiunto GREEN_CONFIRM_READS
    uint32_t  firstGreenConfMs;   // millis() del primo lato confermato

    // ── Flag ignore post-svolta ──
    bool      ignoreSxUntilClear; // ignora SX finché il verde fisico sparisce
    bool      ignoreDxUntilClear; // ignora DX finché il verde fisico sparisce
    uint32_t  postTurnIgnoreMs;   // timestamp inizio periodo ignore

    // ── Conferma incrocio ──
    uint32_t  crossZeroStartMs;   // millis() prima volta che line()==0

    // ── Ostacolo ──
    uint32_t  obstacleBackStartMs; // millis() inizio retromarcia ostacolo

    // ── Conferma linea persa ──
    uint32_t  lineLostStartMs;    // millis() inizio rilevamento linea persa
};

#endif
