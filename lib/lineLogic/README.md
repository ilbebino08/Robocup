# Libreria `lineLogic`

Macchina a stati completa per il seguimento linea nella RoboCup Junior Rescue Line 2026.

## Architettura

FSM (Finite State Machine) non bloccante con 12 stati, suddivisa in moduli indipendenti:
- **GreenManager**: debounce e classificazione eventi verde
- **LineRecovery**: recupero linea persa (retromarcia → centro → avanzamento)
- **ObstacleHandler**: aggiramento ostacoli con scelta laterale
- **IntersectionHandler**: attraversamento incroci
- **UTurnHandler**: inversione a U su doppio verde

## Caratteristiche

- **Zero delay()**: tutto basato su `millis()` e macchina a stati
- **Ottimizzazione RAM**: `RobotContext` ~36 byte, stati come `enum uint8_t`
- **Zero allocazioni dinamiche**: nessun `new`, `malloc` o `String`
- **Costanti centralizzate**: tutte in `config.h`, nessun magic number nei sorgenti
- **Debounce verde robusto**: gestisce falsi positivi singoli e doppio verde letto in ritardo
- **Integrazione trasparente**: usa le API esistenti (`motori`, `sensorBoard`, `tofManager`, `followLine`, `debug`)

## Stati FSM

| Stato | Descrizione |
|---|---|
| `STATE_FOLLOWING` | Seguimento linea normale con PID |
| `STATE_LINE_LOST_REVERSE` | Retromarcia per cercare la linea |
| `STATE_LINE_LOST_CENTER` | Sterzata verso il centro |
| `STATE_LINE_LOST_FORWARD` | Avanzamento lento fino a conferma |
| `STATE_OBSTACLE_BACK` | Retromarcia da ostacolo |
| `STATE_OBSTACLE_CHOOSE_SIDE` | Scelta lato aggiramento |
| `STATE_OBSTACLE_BYPASS` | Aggiramento in corso |
| `STATE_OBSTACLE_REALIGN` | Riallineamento sulla linea |
| `STATE_TURN_LEFT` | Svolta 90° a sinistra (verde singolo SX) |
| `STATE_TURN_RIGHT` | Svolta 90° a destra (verde singolo DX) |
| `STATE_INTERSECTION` | Attraversamento incrocio |
| `STATE_UTURN` | Inversione a U (doppio verde) |

## API

### `void begin()`
Inizializza il contesto FSM e imposta lo stato a `STATE_FOLLOWING`.

### `void update()`
Ciclo principale da chiamare in `loop()`. Aggiorna TOF, verde, linea e smista allo stato corrente.

**Priorità eventi in STATE_FOLLOWING:**
1. Ostacolo (sicurezza fisica)
2. Doppio verde (inversione U)
3. Verde singolo (svolta obbligata)
4. Verde WAITING (attendi finestra)
5. Incrocio (solo se nessun verde)
6. Linea persa (recovery)
7. Following normale (PID)

### `void onRobotReplaced()`
Reset completo dopo riposizionamento manuale del robot.

## Struttura file

```
lib/lineLogic/
├── config.h                  # Tutte le costanti configurabili
├── RescueLineFollower.h      # Header classe principale
├── RescueLineFollower.cpp    # Implementazione FSM
├── library.json              # Metadati PlatformIO
├── README.md
└── src/
    ├── FSMState.h            # Enum stati (uint8_t)
    ├── RobotContext.h        # Struct stato condiviso (~36 byte)
    ├── GreenManager.h/.cpp   # Debounce verde + finestra doppio
    ├── LineRecovery.h/.cpp   # Recovery linea persa
    ├── ObstacleHandler.h/.cpp # Aggiramento ostacoli
    ├── IntersectionHandler.h/.cpp # Incroci
    └── UTurnHandler.h/.cpp   # Inversione a U
```

## Configurazione

Tutte le costanti sono definite in `config.h`:

**Velocità:**
- `BASE_VEL` (500): velocità base following
- `REVERSE_VEL` (-350): retromarcia recovery
- `SEARCH_VEL` (250): avanzamento lento ricerca
- `CROSS_VEL` (350): velocità attraversamento incrocio

**Timeout (ms):**
- `LINE_LOST_CONFIRM_MS` (250): conferma linea persa
- `REVERSE_SEARCH_MS` (600): durata retromarcia recovery
- `GREEN_DOUBLE_WINDOW_MS` (120): finestra attesa doppio verde

**Soglie:**
- `OBSTACLE_DETECT_MM` (120): distanza rilevamento ostacolo
- `GREEN_CONFIRM_READS` (3): cicli consecutivi per confermare verde
- `CENTER_THRESHOLD` (200): soglia posizione per "centrato"

## Esempio d'uso

```cpp
#include <RescueLineFollower.h>

RescueLineFollower lineFollower;

void setup() {
    // ... init motori, sensori, tof ...
    lineFollower.begin();
}

void loop() {
    if (!paused) {
        lineFollower.update();
    }
}
```
