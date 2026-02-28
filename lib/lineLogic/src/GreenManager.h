#ifndef LINELOGIC_GREENMANAGER_H
#define LINELOGIC_GREENMANAGER_H

#include <stdint.h>
#include "RobotContext.h"

// ═══════════════════════════════════════════════════════════════════
//  GreenManager — debounce e classificazione eventi verde.
//  Unico punto che legge checkGreenSx()/checkGreenDx().
// ═══════════════════════════════════════════════════════════════════

enum GreenEvent : uint8_t {
    GREEN_NONE       = 0,
    GREEN_SINGLE_SX  = 1,
    GREEN_SINGLE_DX  = 2,
    GREEN_DOUBLE     = 3,
    GREEN_WAITING    = 4   // finestra doppio-verde ancora aperta
};

// Aggiorna contatori e classificazione (UNA volta per ciclo)
void greenManager_update(RobotContext& ctx);

// Restituisce evento classificato nell'ultima update() (non rilegge HW)
GreenEvent greenManager_getEvent(const RobotContext& ctx);

// Resetta tutti i campi verde del contesto
void greenManager_reset(RobotContext& ctx);

// Post-svolta: imposta flag ignore sul lato appropriato
void greenManager_setIgnoreAfterTurn(RobotContext& ctx, bool wasTurnLeft);

#endif
