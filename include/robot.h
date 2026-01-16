#ifndef ROBOT_H
#define ROBOT_H

//================== COLLEGAMENTI DI I/O ===============================

// MOTORI - SERVO TRAZIONE
#define MSX_PIN         30          // Pin del motore sinistro
#define MDX_PIN         29          // Pin del motore destro
#define MPO_PIN         27          // Pin del motore posteriore

#define MSX_MIN         500         // Massima velocità indietro sinistro
#define MDX_MIN         500         // Massima velocità indietro destro
#define MPO_MIN         500         // Massima velocità indietro posteriore

#define MSX_MAX         2000        // Massima velocità avanti sinistro
#define MDX_MAX         2000        // Massima velocità avanti destro
#define MPO_MAX         2000        // Massima velocità avanti posteriore

#define MSX_ZEROMAX     1550        // Posizione stop motore sinistro
#define MDX_ZEROMAX     1550        // Posizione stop motore destro
#define MPO_ZEROMAX     1550        // Posizione stop motore posteriore

#define MSX_ZERO        1500        // Posizione stop motore sinistro
#define MDX_ZERO        1500        // Posizione stop motore destro
#define MPO_ZERO        1500        // Posizione stop motore posteriore

#define MSX_ZEROMIN     1450        // Posizione stop motore sinistro
#define MDX_ZEROMIN     1450        // Posizione stop motore destro
#define MPO_ZEROMIN     1450        // Posizione stop motore posteriore

#define MSX_INV         false       // Inversione direzione motore sinistro
#define MDX_INV         true        // Inversione direzione motore destro
#define MPO_INV         true       // Inversione direzione motore posteriore

#define BUTTON          2           // Pin del pulsante di interrupt

// LED
#define LED_R           23          // Pin LED rosso
#define LED_V           22          // Pin LED verde
#define LED_G           24          // Pin LED giallo

// SERVO BRACCIO E PINZA
#define PINZA_PIN       28          // Pin del servo pinza
#define BRACCIO_PIN     39          // Pin del servo braccio

#define PINZA_RIP       600         // Valore pinza in posizione di riposo
#define BRACCIO_RIP     570         // Valore braccio in posizione di riposo

#define BRACCIO_AVANTI  1500        // Valore braccio per affrontare le salite

// SENSORI
#define SW_ANT_DX       17          // Finecorsa anteriore destro
#define SW_ANT_SX       51          // Finecorsa anteriore sinistro
#define SENS_INCL       39          // Sensore di inclinazione
#define SENS_PINZA      31

// XSHUT Sensori VL53L0X
#define XSHUT_SX_ANT    52          // Pin XSHUT sensore anteriore sinistro
#define XSHUT_SX_POS    50          // Pin XSHUT sensore posteriore sinistro
#define XSHUT_DX_ANT    3           // Pin XSHUT sensore anteriore destro
#define XSHUT_DX_POS    5           // Pin XSHUT sensore posteriore destro
#define XSHUT_PALLINA   48          // Pin XSHUT sensore pallina
#define XSHUT_FRONT     49          // Pin XSHUT sensore frontale

//==========================================================================

// Costanti velocità motori
#define MAX_VELOCITY        1023    // Velocità massima motori
#define MIN_VELOCITY        0       // Velocità minima motori
#define DEFAULT_VELOCITY    800     // Velocità di default motori

// Costanti per stato della linea
#define LINEA           0   // Linea rilevata davanti al robot
#define COL_RILEVATO    1   // Colore della linea impostato rilevato
#define DOPPIO_VERDE    2   // Rilevato verde su entrambi i lati
#define VERDE_SX        3   // Rilevato verde a sinistra
#define VERDE_DX        4   // Rilevato verde a destra
#define NO_LINEA        5   // Nessuna linea rilevata davanti al robot

#endif