# Libreria `braccio`

Questa libreria gestisce il **braccio robotico** e la **pinza** per la manipolazione della palla nel robot Robocup.

Il sistema è composto da:
- **1 servo braccio** (`BRACCIO_PIN`) per il movimento verticale
- **1 servo pinza** (`PINZA_PIN`) per apertura/chiusura
- **1 sensore digitale** (`SENS_PINZA`) per rilevare la presenza della palla

---

## Hardware

### Pin utilizzati (da `include/robot.h`)
- `BRACCIO_PIN` (39): servo controllo braccio
- `PINZA_PIN` (28): servo controllo pinza
- `SENS_PINZA` (31): sensore digitale (HIGH = palla presente)

### Posizioni predefinite
- `BRACCIO_RIP` (570): posizione di riposo del braccio
- `PINZA_RIP` (600): posizione di riposo della pinza
- `BRACCIO_AVANTI` (1500): posizione braccio per affrontare salite
- Hardcoded: `74°` = braccio abbassato (abbassa baricentro)

---

## API

### `void init()`

Inizializza il sistema braccio/pinza.

**Sequenza operazioni:**
1. Configura `SENS_PINZA` come `INPUT`
2. Attacca i servo ai pin (`BRACCIO_PIN`, `PINZA_PIN`)
3. Delay 50ms per stabilizzazione servo
4. Posiziona braccio e pinza in posizione di riposo
5. Delay 100ms per completamento movimento

**Esempio:**
```cpp
braccio.init();
```

---

### `void riposo()`

Porta braccio e pinza in **posizione di riposo** (configurazione iniziale/attesa).

**Operazioni:**
- `braccio.write(BRACCIO_RIP)` → 570 (angolo servo standard 0-180)
- `pinza.write(PINZA_RIP)` → 600

**Uso tipico:**
- Dopo aver completato un'azione
- Prima di iniziare una nuova strategia
- Durante fase di stop/attesa

**Esempio:**
```cpp
braccio.riposo();
```

---

### `void abbassato()`

Abbassa il braccio a **74°** per **abbassare il baricentro** del robot.

**Scopo:**
- Migliora stabilità durante movimenti rapidi
- Utile in curve strette o accelerazioni

**Nota:** valore hardcoded nel `.cpp` (non usa costante da `robot.h`)

**Esempio:**
```cpp
braccio.abbassato();
```

---

### `bool prendiPallina()`

Verifica se la **palla è presente** nella pinza tramite il sensore.

**Ritorno:**
- `true` se `SENS_PINZA == HIGH` (palla rilevata)
- `false` altrimenti

**Nota implementativa:**
> Nel codice attuale c'è un commento `@todo Implementare la logica per prendere una pallina`.  
> La funzione **non esegue** movimenti servo, solo lettura sensore.  
> Per una sequenza completa "prendi palla" servirebbe:
> 1. Aprire pinza
> 2. Abbassare braccio
> 3. Chiudere pinza
> 4. Verificare con `prendiPallina()`
> 5. Alzare braccio

**Esempio (uso attuale):**
```cpp
if (braccio.prendiPallina()) {
    // Palla afferrata
    Serial.println("Palla in pinza!");
} else {
    // Pinza vuota
}
```

---

## Esempio d'uso completo

```cpp
#include <braccio.h>

Braccio braccio;

void setup() {
    braccio.init();          // Inizializza e va in riposo
    delay(1000);
    
    braccio.abbassato();     // Abbassa per stabilità
    delay(500);
    
    // TODO: implementare sequenza apertura/chiusura pinza
    
    if (braccio.prendiPallina()) {
        // Palla presa
        braccio.riposo();    // Torna in posizione sicura
    }
}

void loop() {
    // ...logica principale...
}
```

---

## Note di sviluppo

### Funzionalità mancanti (da implementare)
La libreria attuale fornisce solo controlli base. Per un sistema completo servirebbero:

1. **Controllo pinza:**
   ```cpp
   void apriPinza();
   void chiudiPinza();
   ```

2. **Posizioni braccio:**
   ```cpp
   void posizioneRaccolta();  // Per avvicinarsi alla palla
   void posizioneTrasporto(); // Durante movimento
   void posizioneRilascio();  // Per depositare
   ```

3. **Sequenze complete:**
   ```cpp
   bool sequenzaRaccolta();   // Apri → abbassa → chiudi → verifica
   void sequenzaRilascio();   // Abbassa → apri → alza
   ```

### Convenzioni servo
- La libreria usa `Servo::write(angle)` con angoli 0-180
- Le costanti in `robot.h` sono valori "grezzi" che potrebbero riferirsi a microsecondi o angoli diretti
- Verificare calibrazione fisica dei servo durante test

---

## Dipendenze
- `Arduino.h`
- `Servo.h` (libreria Arduino standard)
- `include/robot.h` (pin e costanti)

## Posizione
La libreria si trova in `lib/braccio/`.
