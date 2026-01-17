# Libreria `sensorBoard`

Gestisce la lettura dei sensori IR di linea e dei sensori di colore tramite la scheda sensori inferiore, comunicando via Serial1.

## Funzionalità principali

- **start()**  
  Inizializza la comunicazione e la lettura dei sensori.

- **line()**  
  Restituisce la posizione della linea rilevata sotto il robot (`int16_t` da -1750 a 1750, 0 = centro).

- **checkLinea()**  
  Ritorna `true` se la linea è rilevata davanti al robot.

- **checkColor()**  
  Ritorna `true` se viene rilevato il colore configurato.

- **checkGreenSx() / checkGreenDx()**  
  Ritorna `true` se viene rilevato il verde rispettivamente a sinistra o destra.

- **setCheckColor(mode)**  
  Imposta la modalità di ricerca colore (1=Silver, 2=Black, 3=Red).

- **getLineMode()**  
  Restituisce la modalità colore linea attuale.

- **utils.calibration()**  
  Avvia la calibrazione dei sensori, ritorna `true` se riuscita.

- **utils.calibration_val()**  
  Restituisce i valori di calibrazione dei sensori.

- **utils.val_sensor() / utils.val_sensor(n)**  
  Restituisce i valori grezzi dei sensori (array o singolo).

- **utils.val_sensorCal() / utils.val_sensorCal(n)**  
  Restituisce i valori calibrati dei sensori (array o singolo).

- **utils.greenSxCalibration() / utils.greenDxCalibration()**  
  Avvia la calibrazione del sensore di verde sinistro o destro.

## Note

- Comunicazione su Serial1 a 57600 baud.
- Tutte le funzioni sono non bloccanti.