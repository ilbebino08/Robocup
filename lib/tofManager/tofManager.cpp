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
    // Metti tutti i sensori in reset (LOW)
    dWrite(LOW);
    delay(10);
    dWrite(HIGH);
    delay(10);
    dWrite(LOW);

    if(!front.setID(ADDR_FRONT)) return false;
    if(!antSX.setID(ADDR_ANTSX)) return false;
    if(!antDX.setID(ADDR_ANTDX)) return false;
    if(!posSX.setID(ADDR_POSSX)) return false;
    if(!posDX.setID(ADDR_POSDX)) return false;
    if(!ball.setID(ADDR_BALL))   return false;
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
    ball.refresh();
}