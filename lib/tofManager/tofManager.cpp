#include "tofManager.h"

void tof::read() {
    sensor.rangingTest(&data);
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

    tof_Front.xshutPin = XSHUT_FRONT;
    tof_AntSX.xshutPin = XSHUT_SX_ANT;
    tof_AntDX.xshutPin = XSHUT_DX_ANT;
    tof_PosSX.xshutPin = XSHUT_SX_POS;
    tof_PosDX.xshutPin = XSHUT_DX_POS;
    tof_Ball.xshutPin = XSHUT_PALLINA;

    if(!tof_Front.setID(ADDR_FRONT)) return false;
    if(!tof_AntSX.setID(ADDR_ANTSX)) return false;
    if(!tof_AntDX.setID(ADDR_ANTDX)) return false;
    if(!tof_PosSX.setID(ADDR_POSSX)) return false;
    if(!tof_PosDX.setID(ADDR_POSDX)) return false;
    if(!tof_Ball.setID(ADDR_BALL)) return false;
    // Tutti i sensori dovrebbero ora rispondere a indirizzi diversi
    return true;
}

bool tofManager::init() {

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

