# Libreria sensorBoard

Gestisce la comunicazione con la scheda slave che controlla gli 8 sensori IR di linea e i 2 sensori di colore.

## Architettura

**Hardware:** Scheda slave con microcontroller dedicato  
**Comunicazione:** Serial UART @ 57600 baud (Serial1)  
**Protocollo:** Comandi binari via seriale, risposte strutturate

La scheda slave gestisce autonomamente:
- Lettura array 8 sensori IR
- Calcolo posizione linea (weighted average)
- Rilevamento colore linea (silver/black/red)
- Rilevamento marcatori verdi (left/right)
- Trasmissione dati via seriale

## Funzionalità Principali

### Comunicazione

**`bool start()`**  
Inizializza la comunicazione seriale con la scheda slave.
- Baud rate: 57600
- Delay iniziale: 4000ms per stabilizzazione
- Imposta modalità colore di default (black line)

**Esempio:**
```cpp
BottomSensor IR_board;

void setup() {
    IR_board.start();
}
```

### Rilevamento Linea

**`int16_t line()`**  
Restituisce la posizione della linea rilevata sotto il robot.

**Ritorno:** Posizione linea [-1750, +1750]
- Valori negativi: linea a sinistra
- Zero: linea centrata
- Valori positivi: linea a destra
- **Comportamento perdita linea:** mantiene ultimo valore estremo (±1750)

**Esempio:**
```cpp
int16_t pos = IR_board.line();
if (pos < -500) {
    // Linea molto a sinistra
} else if (pos > 500) {
    // Linea molto a destra
}
```

### Rilevamento Stato

**`bool checkLinea()`**  
Ritorna `true` se è presente una linea davanti al robot.

**Esempio:**
```cpp
if (!IR_board.checkLinea()) {
    // Linea persa, gap detection
}
```

**`bool checkColor()`**  
Ritorna `true` se viene rilevato il colore configurato con `setCheckColor()`.

**Esempio:**
```cpp
IR_board.setCheckColor(3); // Red mode
if (IR_board.checkColor()) {
    // Linea rossa rilevata
}
```

### Rilevamento Verde

**`bool checkGreenSx()`**  
Ritorna `true` se il sensore sinistro rileva verde.

**`bool checkGreenDx()`**  
Ritorna `true` se il sensore destro rileva verde.

**Esempio:**
```cpp
if (IR_board.checkGreenSx()) {
    // Marcatore verde a sinistra, curva 90° SX
}
if (IR_board.checkGreenDx()) {
    // Marcatore verde a destra, curva 90° DX
}
if (IR_board.checkGreenSx() && IR_board.checkGreenDx()) {
    // Doppio verde, fine percorso
}
```

### Configurazione Colore Linea

**`void setCheckColor(uint8_t mode)`**  
Imposta la modalità di ricerca colore della linea.

**Modalità:**
- `1`: Silver line
- `2`: Black line (default)
- `3`: Red line

**Esempio:**
```cpp
IR_board.setCheckColor(2); // Black line mode
```

**`uint8_t getLineMode()`**  
Restituisce la modalità colore linea attuale (1, 2 o 3).

**Esempio:**
```cpp
uint8_t mode = IR_board.getLineMode();
```

## Funzioni Utility (utils)

### Calibrazione

**`bool utils.calibration()`**  
Avvia la calibrazione dei sensori IR.

**Ritorno:** `true` se calibrazione riuscita, `false` altrimenti

**Esempio:**
```cpp
if (IR_board.utils.calibration()) {
    Serial.println("Calibrazione OK");
}
```

**`uint16_t* utils.calibration_val()`**  
Restituisce array con i valori di calibrazione (min e max per ogni sensore).

**Ritorno:** Array di `N_SENSOR*2` elementi (9*2 = 18 valori)

**`void utils.greenSxCalibration()`**  
Calibra il sensore di verde sinistro.

**`void utils.greenDxCalibration()`**  
Calibra il sensore di verde destro.

### Lettura Sensori

**`uint16_t* utils.val_sensor()`**  
Restituisce array con i valori grezzi di tutti i sensori.

**`uint16_t utils.val_sensor(uint8_t n)`**  
Restituisce il valore grezzo del sensore `n` (0-8).

**`uint16_t* utils.val_sensorCal()`**  
Restituisce array con i valori calibrati di tutti i sensori.

**`uint16_t utils.val_sensorCal(uint8_t n)`**  
Restituisce il valore calibrato del sensore `n` (0-8).

**Esempio:**
```cpp
uint16_t sensor_0 = IR_board.utils.val_sensor(0);
uint16_t* all_sensors = IR_board.utils.val_sensorCal();
```

## Status Byte

La libreria mantiene un byte di stato interno che codifica:

```
Bit 7: Verde DX rilevato
Bit 6: Verde SX rilevato  
Bit 5: Linea frontale presente
Bit 4: Colore linea match
Bit 3-2: Modalità colore (1=silver, 2=black, 3=red)
Bit 1-0: Non utilizzati
```

## Esempio Completo

```cpp
#include <sensorBoard.h>

BottomSensor IR_board;

void setup() {
    Serial.begin(115200);
    
    // Inizializza comunicazione con scheda slave
    IR_board.start();
    
    // Imposta modalità black line
    IR_board.setCheckColor(2);
    
    // Calibra sensori (opzionale)
    if (IR_board.utils.calibration()) {
        Serial.println("Calibrazione completata");
    }
}

void loop() {
    // Leggi posizione linea
    int16_t line_pos = IR_board.line();
    
    // Verifica presenza linea
    if (!IR_board.checkLinea()) {
        Serial.println("LINEA PERSA!");
    }
    
    // Controlla verde
    if (IR_board.checkGreenSx()) {
        Serial.println("Verde a sinistra");
    }
    if (IR_board.checkGreenDx()) {
        Serial.println("Verde a destra");
    }
    
    // Stampa posizione
    Serial.print("Posizione: ");
    Serial.println(line_pos);
    
    delay(50);
}
```

## Note Tecniche

- Comunicazione non bloccante con timeout interni
- Tutte le letture passano attraverso `wait_serial()` per sincronizzazione
- I valori vengono trasmessi come 2 byte (MSB + LSB)
- La scheda slave gestisce il timing di lettura sensori