---

# MultiClickButton Library

<!-- Language Selector -->

[English](#english) | [Italiano](#italiano)

---

## English

Arduino library for advanced button handling with multi-click detection using hardware interrupts.

### Features

* **Single, Double, and Triple Click Detection** - Recognizes different click patterns
* **Automatic Pause on Single Click** - Single click automatically toggles system pause state
* **Hardware Interrupt Based** - Fast and responsive, doesn't block your code
* **Built-in Debouncing** - Eliminates mechanical bounce issues
* **Customizable Callbacks** - Define your own actions for each click type
* **Multiple Button Support** - Up to 4 buttons simultaneously
* **Easy to Use** - Simple API with minimal setup

### Installation

**Method 1: Arduino IDE (Recommended)**

1. Download this library as a ZIP file
2. Open Arduino IDE
3. Go to: **Sketch → Include Library → Add .ZIP Library**
4. Select the downloaded ZIP file
5. Restart Arduino IDE

**Method 2: Manual Installation**

1. Download this library
2. Navigate to your Arduino libraries folder:

   * **Windows:** `Documents\Arduino\libraries\`
   * **Mac:** `~/Documents/Arduino/libraries/`
   * **Linux:** `~/Arduino/libraries/`
3. Extract the `MultiClickButton` folder into the libraries directory
4. Restart Arduino IDE

### Hardware Setup

Connect your button between any digital pin and GND. **No external resistor needed** - the library uses `INPUT_PULLUP` mode.

```
Arduino Pin → [Button] → GND
```

**Example:**

```
Pin 2 → [Push Button] → GND
Pin 3 → [Push Button] → GND (for second button)
```

### Quick Start

```cpp
#include <MultiClickButton.h>

MultiClickButton button(2);  // Button on pin 2

void setup() {
    Serial.begin(9600);
    button.begin();
    
    button.onSingleClick([]() {
        Serial.println(button.isPaused() ? "PAUSED" : "RESUMED");
    });
    
    button.onDoubleClick([]() {
        Serial.println("Double click!");
    });
    
    button.onTripleClick([]() {
        Serial.println("Triple click!");
    });
}

void loop() {
    button.update();
}
```

### API Reference

#### Constructor

```cpp
MultiClickButton(int pin, unsigned long clickTimeout = 500, unsigned long debounceDelay = 50)
```

**Parameters:**

* `pin` - Arduino pin number where button is connected
* `clickTimeout` - Maximum time between clicks in milliseconds (default: 500ms)
* `debounceDelay` - Debounce delay in milliseconds (default: 50ms)

#### Methods

* `void begin()` - Initialize the button, must be called in `setup()`
* `void update()` - Update button state, must be called in `loop()`
* `void onSingleClick(CallbackFunction callback)` - Set callback for single click (auto-pauses system)
* `void onDoubleClick(CallbackFunction callback)` - Set callback for double click
* `void onTripleClick(CallbackFunction callback)` - Set callback for triple click
* `bool isPaused()` - Returns true if system is paused
* `void setPaused(bool paused)` - Manually set pause state
* `void togglePause()` - Toggle pause state
* `int getClickCount()` - Returns current click count

### How It Works

* **Click Detection:** Detects single, double, and triple clicks based on `clickTimeout`
* **Automatic Pause:** Single click toggles pause state automatically
* **Debouncing:** Handles mechanical bounce with configurable `debounceDelay`

### Important Notes

* Always call `update()` in `loop()`
* Callback functions must be `void` with no parameters
* Maximum 4 buttons supported
* Single click auto-pauses

### Compatible Boards

* Arduino Mega 2560
* Arduino Uno
* Arduino Nano
* ESP32
* ESP8266
* Any board with hardware interrupt support

---

## Italiano

Libreria Arduino per la gestione avanzata dei pulsanti con rilevamento di multi-click tramite **interrupt hardware**.

### Caratteristiche

* **Rilevamento Singolo, Doppio e Triplo Click** – Riconosce diversi schemi di click
* **Pausa Automatica al Singolo Click** – Il singolo click attiva/disattiva automaticamente lo stato di pausa
* **Basata su Interrupt Hardware** – Veloce e reattiva, non blocca il codice
* **Debouncing Integrato** – Elimina i problemi di rimbalzo meccanico
* **Callback Personalizzabili** – Definisci azioni diverse per ciascun tipo di click
* **Supporto per Più Pulsanti** – Fino a 4 pulsanti simultaneamente
* **Facile da Usare** – API semplice e setup minimo

### Installazione

**Metodo 1: Arduino IDE (Consigliato)**

1. Scarica la libreria in formato ZIP
2. Apri Arduino IDE
3. Vai su: **Sketch → Include Library → Add .ZIP Library**
4. Seleziona il file ZIP scaricato
5. Riavvia Arduino IDE

**Metodo 2: Installazione Manuale**

1. Scarica la libreria
2. Vai nella cartella delle librerie di Arduino:

   * **Windows:** `Documenti\Arduino\libraries\`
   * **Mac:** `~/Documents/Arduino/libraries/`
   * **Linux:** `~/Arduino/libraries/`
3. Estrai la cartella `MultiClickButton` nella directory delle librerie
4. Riavvia Arduino IDE

### Collegamento Hardware

Collega il pulsante tra un pin digitale e GND. **Non serve resistenza esterna** – la libreria usa la modalità `INPUT_PULLUP`.

```
Pin Arduino → [Pulsante] → GND
```

**Esempio:**

```
Pin 2 → [Pulsante] → GND
Pin 3 → [Pulsante] → GND (per il secondo pulsante)
```

### Avvio Rapido

```cpp
#include <MultiClickButton.h>

MultiClickButton button(2);  // Pulsante sul pin 2

void setup() {
    Serial.begin(9600);
    button.begin();
    
    button.onSingleClick([]() {
        Serial.println(button.isPaused() ? "PAUSA" : "RIPRESA");
    });
    
    button.onDoubleClick([]() {
        Serial.println("Doppio click!");
    });
    
    button.onTripleClick([]() {
        Serial.println("Triplo click!");
    });
}

void loop() {
    button.update();
}
```

### Riferimento API

#### Costruttore

```cpp
MultiClickButton(int pin, unsigned long clickTimeout = 500, unsigned long debounceDelay = 50)
```

**Parametri:**

* `pin` - Numero del pin dove il pulsante è collegato
* `clickTimeout` - Tempo massimo tra i click in millisecondi (default: 500ms)
* `debounceDelay` - Ritardo per il debouncing in millisecondi (default: 50ms)

#### Metodi

* `void begin()` - Inizializza il pulsante, da chiamare in `setup()`
* `void update()` - Aggiorna lo stato del pulsante, da chiamare in `loop()`
* `void onSingleClick(CallbackFunction callback)` - Callback per singolo click (pausa automatica)
* `void onDoubleClick(CallbackFunction callback)` - Callback per doppio click
* `void onTripleClick(CallbackFunction callback)` - Callback per triplo click
* `bool isPaused()` - Restituisce true se il sistema è in pausa
* `void setPaused(bool paused)` - Imposta manualmente lo stato di pausa
* `void togglePause()` - Attiva/disattiva lo stato di pausa
* `int getClickCount()` - Restituisce il numero di click rilevati

### Come Funziona

* **Rilevamento Click:** Rileva singolo, doppio e triplo click basandosi su `clickTimeout`
* **Pausa Automatica:** Il singolo click attiva/disattiva automaticamente la pausa
* **Debouncing:** Gestisce il rimbalzo meccanico con `debounceDelay` configurabile

### Note Importanti

* Chiamare sempre `update()` nel `loop()`
* Le callback devono essere `void` senza parametri
* Supporto massimo di 4 pulsanti
* Il singolo click attiva automaticamente la pausa

### Schede Compatibili

* Arduino Mega 2560
* Arduino Uno
* Arduino Nano
* ESP32
* ESP8266
* Qualsiasi scheda con supporto agli interrupt hardware