# Libreria LineLogic

Gestisce la logica di alto livello per l'inseguimento della linea e la gestione degli stati del robot.

## Caratteristiche

- **Macchina a stati** per gestione intelligente di situazioni complesse
- **Rilevamento intersezioni verdi** con distinzione pre/post curva
- **Gestione interruzioni linea** con attraversamento automatico
- **Rotazione 180°** su doppio verde
- **Correzione automatica** in caso di perdita linea
- **Feedback LED** per stato corrente
- **Gestione linea centrata** (valore 0) con verifica tipo verde

## Stati Gestiti

### Stati Principali

- **LINEA**: Inseguimento normale con PID
- **VERDE_SX/DX**: Intersezione verde a sinistra/destra
- **DOPPIO_VERDE**: Rotazione 180°
- **COL_RILEVATO**: Colore speciale rilevato (stop)
- **NO_LINEA**: Linea persa, attiva recupero

### Macchina a Stati per Verde

1. **S_NORMALE**: Nessun verde rilevato
2. **S_RILEVATO**: Verde appena rilevato, verifica consistenza
3. **S_VERIFICA**: Verifica se verde è pre o post curva
4. **S_IN_MANOVRA**: Esecuzione sterzata
5. **S_AVANZA_DOPO**: Avanzamento post-curva (verde ignorato)

### Macchina a Stati per Interruzione

1. **S_NORMALE**: Nessuna interruzione
2. **S_FERMATO**: Stop per valutazione
3. **S_AVANTI_COLORE**: Cerca colore speciale avanzando
4. **S_INDIETRO_COLORE**: Cerca colore speciale tornando indietro
5. **S_CERCA_LINEA**: Retromarcia fino a ritrovare linea
6. **S_VERIFICA**: Verifica tipo (interruzione vs rotatoria)
7. **S_AVANZA_INTERRUZIONE**: Attraversamento interruzione

### Gestione Linea a Zero

Quando la linea è perfettamente centrata (valore 0):
1. **S_NORMALE**: Rilevamento linea a 0
2. **S_FERMO**: Pausa per stabilizzazione
3. **S_INDIETRO_VERIFICA**: Retromarcia lenta per identificare tipo verde

## Feedback LED

- **LED Verde**: Verde rilevato (SX/DX/DOPPIO)
- **LED Rosso**: Interruzione in corso
- **LED Giallo**: Inseguimento linea normale

## Parametri Configurabili

```cpp
#define VELOCITA_STERZATA   600   // Velocità durante sterzata
#define ANGOLO_STERZATA     1750  // Angolo sterzata massima
#define TEMPO_STERZATA      300   // Durata sterzata (ms)
#define SOGLIA_SCOSTAMENTO  1500  // Soglia per rilevare verde pre-curva
#define TEMPO_VERIFICA      250   // Tempo verifica verde (ms)
#define CONTATORE_CONFERMA  3     // Letture consecutive per conferma
#define TIMEOUT_FALSO_VERDE 500   // Timeout falsi positivi (ms)
#define VELOCITA_180        0     // Velocità rotazione 180°
#define ANGOLO_180          1750  // Angolo rotazione 180°
#define TEMPO_180           1000  // Durata rotazione 180° (ms)
```

## API Principali

### `void initLineLogic()`
Inizializza tutti gli stati interni. Chiamare in `setup()`.

### `int statoLinea()`
Restituisce lo stato corrente della linea:
- `LINEA`: Linea presente
- `NO_LINEA`: Linea ai limiti (-1750/1750)
- `VERDE_SX/DX/DOPPIO_VERDE`: Intersezioni
- `COL_RILEVATO`: Colore speciale

### `void gestisciLinea(int stato)`
Funzione principale di gestione. Chiamare nel loop con: `gestisciLinea(statoLinea())`

## Logica Avanzata

### Distinzione Verde Pre/Post Curva

Il sistema distingue se un verde è prima o dopo una curva:
- **Pre-curva**: Linea si scosta significativamente → esegue sterzata
- **Post-curva**: Linea stabile, verde persistente → ignora e continua dritto

### Rilevamento Interruzione

Una linea è considerata interruzione se:
1. Sensori laterali non rilevano linea (valore -1750/1750)
2. Dopo retromarcia, sensore frontale rileva linea
3. Linea ritrovata è nel range centrale (-500 a +500)

Se non è interruzione, applica correzione direzionale in base alla posizione.

### Controllo Verdi Durante Interruzione

Durante l'attraversamento di un'interruzione, il sistema controlla continuamente se rileva verdi e interrompe la procedura per gestirli normalmente.

## Esempio d'uso

```cpp
#include <lineLogic.h>

void setup() {
    // ... inizializza sensori, motori, ecc ...
    initLineLogic();
}

void loop() {
    button.update();  // Aggiorna stato pulsante
    int stato = statoLinea();
    gestisciLinea(stato);
}
```
