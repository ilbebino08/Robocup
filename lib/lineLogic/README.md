# Libreria `lineLogic`

Questa libreria gestisce la **logica di stato** per il tracciamento della linea e la gestione delle **intersezioni** (verde) e **ostacoli** durante la gara Robocup Rescue Line Junior.

Il sistema coordina le azioni del robot in base allo stato rilevato dai sensori di linea, delegando l'attuazione alle librerie `motori` e `followLine`.

---

## Hardware e dipendenze

### Dipendenze
- **`sensorBoard`**: per `statoLinea()` e rilevamento verde
- **`motori`**: per comandi diretti (`muovi()`, `stop()`)
- **`followLine`**: per `pidLineFollowing()` con controllo PID
- **`MultiClickButton`**: per gestione pause (`setPaused()`)
- **`debug`**: per log multi-output

### Stati gestiti (da `sensorBoard`)
- `LINEA`: linea nera rilevata normalmente
- `COL_RILEVATO`: ostacolo colore rilevato
- `DOPPIO_VERDE`: intersezione con verde su entrambi i lati
- `VERDE_SX`: verde rilevato a sinistra
- `VERDE_DX`: verde rilevato a destra
- `NO_LINEA`: nessuna linea rilevata

---

## API

### `void initLineLogic()`

Inizializza lo stato interno della libreria.

**Operazioni:**
- Azzera tutti i contatori e stati interni
- Imposta la macchina a stati per il verde a `NORMALE`

**Esempio:**
```cpp
void setup() {
    // ... altre inizializzazioni ...
    initLineLogic();
}
```

---

### `void gestisciLinea(int stato)`

Funzione principale che gestisce lo stato corrente della linea.

**Parametri:**
- `stato`: valore ritornato da `statoLinea()` da `sensorBoard`

**Comportamento:**
Esegue la logica appropriata in base allo stato:
- `LINEA` → segue la linea con PID
- `COL_RILEVATO` → ferma il robot
- `DOPPIO_VERDE` → ferma e mette in pausa
- `VERDE_SX` → esegue curva a sinistra
- `VERDE_DX` → gestione intelligente (vedi sotto)
- `NO_LINEA` → retromarcia di recupero

**Esempio:**
```cpp
void loop() {
    button.update();
    
    if(!button.isPaused()) {
        gestisciLinea(statoLinea());
    }
}
```

---

### `void gestisciDoppioVerde()`

Gestisce l'intersezione con **doppio verde**.

**Operazioni:**
1. Ferma i motori
2. Log messaggio debug
3. Mette il robot in pausa tramite button

**Uso tipico:**
- Fine percorso
- Punto di decisione che richiede input esterno

---

### `void gestisciVerdeSinistra()`

Gestisce la curva a **sinistra** quando rileva verde.

**Operazioni:**
- Avvia manovra di rotazione a sinistra (non bloccante)
- `motori.muovi(-200, 200)` → gira a sinistra

**Nota:**
La funzione è **non bloccante**: avvia solo la manovra e ritorna subito. Il loop successivo continuerà il movimento o passerà ad altro stato.

---

### `void gestisciVerdeDestra()`

Gestisce la curva a **destra** con verifica intelligente.

**Logica a macchina a stati (4 stati interni):**

#### Stato 0: `NORMALE`
- Prima rilevazione verde destra
- **Azione**: rallenta a metà velocità e passa a stato 1

#### Stato 1: `RILEVATO`
- Verifica che il verde sia consistente
- **Conta**: almeno 3 letture consecutive per conferma
- **Timeout**: 500ms → se non confermato = falso positivo → torna a NORMALE
- **Se confermato**: passa a stato 2

#### Stato 2: `VERIFICA`
- Avanza per 250ms mentre segue la linea (rallentato)
- **Verifica posizione verde**:
  - Se **ancora verde dopo 250ms** → verde DOPO la curva → passa a stato 3 (ignora)
  - Se **non più verde** → verde PRIMA della curva → esegue svolta destra

#### Stato 3: `AVANZA_DOPO`
- Continua dritto a velocità normale ignorando il verde post-curva
- **Timeout**: 500ms o quando non vede più verde → torna a NORMALE

**Scopo:**
Distingue il verde **prima** della curva (da seguire) dal verde **dopo** la curva (da ignorare), come richiesto dal regolamento Robocup.

---

## Logica interna

### Stati interni per verde destra
```cpp
enum StatoVerdeDx {
    S_NORMALE = 0,        // Nessun verde rilevato
    S_RILEVATO = 1,       // Verde appena rilevato, in verifica
    S_VERIFICA = 2,       // Verde confermato, verifica posizione
    S_AVANZA_DOPO = 3     // Ignorando verde post-curva
};
```

### Variabili di stato
- `statoVerdeDx`: stato corrente macchina a stati
- `tempoRilevazioneVerde`: timestamp prima rilevazione
- `contatoreVerdeConsecutivo`: contatore per conferma verde
- `tempoAvanzamento`: timestamp per timeout avanzamento

---

## Parametri di configurazione

### Velocità
- `DEFAULT_VELOCITY`: velocità normale (definita in `robot.h`)
- `DEFAULT_VELOCITY / 2`: velocità rallentata per verifica verde

### Temporizzazioni
- **500ms**: timeout per conferma verde consistente
- **250ms**: tempo di avanzamento per verificare posizione verde
- **500ms**: timeout per superare verde post-curva
- **300ms**: delay per iniziare curva destra (da rimuovere per miglior non-bloccanza)

### Comandi motori
- **Curva sinistra**: `motori.muovi(-200, 200)`
- **Curva destra**: `motori.muovi(200, -200)`
- **Retromarcia NO_LINEA**: `motori.muovi(-400, 0)`

---

## Note implementative

### Comportamento non bloccante
La libreria è progettata per essere **non bloccante**: ogni chiamata a `gestisciLinea()` esegue un passo della logica e ritorna immediatamente. Il loop principale continua a chiamare la funzione ad ogni iterazione.

**Eccezione**: il delay di 300ms in `gestisciVerdeDestra()` per la curva dovrebbe essere rimosso in futuro per completa non-bloccanza.

### Reset automatico
Quando il robot torna su `LINEA` normale, lo stato del verde viene automaticamente resettato a `NORMALE`.

### Oggetti esterni richiesti
La libreria usa `extern` per accedere agli oggetti globali:
- `extern Motori motori;`
- `extern BottomSensor IR_board;`
- `extern MultiClickButton button;`

Questi devono essere definiti in `main.cpp`.

---

## Esempio completo

```cpp
#include <Arduino.h>
#include <sensorBoard.h>
#include <motori.h>
#include <MultiClickButton.h>
#include <debug.h>
#include <lineLogic.h>
#include "followLine.h"
#include "robot.h"

BottomSensor IR_board;
Motori motori;
MultiClickButton button(BUTTON);

void setup() {
    Serial.begin(115200);
    debug.begin(DEBUG_USB);
    
    IR_board.start();
    motori.init();
    button.begin();
    
    resetPID();
    initLineLogic();
    
    button.onSingleClick([]() { button.setPaused(false); });
    button.onDoubleClick([]() { button.setPaused(true); });
}

void loop() {
    button.update();
    
    if(!button.isPaused()) {
        gestisciLinea(statoLinea());
    }
}
```

---

## TODO / Miglioramenti futuri

- [ ] Rimuovere il `delay(300)` in `gestisciVerdeDestra()` per completa non-bloccanza
- [ ] Aggiungere parametri di configurazione in header separato per tuning facile
- [ ] Gestire meglio il caso `NO_LINEA` con recovery più sofisticato
- [ ] Aggiungere telemetria per analisi post-gara
- [ ] Implementare strategie diverse per `DOPPIO_VERDE` (checkpoint, fine, ecc.)
