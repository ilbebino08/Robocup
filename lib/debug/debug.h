#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>
#include <SD.h>

enum DebugOutput {
    DEBUG_USB = 1,
    DEBUG_BLUETOOTH = 2,
    DEBUG_SD = 4,
    DEBUG_ALL = 7
};

class Debug {
private:
    uint8_t outputMask;
    HardwareSerial* bluetoothSerial;
    File logFile;
    bool sdEnabled;
    String logFileName;
    
public:
    Debug();
    
    /**
     * @brief Inizializza il sistema di debug
     * @param outputs Maschera di bit per gli output desiderati (es: DEBUG_USB | DEBUG_BLUETOOTH)
     */
    void begin(uint8_t outputs = DEBUG_USB);
    
    /**
     * @brief Configura la seriale Bluetooth
     * @param serial Puntatore alla seriale hardware per il Bluetooth
     */
    void setBluetoothSerial(HardwareSerial* serial);
    
    /**
     * @brief Configura il logging su SD card
     * @param fileName Nome del file di log
     * @param csPin Pin CS della SD card
     * @return true se l'inizializzazione è riuscita
     */
    bool setSDCard(const char* fileName, uint8_t csPin);
    
    /**
     * @brief Abilita o disabilita un output specifico
     * @param output Output da modificare
     * @param enable true per abilitare, false per disabilitare
     */
    void setOutput(DebugOutput output, bool enable);
    
    /**
     * @brief Stampa un messaggio di debug
     * @param message Messaggio da stampare
     */
    void print(const String& message);
    void print(const char* message);
    void print(int value);
    void print(float value);
    void print(double value);
    
    /**
     * @brief Stampa un messaggio di debug con newline
     * @param message Messaggio da stampare
     */
    void println(const String& message);
    void println(const char* message);
    void println(int value);
    void println(float value);
    void println(double value);
    void println();
    
    /**
     * @brief Stampa un messaggio formattato (come printf)
     * @param format Stringa di formato
     * @param ... Argomenti variabili
     */
    void printf(const char* format, ...);
    
    /**
     * @brief Chiude il file di log su SD (se aperto)
     */
    void flush();
};

// Istanza globale
extern Debug debug;

#endif
