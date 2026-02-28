#ifndef TOFMANAGER_H
#define TOFMANAGER_H

#include <debug.h>
#include "../../include/robot.h"
#include <Adafruit_VL53L0X.h>

class tof{
    private:
        Adafruit_VL53L0X sensor;  // Oggetto per il sensore ToF
        
    public:
        int xshutPin; // Pin XSHUT per il controllo del sensore
        uint16_t distance; // Distanza in millimetri (invece della struttura pesante)

        bool setID(uint8_t newAddr);
        void refresh();
        uint16_t getDistance(); // Ottieni la distanza
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
        uint16_t* getAllDistances();

};

#endif