#ifndef LINELOGIC_INTERSECTIONHANDLER_H
#define LINELOGIC_INTERSECTIONHANDLER_H

#include "RobotContext.h"

// ═══════════════════════════════════════════════════════════════════
//  IntersectionHandler — gestisce STATE_INTERSECTION
// ═══════════════════════════════════════════════════════════════════

// Valuta se siamo su un incrocio (da chiamare solo se nessun verde attivo)
bool intersectionDetected(RobotContext& ctx, uint32_t now);

void intersectionHandler_enter(RobotContext& ctx);
void intersectionHandler_update(RobotContext& ctx);

#endif
