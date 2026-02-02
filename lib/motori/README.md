# Libreria Motori

Gestisce il controllo dei 3 motori servo per la trazione del robot.

## Architettura

Sistema di locomozione omnidirezionale a 3 ruote:
- 2 motori anteriori (sinistro e destro): propulsione principale
- 1 motore posteriore: sterzo omnidirezionale

## Caratteristiche

- Controllo differenziale con sterzo posteriore indipendente
- Velocità e angolo come parametri separati
- Inversione motori configurabile per correggere cablaggi
- Deadzone configurabile per motori a riposo
- Limiti di sicurezza su velocità e sterzo

## API

### `void init()`
Inizializza i 3 motori servo e imposta i valori a zero (fermo).

**Esempio:**
```cpp
Motori motori;
motori.init();
```

### `void Muovi(short vel, short ang)`
Comanda i motori con velocità e angolo di sterzo specificati.

**Parametri:**
- `vel`: velocità [-1023, +1023]
  - Positivo: avanti
  - Negativo: indietro
  - Zero: fermo
  - Controlla entrambi i motori anteriori in modo uguale

- `ang`: angolo sterzo [-1750, +1750]
  - Negativo: ruota posteriore a sinistra
  - Zero: ruota posteriore centrata
  - Positivo: ruota posteriore a destra
  - Controlla solo il motore posteriore

**Mapping velocità:**
- Avanti (vel ≥ 0): `map(vel, 0, 1023, ZEROMIN, MAX)`
- Indietro (vel < 0): `map(abs(vel), 0, 1023, ZEROMAX, MIN)`

**Mapping sterzo:**
- `map(ang, -1750, 1750, MIN, MAX)` applicato al motore posteriore

**Esempio:**
```cpp
motori.Muovi(500, 0);      // Avanti dritto
motori.Muovi(500, 1750);   // Avanti con sterzo destro massimo
motori.Muovi(-300, -500);  // Indietro con sterzo sinistro
```

### `void stop()`
Ferma tutti i motori impostando i valori ZERO.

**Esempio:**
```cpp
motori.stop();
```

## Configurazione

Definita in `include/robot.h`:

**Pin dei motori:**
- `MSX_PIN` (30): motore sinistro anteriore
- `MDX_PIN` (29): motore destro anteriore  
- `MPO_PIN` (27): motore posteriore omnidirezionale

**Range PWM (µs):**
- MIN (500): massima velocità inversa
- MAX (2000): massima velocità avanti
- ZERO (1500): fermo
- ZEROMIN (1450): limite inferiore deadzone
- ZEROMAX (1550): limite superiore deadzone

**Inversioni:**
- `MSX_INV` (false): inversione motore sinistro
- `MDX_INV` (true): inversione motore destro
- `MPO_INV` (true): inversione motore posteriore

La formula di inversione: `output = ZERO - (value - ZERO)` riflette il valore attorno al centro.

## Esempio Completo

```cpp
#include <motori.h>

Motori motori;

void setup() {
    motori.init();
}

void loop() {
    // Avanti dritto per 2 secondi
    motori.Muovi(500, 0);
    delay(2000);
    
    // Curva destra per 1 secondo
    motori.Muovi(500, 1000);
    delay(1000);
    
    // Ferma
    motori.stop();
    delay(1000);
    
    // Indietro con sterzo sinistro
    motori.Muovi(-400, -800);
    delay(1500);
    
    motori.stop();
    delay(2000);
}
```
