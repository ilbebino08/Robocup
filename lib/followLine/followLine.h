#ifndef followLine
#define followLine

#include <Arduino.h>

/**
 * @brief Funzione di controllo PID per il line following.
 * 
 * Legge la posizione della linea dal sensorBoard, calcola l'errore rispetto
 * al centro (0), applica un controllo PID e steera il robot di conseguenza
 * usando i motori per mantenere il robot sulla linea.
 * 
 * @param base_vel Velocità base di movimento (range: 0-1023)
 * @return true se la linea è stata rilevata e il controllo è stato applicato
 * @return false se la linea non è stata rilevata
 */
short pidLineFollowing(short base_vel);

/**
 * @brief Funzione di reset dei parametri PID.
 * 
 * Chiama questa funzione quando desideri azzerare lo stato del controllore PID
 * (utile quando il robot ricomincia a seguire la linea dopo una pausa).
 */
void resetPID();

#endif