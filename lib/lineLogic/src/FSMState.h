#ifndef LINELOGIC_FSMSTATE_H
#define LINELOGIC_FSMSTATE_H

#include <stdint.h>

// ═══════════════════════════════════════════════════════════════════
//  Stati della macchina a stati per il Rescue Line Follower.
//  Tipo uint8_t per minimizzare l'uso di RAM su AVR.
// ═══════════════════════════════════════════════════════════════════

enum FSMState : uint8_t {
    STATE_FOLLOWING          = 0,
    STATE_LINE_LOST_REVERSE  = 1,
    STATE_LINE_LOST_CENTER   = 2,
    STATE_LINE_LOST_FORWARD  = 3,
    STATE_OBSTACLE_BACK      = 4,
    STATE_OBSTACLE_CHOOSE_SIDE = 5,
    STATE_OBSTACLE_BYPASS    = 6,
    STATE_OBSTACLE_REALIGN   = 7,
    STATE_TURN_LEFT          = 8,
    STATE_TURN_RIGHT         = 9,
    STATE_INTERSECTION       = 10,
    STATE_UTURN              = 11
};

#endif
