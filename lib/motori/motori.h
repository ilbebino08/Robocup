#ifndef MOTORI_H
#define MOTORI_H

#include <Arduino.h>
#include <Servo.h>
#include "../../include/robot.h"


class Motori{
    private:
        short msx_vel;
        short mdx_vel;
        short mpo_vel;

        Servo mot_sx;
        Servo mot_dx;
        Servo mot_po;

    public:

    void init();

    /**
     * @brief Muove i motori con una velocità e un angolo di sterzata specificati.
     * 
     * @param vel Velocità di movimento (-1023 a 1023).
     * @param ang Angolo di sterzata (-1750 a 1750).
     */
    void muovi(short vel, short ang);

    /**
     * @brief Ferma tutti i motori riportandoli alla posizione di stop.
     */
    void stop();
};

#endif