#ifndef LINELOGIC_UTURNHANDLER_H
#define LINELOGIC_UTURNHANDLER_H

#include "RobotContext.h"

// ═══════════════════════════════════════════════════════════════════
//  UTurnHandler — gestisce STATE_UTURN (doppio verde = inversione U)
// ═══════════════════════════════════════════════════════════════════

void uTurnHandler_enter(RobotContext& ctx);
void uTurnHandler_update(RobotContext& ctx);

#endif
