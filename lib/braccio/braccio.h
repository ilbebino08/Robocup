#include <Arduino.h>
#include <Servo.h>
#include "../../include/robot.h"

class Braccio {
    private:
        Servo braccio;
        Servo pinza;
    public:
        void init();

        /**
         * @brief Imposta il braccio in posizione di riposo.
         */
        void riposo();

        /**
         * @brief Abbassa il braccio per abbassare il baricentro.
         */
        void abbassato();

        /**
         * @brief Prende la pallina.
         * 
         * @todo Implementare la logica per prendere la pallina.
         * @return true se la pallina è stata presa con successo, false altrimenti.
         */
        bool prendiPallina();
};