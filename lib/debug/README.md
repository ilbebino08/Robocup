# Libreria Debug

Libreria per la gestione flessibile del debug con supporto per multipli output simultanei.

## Caratteristiche

- **Output multipli**: USB Serial, Bluetooth, SD Card
- **Configurazione flessibile**: Abilita/disabilita output dinamicamente
- **API semplice**: Simile a Serial (print, println, printf)
- **Non bloccante**: Se un output non è disponibile, continua con gli altri

## Utilizzo Base

### Esempio 1: Solo USB Serial
```cpp
#include <debug.h>

void setup() {
    Serial.begin(115200);
    debug.begin(DEBUG_USB);
    
    debug.println("Hello World!");
}
```

### Esempio 2: USB + Bluetooth
```cpp
#include <debug.h>

void setup() {
    Serial.begin(115200);
    debug.begin(DEBUG_USB | DEBUG_BLUETOOTH);
    debug.setBluetoothSerial(&Serial1); // Usa Serial1 per Bluetooth
    
    debug.println("Messaggio su USB e Bluetooth!");
}
```

### Esempio 3: USB + SD Card
```cpp
#include <debug.h>

void setup() {
    Serial.begin(115200);
    debug.begin(DEBUG_USB | DEBUG_SD);
    debug.setSDCard("log.txt", 10); // Pin CS = 10
    
    debug.println("Messaggio su USB e SD!");
}
```

### Esempio 4: Tutti gli output
```cpp
#include <debug.h>

void setup() {
    Serial.begin(115200);
    debug.begin(DEBUG_ALL); // Abilita tutti
    debug.setBluetoothSerial(&Serial1);
    debug.setSDCard("log.txt", 10);
    
    debug.println("Messaggio su tutti gli output!");
}
```

## API Completa

### Inizializzazione
- `debug.begin(outputs)` - Inizializza con maschera di output
- `debug.setBluetoothSerial(serial)` - Configura Bluetooth
- `debug.setSDCard(fileName, csPin)` - Configura SD card

### Output
- `debug.print(message)` - Stampa senza newline
- `debug.println(message)` - Stampa con newline
- `debug.printf(format, ...)` - Stampa formattata (stile printf)

### Gestione Output
- `debug.setOutput(output, enable)` - Abilita/disabilita output specifico
- `debug.flush()` - Forza scrittura su SD card

## Costanti

- `DEBUG_USB` - Output USB Serial
- `DEBUG_BLUETOOTH` - Output Bluetooth
- `DEBUG_SD` - Output SD Card
- `DEBUG_ALL` - Tutti gli output

## Note

- Gli output possono essere combinati con l'operatore OR bitwise (`|`)
- Se Serial non è inizializzato prima di `debug.begin()`, viene inizializzato automaticamente a 115200 baud
- I messaggi su SD vengono scritti in modalità append
- Chiamare `debug.flush()` periodicamente per assicurarsi che i dati vengano scritti su SD
