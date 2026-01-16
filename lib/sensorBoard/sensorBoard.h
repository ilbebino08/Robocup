#include <Arduino.h>
#include <SPI.h>

/* Version 2.0.0 - 15/01/2025 */

#ifndef sensorBoard
#define sensorBoard

/*Configurazione*/

/**@brief Numero sensori scheda sotto robot*/
#define N_SENSOR 9

/**@brief Velocità seriale scheda sotto robot*/
#define SERIAL_SPEED_BS 57600


/** Classe Rappresentante I Sensori Posti Sotto al Robot.
 *
 */
class BottomSensor{

private:

    /**
     * @brief Sottoclasse contenente le funzioni di controllo / configurazione dei sensori.
     */
    class Utils{
    private:

        //Attesa seriale
        static void wait_serial() { while (Serial1.available() == 0) { } }

    public:
        /**
         * @brief Calibra sensori, ritorna un booleano contenente il risultato della calibrazione.
         *
         * @return True -> Calibrazione effettuata con successo
         * @return False -> Calibrazione fallita
         */
        bool calibration();


        /**
         * @brief Restituisce i valori della calibrazione dei sensori.
         *
         * @return uint16_t* -> Array grande N_SENSOR*2
         */
        uint16_t* calibration_val();


        /**
         * @brief Restituisce i valori dei sensori infrarossi e li restituisce in un array.
         *
         * @return uint16_t* -> Array valori sensori .
         */
        uint16_t* val_sensor();


        /**
         * @brief Restituisce il valore di un sensore il cui numero dato in input.
         *
         * @param n_sensore -> Numero sensore.
         * @return uint16_t -> Valore sensore.
         */
        uint16_t val_sensor(uint8_t n_sensore);


        /**
         * @brief Restituisce i valori calibrati dei sensori e li restituisce in un array.
         *
         * @return uint16_t* -> Array valori sensori calibrati.
         */
        uint16_t* val_sensorCal();


        /**
         * @brief Restituisce il valore calibrato di un sensore il cui numero dato in input.
         *
         * @param n_sensore -> Numero sensore.
         * @return uint16_t -> Valore sensore.
         */
        uint16_t val_sensorCal(uint8_t n_sensore);

        /**
         *  @brief Avvia la calibrazione del sensore di colore sinistro.
         *
         */
        void greenSxCalibration();


        /**
         *  @brief Avvia la calibrazione del sensore di colore destro.
         *
         */
        void greenDxCalibration();
    };

    /**
     *    +--------+-------+-------+------+------+-------+-----+-----+
     *    |  Bit   |   7   |   6   |  5   |  4   | 3 & 2 |  1  |  0  |
     *    +--------+-------+-------+------+------+-------+-----+-----+
     *    | Descr. | V. DX | V. SX | L.F. | L.T. | Mode  | N/D | N/D |
     *    +--------+-------+-------+------+------+-------+-----+-----+
     */
    uint8_t status;

    static void wait_serial() { while (Serial1.available() == 0) { } }

public:

    /**@brief Sottoclasse contenente le funzioni di controllo / configurazione dei sensori.*/
    Utils utils;

    /**
     * @brief Start Sensori
     *
     */
    bool start();

    /**
      * @brief Restituisce la posizione della linea rispetto al centro.
      *
      * @return int16_t -> Valore posizione della linea [-1750 <= x <= 1750].
      */
    int16_t line();


    /**
     * @brief Ritorna true se la linea è stata rilevata del colore impostato nel "setCheckColor".
     */
    bool checkColor(){ return bitRead(status, 4); }

    /**
     * @brief Ritorna true se è presente una linea difronte al robot.
     */
    bool checkLinea() { return bitRead(status, 5); }


    /** @brief Ritorna true se a sinistra del robot è presente il verde.
     */
    bool checkGreenSx(){ return bitRead(status, 6); }


    /** @brief Ritorna true se a destra del robot è presente il verde.
     *
     */
    bool checkGreenDx(){ return bitRead(status, 7); }


    /** @brief Imposta la modalità di ricerca colore della linea.
     *
     * @param mode Numero modalità.\n
     *          1) Silver
     *          2) Black
     *          3) Red
     */
    void setCheckColor(uint8_t mode);


    /** @brief Restituisce la modalità ricerca colore della linea attuale.
     *
     * @return int -> Modalità colore linea.
     */
    uint8_t getLineMode(){ return ((status & 0x0C) >> 2); }
};

#endif