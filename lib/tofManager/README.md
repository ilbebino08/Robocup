# Libreria `tofManager`

Gestisce un array di sensori di distanza VL53L0X (Time-of-Flight) utilizzando indirizzi I2C univoci.

## Caratteristiche
- Supporto per multipli sensori VL53L0X sullo stesso bus I2C.
- Gestione tramite pin XSHUT per l'inizializzazione sequenziale degli indirizzi.
- **Ottimizzazione RAM**: Memoria ridotta memorizzando solo la distanza come `uint16_t` invece della struttura completa di misurazione.
- **Gestione avanzata**: I sensori sono organizzati in vettori separati per tipo e funzione, migliorando la modularità e la leggibilità del codice.

## Sensori gestiti
Il robot utilizza i seguenti sensori (definiti in `include/robot.h`):
- `front`: Sensore frontale (per ostacoli)
- `antSX` / `antDX`: Sensori anteriori laterali
- `posSX` / `posDX`: Sensori posteriori laterali
- `ball`: Sensore per il rilevamento della pallina

## API Principali

### `bool init()`
Inizializza il bus I2C e assegna indirizzi univoci a tutti i sensori tramite i rispettivi pin XSHUT.
Ritorna `true` se tutti i sensori critici sono inizializzati correttamente.

### `void refreshAll()`
Aggiorna le letture di tutti i sensori contemporaneamente.

### `uint16_t getDistance()` (Classe `tof`)
Restituisce l'ultima distanza misurata in millimetri.

## Esempio d'uso
```cpp
#include <tofManager.h>

void setup() {
    if (tof_manager.init()) {
        debug.println("ToF Inizializzati!");
    }
}

void loop() {
    tof_manager.refreshAll();
    uint16_t dist = tof_manager.front.getDistance();
    if (dist < 100) {
        // Ostacolo rilevato!
    }
}
```
