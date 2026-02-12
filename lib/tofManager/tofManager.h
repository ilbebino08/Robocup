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
        void read();

};

class tofManager {
    private:
        bool setID();
    public:
        tof tof_Front;
        tof tof_AntSX;
        tof tof_AntDX;
        tof tof_PosSX;
        tof tof_PosDX;
        tof tof_Ball;

        bool init();

};

#endif