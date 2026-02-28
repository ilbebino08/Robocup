#ifndef LINELOGIC_OBSTACLEHANDLER_H
#define LINELOGIC_OBSTACLEHANDLER_H

#include "RobotContext.h"

// ═══════════════════════════════════════════════════════════════════
//  ObstacleHandler — gestisce OBSTACLE_BACK / CHOOSE_SIDE /
//                    BYPASS / REALIGN
// ═══════════════════════════════════════════════════════════════════

// Controlla se il TOF frontale rileva un ostacolo (non modifica ctx)
bool obstacleDetected();

void obstacleHandler_enter(RobotContext& ctx);
void obstacleHandler_update(RobotContext& ctx);

#endif
