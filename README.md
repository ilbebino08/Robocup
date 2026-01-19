# Progetto RoboCup - Robot Line Follower

Progetto per robot line follower per competizione RoboCup sviluppato per Arduino Mega 2560.

## Descrizione

Il robot è progettato per seguire linee nere, gestire intersezioni con marcatori verdi, rilevare interruzioni di linea e raccogliere oggetti con un braccio meccanico.

## Caratteristiche Principali

- **Inseguimento linea**: PID controller per seguire linee nere con precisione
- **Gestione intersezioni**: Rilevamento e gestione intelligente di marcatori verdi (sinistra, destra, doppio)
- **Interruzioni linea**: Rilevamento e attraversamento automatico di gap nella linea
- **Raccolta oggetti**: Braccio meccanico con servo per presa e rilascio
- **Debug multi-canale**: Output su USB, Bluetooth e SD card
- **Sensori multipli**: Array di sensori IR, sensori di colore, TOF laser

## Struttura del Progetto

### Librerie Principali

- **motori**: Controllo dei 3 motori (sinistro, destro, posteriore) con movimento differenziale
- **sensorBoard**: Gestione array sensori IR e sensori colore
- **lineLogic**: Logica principale per inseguimento linea e gestione stati
- **followLine**: Implementazione algoritmo PID per line following
- **braccio**: Controllo servo per braccio di presa
- **debug**: Sistema di debug multi-output (USB/Bluetooth/SD)
- **MultiClickButton**: Gestione pulsanti con rilevamento click singoli/doppi/tripli

### Hardware

- Arduino Mega 2560
- 3 motori servo per trazione
- Array 8 sensori IR per linea + 1 sensore frontale
- 2 sensori colore AS7341 per rilevamento verde
- 6 sensori TOF VL53L0X per distanze
- Servo per braccio meccanico
- Modulo Bluetooth HC-05/06
- LED RGB per feedback visivo

## Compilazione e Upload

```bash
platformio run --target upload
```

## Configurazione

I parametri principali sono configurabili in:
- `include/robot.h`: Pin mapping e costanti hardware
- `lib/lineLogic/lineLogic.cpp`: Parametri PID e comportamenti
- `lib/followLine/followLine.h`: Costanti PID

## Autori

Team RoboCup 2026
