# Libreria lineLogic

Gestisce la logica di alto livello per l'inseguimento della linea e la gestione degli stati del robot tramite macchina a stati.

## Caratteristiche

- Macchina a stati per gestione intelligente di situazioni complesse
- Rilevamento intersezioni verdi con distinzione pre/post curva
- Gestione interruzioni linea con attraversamento automatico
- Raddrizzamento automatico agli incroci (fix recente)
- Rotazione su doppio verde
- Correzione automatica in caso di perdita linea
- Feedback LED per stato corrente

## Stati Gestiti

### Stati Principali della Linea

- `LINEA`: Inseguimento normale con PID
- `VERDE_SX`: Intersezione verde a sinistra (curva 90° SX)
- `VERDE_DX`: Intersezione verde a destra (curva 90° DX)
- `DOPPIO_VERDE`: Doppio verde rilevato (stop/fine percorso)
- `RADDRIZZAMENTO`: Correzione direzione post-curva (fix commit 9565883)
- `NO_LINEA`: Linea persa (gap detection/recupero)
- `COL_RILEVATO`: Colore speciale rilevato (stop)

### Macchina a Stati per Verde

Gestione intelligente per distinguere verde prima della curva (da seguire) dal verde dopo la curva (da ignorare):

1. `S_NORMALE`: Nessun verde rilevato, inseguimento normale
2. `S_RILEVATO`: Verde appena rilevato, avvia verifica consistenza
3. `S_VERIFICA`: Verifica se verde è pre-curva o post-curva
   - Pre-curva: linea scostata (> SOGLIA_SCOSTAMENTO) → procedi a sterzata
   - Post-curva: linea stabile → ignora verde e continua
4. `S_IN_MANOVRA`: Esecuzione sterzata 90°
5. `S_AVANZA_DOPO`: Avanzamento post-curva, verde ignorato temporaneamente

### Macchina a Stati per Interruzione

Gestione attraversamento gap nella linea:

1. `S_NORMALE`: Nessuna interruzione rilevata
2. `S_FERMATO`: Stop per valutazione situazione
3. `S_AVANTI_COLORE`: Avanza cercando colore speciale
4. `S_INDIETRO_COLORE`: Torna indietro cercando colore speciale
5. `S_CERCA_LINEA`: Retromarcia fino a ritrovare linea
6. `S_VERIFICA`: Verifica tipo (interruzione vs rotatoria)
7. `S_AVANZA_INTERRUZIONE`: Attraversamento gap confermato

Durante l'interruzione controlla continuamente eventuali verdi per gestirli con priorità.

### Gestione Linea a Zero

Quando la linea è perfettamente centrata (posizione 0):

1. `S_NORMALE`: Rilevamento linea centrata
2. `S_FERMO`: Pausa per stabilizzazione
3. `S_INDIETRO_VERIFICA`: Retromarcia lenta per identificare tipo verde

## Feedback LED

- **LED Verde**: Verde rilevato (SX/DX/DOPPIO)
- **LED Rosso**: Interruzione in corso
- **LED Giallo**: Inseguimento linea normale

Pin LED definiti in `include/robot.h`:
- `LED_V` (22): LED verde
- `LED_R` (23): LED rosso
- `LED_G` (24): LED giallo

## API Principali

### `void initLineLogic()`

Inizializza tutti gli stati interni della macchina a stati.

**Chiamare in setup() prima di iniziare il line following.**

**Esempio:**
```cpp
void setup() {
    // ... altri setup ...
    initLineLogic();
}
```

### `int statoLinea()`

Legge i sensori e restituisce lo stato corrente della linea.

**Ritorno:**
- `LINEA`: Linea presente e tracciabile
- `NO_LINEA`: Linea ai limiti estremi (±1750) o persa
- `VERDE_SX`: Marcatore verde a sinistra
- `VERDE_DX`: Marcatore verde a destra
- `DOPPIO_VERDE`: Entrambi i sensori verdi attivi
- `COL_RILEVATO`: Colore speciale rilevato

**Esempio:**
```cpp
int stato = statoLinea();
```

### `void gestisciLinea(int stato)`

Funzione principale di gestione della macchina a stati.

**Parametri:**
- `stato`: Valore ritornato da `statoLinea()`

**Chiamare ad ogni iterazione del loop.**

**Esempio:**
```cpp
void loop() {
    button.update();
    int stato = statoLinea();
    gestisciLinea(stato);
}
```

### Funzioni di Gestione Stati Specifici

**`void gestisciVerdeSinistra()`**  
Gestisce manovra curva a sinistra 90°. Non bloccante.

**`void gestisciVerdeDestra()`**  
Gestisce manovra curva a destra 90° con verifica intelligente pre/post curva.

**`void gestisciDoppioVerde()`**  
Ferma il robot. Usato per fine percorso.

**`void gestisciRaddrizzamento()`**  
Raddrizza il robot dopo una curva verde (fix recente per evitare deviazioni).

**`void gestisciNoLinea()`**  
Gestisce gap detection e attraversamento interruzioni.

## Logica Avanzata

### Distinzione Verde Pre/Post Curva

Il sistema previene falsi positivi distinguendo:

- **Pre-curva**: Linea si scosta significativamente (> `SOGLIA_SCOSTAMENTO`) → esegue sterzata
- **Post-curva**: Linea stabile, verde persistente per `TIMEOUT_FALSO_VERDE` → ignora e continua dritto

Questo evita di curvare due volte sulla stessa intersezione.

### Rilevamento Tipo Interruzione

Una linea è considerata interruzione se:
1. Sensori laterali non rilevano linea (valore ±1750)
2. Dopo retromarcia, sensore frontale rileva linea
3. Linea ritrovata è nel range centrale (-500 a +500)

Se non è interruzione → applica correzione direzionale basata sulla posizione.

### Controllo Verde Durante Interruzione

Durante l'attraversamento, il sistema controlla continuamente eventuali marcatori verdi e interrompe la procedura di gap per gestirli normalmente (priorità alta).

### Raddrizzamento Post-Curva (NUOVO)

Dopo una curva verde, il robot entra nello stato `RADDRIZZAMENTO` che:
- Raddrizza la direzione del robot
- Previene deviazioni dalla traiettoria corretta
- Fix introdotto nel commit 9565883 (30/01/2026)

## Parametri Configurabili

Definiti in `lineLogic.cpp`:

```cpp
VELOCITA_STERZATA   600   // Velocità durante sterzata
ANGOLO_STERZATA     1750  // Angolo sterzata massima  
TEMPO_STERZATA      300   // Durata sterzata (ms)
SOGLIA_SCOSTAMENTO  1500  // Soglia per rilevare verde pre-curva
TEMPO_VERIFICA      250   // Tempo verifica verde (ms)
CONTATORE_CONFERMA  3     // Letture consecutive per conferma
TIMEOUT_FALSO_VERDE 500   // Timeout falsi positivi (ms)
VELOCITA_180        0     // Velocità rotazione 180°
ANGOLO_180          1750  // Angolo rotazione 180°
TEMPO_180           1000  // Durata rotazione 180° (ms)
```

Modifica questi valori per ottimizzare il comportamento del robot.

## Esempio Completo

```cpp
#include <Arduino.h>
#include <sensorBoard.h>
#include <motori.h>
#include <lineLogic.h>
#include <MultiClickButton.h>

BottomSensor IR_board;
Motori motori;
MultiClickButton button(BUTTON);

void singoloClick() {
    // Toggle pausa
}

void setup() {
    Serial.begin(115200);
    
    // Inizializza subsistemi
    IR_board.start();
    motori.init();
    button.begin();
    button.onSingleClick(singoloClick);
    
    // Inizializza logica linea
    initLineLogic();
    
    pinMode(LED_V, OUTPUT);
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
}

void loop() {
    button.update();
    
    int stato = statoLinea();
    gestisciLinea(stato);
    
    delay(10);
}
```

## Note Tecniche

- La libreria dipende da: `sensorBoard`, `motori`, `followLine`, `debug`
- Stati gestiti tramite `switch-case` per efficienza
- Timer basati su `millis()` per operazioni non bloccanti
- Contatori di conferma per evitare falsi positivi
- Isteresi sulle transizioni di stato per stabilità
```
