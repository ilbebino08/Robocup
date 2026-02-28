#ifndef LINELOGIC_LINERECOVERY_H
#define LINELOGIC_LINERECOVERY_H

#include "RobotContext.h"

// ═══════════════════════════════════════════════════════════════════
//  LineRecovery — gestisce STATE_LINE_LOST_REVERSE / CENTER / FORWARD
// ═══════════════════════════════════════════════════════════════════

void lineRecovery_enter(RobotContext& ctx);
void lineRecovery_update(RobotContext& ctx);

#endif
