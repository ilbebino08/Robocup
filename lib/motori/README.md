# Libreria `motori`

Questa libreria converte comandi “astratti” (`vel`, `ang`) in impulsi PWM per 3 servi:
- **MSX** (front-left)
- **MDX** (front-right)
- **MPO** (posteriore)

> Nota importante (coerenza progetto): in `include/robot.h` MPO è descritto come “sterzo posteriore”, ma **l’implementazione attuale** (`motori.cpp`) usa `ang` per differenziare **MSX/MDX** e fa seguire **MPO alla media** della trazione anteriore. Quindi MPO, di fatto, viene trattato come **terzo motore di trazione**, non come servo di sterzata.

---

## API

### `void init()`
- Attacca i 3 `Servo` ai pin (`MSX_PIN`, `MDX_PIN`, `MPO_PIN`)
- Imposta tutti a posizione di stop (`*_ZERO`)
- Attende piccoli delay per stabilizzare i servi

### `void muovi(short vel, short ang)`
- **Input**
  - `vel` viene saturata a `[-1023, 1023]`
  - `ang` viene saturata a `[-1750, 1750]`
- **Output**
  - scrive microsecondi su MSX/MDX/MPO tramite `writeMicroseconds(...)`

### `void stop()`
- Porta tutti i servi a `*_ZERO`.

---

## Convenzioni PWM e deadzone

I servi sono controllati in microsecondi:
- `*_MIN`  (es. 500)  = massimo “indietro”
- `*_ZERO` (es. 1500) = stop
- `*_MAX`  (es. 2000) = massimo “avanti”
- `*_ZEROMIN` / `*_ZEROMAX` = bordi deadzone attorno allo stop

Libreria usa `map()` per trasformare:
- `vel` (0..1023) in un intervallo PWM “avanti” o “indietro”
- `ang` (±1750) per introdurre differenziale tra MSX/MDX (vedi sotto)

---

## Logica completa di `muovi(vel, ang)` (come in `motori.cpp`)

### 1) Saturazioni
- `vel = constrain(vel, -1023, 1023)`
- `ang = constrain(ang, -1750, 1750)`

### 2) Calcolo MSX/MDX (con “mixing” di `ang`)

#### Caso A — `vel == 0` (rotazione sul posto tramite differenziale)
- Se `ang < 0`:
  - MSX viene spinto verso “avanti”
  - MDX viene spinto verso “indietro”
- Se `ang > 0`:
  - MSX verso “indietro”
  - MDX verso “avanti”

In pratica: con `vel=0`, `ang` diventa un comando di “spin”.

#### Caso B — `vel > 0` (avanti con correzione curva)
1) Base trazione:
- `vel_sx = map(vel, 0..1023, MSX_ZEROMIN..MSX_MAX)`
- `vel_dx = map(vel, 0..1023, MDX_ZEROMIN..MDX_MAX)`

2) Curva con `ang`:
- se `ang < 0` viene ridotto **solo MSX** (verso la deadzone)
- se `ang > 0` viene ridotto **solo MDX** (verso la deadzone)

#### Caso C — `vel < 0` (indietro con correzione curva)
1) Base trazione:
- `vel_sx = map(-vel, 0..1023, MSX_ZEROMAX..MSX_MIN)`
- `vel_dx = map(-vel, 0..1023, MDX_ZEROMAX..MDX_MIN)`

2) Curva con `ang`:
- se `ang < 0` viene modificato **solo MSX** verso `MSX_ZEROMAX`
- se `ang > 0` viene modificato **solo MDX** verso `MDX_ZEROMAX`

> Quindi: `ang` NON è “sterzo MPO” in questa versione; è un bias differenziale su uno dei due anteriori.

---

### 3) Inversione MSX/MDX
Se un motore è invertito, si riflette l’output attorno allo ZERO:

`out = ZERO - (out - ZERO)`

Applicato separatamente:
- `if (MSX_INV) vel_sx = MSX_ZERO - (vel_sx - MSX_ZERO);`
- `if (MDX_INV) vel_dx = MDX_ZERO - (vel_dx - MDX_ZERO);`

---

### 4) Calcolo MPO (segue la media della trazione anteriore)
1) Normalizza MSX/MDX in scala “comando” `[-1023, 1023]` (in base al verso):
- se `vel >= 0` normalizza da `(ZEROMIN..MAX) -> (0..1023)`
- se `vel < 0` normalizza da `(ZEROMAX..MIN) -> (0..1023)` e poi mette segno negativo

2) Media:
- `vel_media = (vel_sx_norm + vel_dx_norm) / 2`

3) Converte `vel_media` in PWM MPO:
- se `vel_media >= 0`: `map(0..1023, MPO_ZEROMIN..MPO_MAX)`
- se `vel_media < 0`:  `map(0..1023, MPO_ZEROMAX..MPO_MIN)`

4) Inversione MPO:
- `if (MPO_INV) mpo = MPO_ZERO - (mpo - MPO_ZERO);`

---

## Esempi

```cpp
// Avanti dritto (ang ~ 0)
motori.muovi(800, 0);

// Curva (ang positivo): riduce MDX, MSX resta più “spinto”
motori.muovi(800, 900);

// Spin sul posto (vel=0): motori anteriori opposti
motori.muovi(0, -1200);

// Stop
motori.stop();
```

---

## Note / TODO di architettura

- Se l’obiettivo è un sistema “2x trazione + 1x sterzo posteriore”, allora MPO dovrebbe mappare `ang` (non la media della trazione) e MSX/MDX dovrebbero ricevere la stessa `vel` (salvo eventuali compensazioni).  
- L’implementazione attuale è più vicina a un comportamento “differenziale + terzo motore che segue”.
