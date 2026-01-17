#ifndef MOTORI_H
#define MOTORI_H

#include <Arduino.h>
#include <Servo.h>
#include "../../include/robot.h"

/**
 * @brief Classe per il controllo dei motori del robot.
 * 
 * Questa classe gestisce le operazioni di base
 * dei motori del robot, inclusi il movimento e l'arresto.
 */
class Motori{
    private:
        short msx_vel;  // Velocità motore sinistro
        short mdx_vel;  // Velocità motore destro
        short mpo_vel;  // Velocità motore posteriore

        Servo mot_sx;   // Oggetto Servo per il controllo del motore sinistro
        Servo mot_dx;   // Oggetto Servo per il controllo del motore destro
        Servo mot_po;   // Oggetto Servo per il controllo del motore posteriore

    public:

    /**
     * @brief Inizializza i motori.
     * 
     * Configura i pin e imposta le velocità iniziali dei motori.
     */
    void init();

    /**
     * @brief Muove i motori con una velocità e un angolo di sterzata specificati.
     * 
     * @param vel Velocità di movimento (-1023 a 1023).
     * @param ang Angolo di sterzata (-1750 a 1750).
     */
    void muovi(short vel, short ang);

    /**
     * @brief Ferma tutti i motori.
     */
    void stop();
};

#endif