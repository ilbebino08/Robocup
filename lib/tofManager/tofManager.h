#ifndef TOFMANAGER_H
#define TOFMANAGER_H

#include <debug.h>
#include <robot.h>
#include <Adafruit_VL53L0X.h>

class tof{
    private:
        Adafruit_VL53L0X sensor;  // Oggetto per il sensore ToF
        
        
    public:
        int xshutPin; // Pin XSHUT per il controllo del sensore
        VL53L0X_RangingMeasurementData_t data; // Struttura per i dati del sensore

        bool setID(uint8_t newAddr);
        void refresh();

};

class tofManager {
    private:
        bool setID();
    public:
        tof front;
        tof antSX;
        tof antDX;
        tof posSX;
        tof posDX;
        tof ball;

        bool init();
        void refreshAll();

};

#endif