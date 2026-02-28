#ifndef RESCUELINEFOLLOWER_H
#define RESCUELINEFOLLOWER_H

#include "config.h"
#include "src/FSMState.h"
#include "src/RobotContext.h"
#include "src/GreenManager.h"
#include "src/LineRecovery.h"
#include "src/ObstacleHandler.h"
#include "src/IntersectionHandler.h"
#include "src/UTurnHandler.h"

// ═══════════════════════════════════════════════════════════════════
//  RescueLineFollower — classe principale della macchina a stati
//  per RoboCup Junior Rescue Line 2026.
// ═══════════════════════════════════════════════════════════════════

class RescueLineFollower {
public:
    void begin();
    void update();
    void onRobotReplaced();

private:
    RobotContext _ctx;

    void _transitionTo(FSMState newState, const char* label);
    void _handleFollowing(int16_t pos, uint32_t now);
    void _handleTurn(bool isLeft);
};

#endif
