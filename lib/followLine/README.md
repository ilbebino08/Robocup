# Libreria `followLine`

Questa libreria implementa il **line following** tramite un controllore **PID** usando la posizione linea fornita da `sensorBoard` e comandando i motori tramite `motori`.

> Punto chiave: `pidLineFollowing()` **legge i sensori, calcola la correzione e chiama direttamente** `motori.muovi(...)`.  
> Non è solo "calcolo PID": è una funzione "alto livello" che esegue anche l'attuazione.

---

## Dipendenze e flusso dati

- **Input sensore**: `IR_board.line()` (da `lib/sensorBoard`)  
  - ritorna `int16_t` in `[-1750, 1750]`  
  - `0` = linea centrata sotto al robot
- **Output attuazione**: `motori.muovi(vel, ang)` (da `lib/motori`)  
  - `vel` in `[-1023, 1023]`
  - `ang` in `[-1750, 1750]` (sterzata / comando direzionale)
- **Ridimensionamento velocità in curva**: Scaling automatico per mantenere controllo in manovre strette (commit bb8cdab)
- **Retromarcia intelligente**: Inversione automatica della sterzata in retromarcia per evitare inversione di controllo (commit bb8cdab)
- **Ottimizzazione RAM** (commit efab69f): Algoritmo PID ottimizzato con integrale clampato per anti wind-up, scaling velocità ridotto a operazioni intere

---

## API

### `short pidLineFollowing(short base_vel);`

- **Cosa fa**
  1. Legge la posizione linea (`line_position = IR_board.line()`).
  2. Calcola `delta_time` con `millis()` per rendere il PID indipendente dal loop.
  3. Calcola PID su errore `error = line_position`.
  4. Satura l'uscita PID a `±MAX_STEERING`.
  5. Riduce la velocità in curva (scaling in base a quanto sei lontano dal centro).
  6. Se sei in retromarcia, inverte la sterzata.
  7. Comanda i motori: `motori.muovi(scaled_vel, steering_angle)`.
  8. Aggiorna lo stato interno (integrale, last error, last time).

- **Parametri**
  - `base_vel`: velocità desiderata (tipicamente `DEFAULT_VELOCITY`), con segno:
    - `> 0` avanti
    - `< 0` indietro

- **Ritorno**
  - `short steering_angle`: l'angolo calcolato dal PID (già saturato), in `[-1750, 1750]`.

> Nota importante: la funzione **non** verifica `IR_board.checkLinea()`.
> Se vuoi gestire "linea persa", fallo nel chiamante (state machine / fallback).

---

### `void resetPID();`

- **Cosa fa**
  - azzera:
    - `integral_error`
    - `last_line_position`
  - reimposta `last_time = millis()`

Usala quando:
- riparti dopo uno stop,
- cambi modalità/strategia,
- hai oscillazioni dovute a integrale "carico" (wind-up).

---

## Logica interna (come funziona davvero)

### 1) Errore
- `error = line_position`
  - se la linea è a destra/sinistra, il segno dell'errore segue quello della `sensorBoard`.

### 2) PID con tempo reale
- `delta_time = (current_time - last_time) / 1000.0`
- Termini:
  - **P**: `p_term = KP * error`
  - **I**: `integral_error += error * delta_time` con clamp `[-100, 100]` (anti wind-up semplice)
  - **D**: `derivative = (error - last_line_position) / delta_time`

Output:
- `pid_output = p_term + i_term + d_term`
- Saturazione:
  - `pid_output ∈ [-MAX_STEERING, MAX_STEERING]`

### 3) Conversione in sterzata
- `steering_angle = (short)pid_output`

### 4) Scaling automatico della velocità in curva
- Riduce la velocità quando `|error|` aumenta:
  - `scale = 1.0 - (|error|/MAX_STEERING) * 0.7`
  - clamp: minimo `0.3` (=> almeno 30% di `base_vel`)
- `scaled_vel = abs(base_vel) * scale` mantenendo il segno di `base_vel`.

### 5) Retromarcia
Se `base_vel < 0`:
- `steering_angle = -steering_angle`

Questo evita che il controllo "giri al contrario" quando si va indietro.

---

## Parametri di tuning (nel `.cpp`)

Nel file `followLine.cpp` sono definiti:
