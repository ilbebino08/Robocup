# RoboCup 2026 - Robot Line Follower

Codice scheda master del robot Mt20-16 per RoboCup Junior Rescue Line.

## Descrizione

Robot line follower omnidirezionale a 3 ruote progettato per la categoria RoboCup Junior Rescue Line. Il robot è capace di:

- Seguire linee nere con precisione usando un controllore PID
- Rilevare e gestire intersezioni con marcatori verdi (sinistra, destra, doppio verde)
- Attraversare interruzioni di linea (gap detection)
- Raddrizzarsi automaticamente agli incroci per evitare deviazioni
- Raccogliere e rilasciare oggetti con braccio meccanico
- Debug in tempo reale via USB e Bluetooth

## Architettura del Progetto

### Struttura Directory

```
Robocup/
├── src/                  # Codice principale
│   └── main.cpp          # Entry point e loop principale
├── include/              # Header globali
│   └── robot.h           # Configurazione pin e costanti
├── lib/                  # Librerie custom
│   ├── motori/           # Controllo motori omnidirezionali
│   ├── sensorBoard/      # Gestione array sensori IR e colore
│   ├── lineLogic/        # Logica stati e decisioni
│   ├── followLine/       # Algoritmo PID per line following
│   ├── braccio/          # Controllo servo braccio/pinza
│   ├── debug/            # Sistema debug multi-canale
│   └── MultiClickButton/ # Gestione pulsante con multi-click
└── platformio.ini        # Configurazione build e dipendenze
```

### Librerie Principali

| Libreria | Funzione |
|----------|----------|
| **motori** | Controllo 3 motori servo (2 anteriori + 1 posteriore omnidirezionale) |
| **sensorBoard** | Lettura array 8 sensori IR + 2 sensori colore AS7341 |
| **lineLogic** | Macchina a stati per gestione linea, intersezioni, verde |
| **followLine** | Controllore PID per calcolo errore e correzione traiettoria |
| **braccio** | Controllo 2 servo (braccio + pinza) per raccolta oggetti |
| **debug** | Output simultaneo su USB, Bluetooth, SD card |
| **MultiClickButton** | Rilevamento click singoli/doppi/tripli con debounce |

## Hardware

### Componenti Principali

- **Microcontroller**: Arduino Mega 2560 (ATmega2560)
- **Motori**: 3x servo a rotazione continua (2 anteriori + 1 posteriore sterzo)
- Microcontroller: Arduino Mega 2560 (ATmega2560)
- Motori: 3x servo a rotazione continua (2 anteriori + 1 posteriore sterzo)
- IR_Board
- Sensori distanza: 6x VL53L0X Time-of-Flight laser (ostacoli/vittime)
- Braccio: 2x micro servo (shoulder + gripper)
- Feedback: LED RGB (rosso, verde, giallo) per stato robot
- Pulsante di interrupt

Vedi [include/robot.h](include/robot.h) per il mapping completo dei pin e i valori di calibrazione dei motori.

## Funzionalità Avanzate

### Gestione Intersezioni Verdi

Il robot implementa una **macchina a stati** per distinguere il verde prima della curva (da seguire) dal verde dopo la curva (da ignorare):

- **VERDE_SX**: Macchina a stati per distinguere il verde prima della curva (da seguire) dal verde dopo la curva (da ignorare):

- VERDE_SX: Rotazione a sinistra di 90°
- VERDE_DX: Rotazione a destra di 90° con verifica intelligente
- DOPPIO_VERDE agli Incroci

Feature recente (commit `9565883`): il robot si raddrizza automaticamente quando rileva un incrocio verde, evitando di deviare dalla traiettoria corretta.

### Controllore PID

Parametri PID ottimizzati per line following fluido:
- Kp: Guadagno proporzionale (risposta immediata)
- Ki: Guadagno integrale (correzione offset)
- Kd: Guadagno derivativo (smoothing e anticipazione)

Modifica i parametri in [lib/followLine/followLine.h](lib/followLine/followLine.h) per ottimizzare le prestazioni.

## Debug & Testing

### Monitor Seriale

Monitor seriale a 115200 baud

### Debug Bluetooth

Modulo Bluetooth (default: 57600 baud).

## Calibrazione Motori

Modifica in [include/robot.h](include/robot.h):
- `MSX_MIN`, `MSX_MAX`: Range motore sinistro
- `MDX_MIN`, `MDX_MAX`: Range motore destro  
- `MPO_MIN`, `MPO_MAX`: Range motore posteriore
- `MSX_INV`, `MDX_INV`, `MPO_INV`: Flag inversione direzione

### Tuning PID

Modifica in [lib/followLine/followLine.h](lib/followLine/followLine.h):
- Aumenta `Kp` per risposta più aggressiva
- Aumenta `Kd` per maggiore stabilità
- Aggiungi `Ki` solo se necessario (drift correction)

### Soglie Sensori

Modifica in [lib/lineLogic/lineLogic.cpp](lib/lineLogic/lineLogic.cpp):
- Soglie rilevamento verde
- Timeout gap detection
- Durate rotazioni

---

## Team

Team RoboCup 2026 - Mt20-16