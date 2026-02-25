#include "tofManager.h"

void tof::refresh() {
    VL53L0X_RangingMeasurementData_t measure;  // Variabile locale temporanea
    sensor.rangingTest(&measure);
    distance = measure.RangeMilliMeter;  // Salva solo la distanza
}

uint16_t tof::getDistance() {
    return distance;
}

void dWrite(bool val) {
    digitalWrite(XSHUT_SX_ANT, val);
    digitalWrite(XSHUT_SX_POS, val);
    digitalWrite(XSHUT_DX_ANT, val);
    digitalWrite(XSHUT_DX_POS, val);
    digitalWrite(XSHUT_PALLINA, val);
    digitalWrite(XSHUT_FRONT, val); 
}

bool tof::setID(uint8_t newAddr) {
    digitalWrite(xshutPin, HIGH);
    delay(10);
    if (!sensor.begin()) return false;
    sensor.setAddress(newAddr);
    delay(5);
    return true;
}

bool tofManager::setID() {
    // Metti tutti i sensori in reset (LOW) - spegni tutto
    digitalWrite(XSHUT_FRONT, LOW);
    digitalWrite(XSHUT_SX_ANT, LOW);
    digitalWrite(XSHUT_SX_POS, LOW);
    digitalWrite(XSHUT_DX_ANT, LOW);
    digitalWrite(XSHUT_DX_POS, LOW);
    digitalWrite(XSHUT_PALLINA, LOW);
    delay(50);
    
    debug.println("Assegnazione indirizzi I2C...");
    delay(10);
    
    // Accendi SOLO front, assegna indirizzo, poi spegni
    digitalWrite(XSHUT_FRONT, HIGH);
    delay(50);
    if(!front.setID(ADDR_FRONT)) {
        debug.println("  ✗ FRONT fallito");
        return false;
    }
    debug.println("  ✓ FRONT assegnato");
    digitalWrite(XSHUT_FRONT, LOW);
    delay(20);

    // Accendi SOLO antSX
    digitalWrite(XSHUT_SX_ANT, HIGH);
    delay(50);
    if(!antSX.setID(ADDR_ANTSX)) {
        debug.println("  ✗ ANTSX fallito");
        return false;
    }
    debug.println("  ✓ ANTSX assegnato");
    digitalWrite(XSHUT_SX_ANT, LOW);
    delay(20);

    // Accendi SOLO antDX
    digitalWrite(XSHUT_DX_ANT, HIGH);
    delay(50);
    if(!antDX.setID(ADDR_ANTDX)) {
        debug.println("  ✗ ANTDX fallito");
        return false;
    }
    debug.println("  ✓ ANTDX assegnato");
    digitalWrite(XSHUT_DX_ANT, LOW);
    delay(20);

    // Accendi SOLO posSX
    digitalWrite(XSHUT_SX_POS, HIGH);
    delay(50);
    if(!posSX.setID(ADDR_POSSX)) {
        debug.println("  ✗ POSSX fallito");
        return false;
    }
    debug.println("  ✓ POSSX assegnato");
    digitalWrite(XSHUT_SX_POS, LOW);
    delay(20);

    // Accendi SOLO posDX
    digitalWrite(XSHUT_DX_POS, HIGH);
    delay(100); // Delay più lungo per POSDX
    debug.println("  Inizializzazione POSDX...");
    delay(5);
    if(!posDX.setID(ADDR_POSDX)) {
        debug.println("  ✗ POSDX fallito");
        digitalWrite(XSHUT_DX_POS, LOW);
        delay(20);
        return false;
    }
    debug.println("  ✓ POSDX assegnato");
    delay(50); // Delay ancora più lungo
    digitalWrite(XSHUT_DX_POS, LOW);
    delay(100); // Delay lungo dopo spegnimento per stabilizzare

    // Accendi SOLO ball - TEMPORANEAMENTE DISABILITATO PER DEBUG
    debug.println("  Inizializzazione PALLINA...");
    delay(20);
    digitalWrite(XSHUT_PALLINA, HIGH);
    delay(150);
    debug.println("  Chiamata setID PALLINA...");
    delay(10);
    
    // Commentiamo il setID di ball per escludere dal debug
    // if(!ball.setID(ADDR_BALL)) {
    //     debug.println("  ✗ PALLINA fallita");
    //     digitalWrite(XSHUT_PALLINA, LOW);
    //     delay(50);
    //     return false;
    // }
    debug.println("  ⊘ PALLINA saltata (debug)");
    delay(50);
    digitalWrite(XSHUT_PALLINA, LOW);
    delay(100);

    // Accendi TUTTI i sensori per il funzionamento normale
    digitalWrite(XSHUT_FRONT, HIGH);
    digitalWrite(XSHUT_SX_ANT, HIGH);
    digitalWrite(XSHUT_SX_POS, HIGH);
    digitalWrite(XSHUT_DX_ANT, HIGH);
    digitalWrite(XSHUT_DX_POS, HIGH);
    digitalWrite(XSHUT_PALLINA, HIGH);
    delay(50);
    
    return true;
}

bool tofManager::init() {

    front.xshutPin = XSHUT_FRONT;
    antSX.xshutPin = XSHUT_SX_ANT;
    antDX.xshutPin = XSHUT_DX_ANT;
    posSX.xshutPin = XSHUT_SX_POS;
    posDX.xshutPin = XSHUT_DX_POS;
    ball.xshutPin = XSHUT_PALLINA;

    debug.println("Configurazione pin XSHUT sensori ToF...");

    pinMode(XSHUT_SX_ANT, OUTPUT);
    pinMode(XSHUT_SX_POS, OUTPUT);
    pinMode(XSHUT_DX_ANT, OUTPUT);
    pinMode(XSHUT_DX_POS, OUTPUT);
    pinMode(XSHUT_PALLINA, OUTPUT);
    pinMode(XSHUT_FRONT, OUTPUT);

    debug.println("Inizializzazione sensori ToF...");

    digitalWrite(XSHUT_SX_ANT, LOW);
    digitalWrite(XSHUT_SX_POS, LOW);
    digitalWrite(XSHUT_DX_ANT, LOW);
    digitalWrite(XSHUT_DX_POS, LOW);
    digitalWrite(XSHUT_PALLINA, LOW);
    digitalWrite(XSHUT_FRONT, LOW);

    debug.println("Assegnazione indirizzi I2C univoci...");
    if (!setID()) return false;

    debug.println("Sensori ToF inizializzati correttamente.");

    return true;
}

void tofManager::refreshAll(){
    front.refresh();
    antSX.refresh();
    antDX.refresh();
    posSX.refresh();
    posDX.refresh();
    // ball.refresh(); // DISABILITATO - Sensore guasto
}