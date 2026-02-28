#ifndef followLine
#define followLine

#include <Arduino.h>

/**
 * @brief Funzione di controllo PID per il line following.
 * 
 * Legge la posizione della linea dal sensorBoard, calcola l'errore rispetto
 * al centro (0), applica un controllo PID e sterza il robot di conseguenza
 * usando i motori per mantenere il robot sulla linea.
 * 
 * @param base_vel Velocità base di movimento (range: -1023..1023). Negativa = retromarcia.
 * @return short Valore di correzione (angolo) calcolato dal PID.
 */
short pidLineFollowing(short base_vel);

/**
 * @brief Funzione di controllo PID per il line following con input della posizione linea.
 * 
 * Simile a pidLineFollowing(base_vel) ma accetta direttamente la posizione della linea
 * come parametro, evitando la lettura interna. Utile se hai già letto la posizione
 * della linea e vuoi applicare il PID su quella lettura.
 * 
 * @param base_vel Velocità base di movimento (range: -1023..1023). Negativa = retromarcia.
 * @param line_position Posizione della linea rispetto al centro (range: -1750..1750).
 * @return short Valore di correzione (angolo) calcolato dal PID.
 */

short pidLineFollowing(short base_vel, int16_t line_position);

/**
 * @brief Funzione di reset dei parametri PID.
 * 
 * Chiama questa funzione quando desideri azzerare lo stato del controllore PID
 * (utile quando il robot ricomincia a seguire la linea).
 */
void resetPID();

#endif