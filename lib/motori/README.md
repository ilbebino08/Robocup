# Libreria Motori

Gestisce il controllo dei 3 motori servo per la trazione del robot.

## Caratteristiche

- Controllo indipendente di velocità e direzione
- Movimento differenziale con 3 motori (2 anteriori + 1 posteriore)
- Velocità e angolo trattati come componenti indipendenti
- Inversione motori configurabile
- Limiti di sicurezza sui valori

## Architettura

### Logica di Movimento

Il sistema tratta **velocità** e **angolo** come componenti indipendenti:

- **Velocità (vel)**: Componente di movimento in avanti/indietro (-1023 a +1023)
- **Angolo (ang)**: Componente di sterzata laterale (-1750 a +1750)

Le due componenti vengono combinate:
```
vel_motore_sx = ZERO + vel_avanti + componente_laterale_sx
vel_motore_dx = ZERO + vel_avanti + componente_laterale_dx
```

Questo permette curve rapide anche a bassa velocità.

### Motore Posteriore

Il motore posteriore segue la media dei due motori anteriori, considerando le eventuali inversioni.

## API

### `void init()`
Inizializza i 3 motori e imposta le velocità iniziali a zero.

### `void muovi(short vel, short ang)`
Comanda i motori con velocità e angolo specificati.
- `vel`: -1023 (indietro massimo) a +1023 (avanti massimo)
- `ang`: -1750 (sinistra massimo) a +1750 (destra massimo)

### `void stop()`
Ferma tutti i motori impostando i valori ZERO.

## Configurazione

Definita in `include/robot.h`:
- Pin dei motori: `MSX_PIN`, `MDX_PIN`, `MPO_PIN`
- Range velocità: `MSX_MIN/MAX`, `MDX_MIN/MAX`, `MPO_MIN/MAX`
- Valori zero: `MSX_ZERO`, `MDX_ZERO`, `MPO_ZERO`
- Inversioni: `MSX_INV`, `MDX_INV`, `MPO_INV`

## Esempio d'uso

```cpp
Motori motori;

void setup() {
    motori.init();
}

void loop() {
    motori.muovi(500, 0);     // Avanti dritto
    delay(1000);
    motori.muovi(500, 1750);  // Avanti con sterzata destra massima
    delay(1000);
    motori.stop();
}
```
