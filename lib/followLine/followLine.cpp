#include <Arduino.h>
#include "followLine.h"
#include <sensorBoard.h>
#include <motori.h>
#include "../../include/robot.h"

// ===== COSTANTI PER IL PID LINE FOLLOWING =====
// TODO: aumentare KP e KD dopo test
#define KP           1.0    // Coefficiente proporzionale. Aumenta se il robot risponde lentamente alle curve, Diminuisci se oscilla (range: 0.5-2.0)
#define KI           0.1    // Coefficiente integrale. Aumenta per eliminare errori persistenti, Diminuisci se causa oscillazioni (range: 0.0-0.5)
#define KD           0.2    // Coefficiente derivativo. Aumenta per stabilizzare, Diminuisci se il robot diventa rigido (range: 0.2-1.5)
#define MAX_STEERING 1750   // Massimo angolo di sterzata. Aumenta per curve più marcate, Diminuisci per movimenti più dolci (min: 500, max: 1750)

// ===== VARIABILI GLOBALI PER IL PID (dichiarazioni esterne) =====
extern float integral_error;     // Errore integrale accumulato
extern float last_line_position; // Ultima posizione della linea
extern unsigned long last_time;  // Ultimo tempo di aggiornamento

// Dichiarazioni esterne
extern BottomSensor IR_board;
extern Motori motori;

// Definizione variabili globali PID
float integral_error = 0.0;
float last_line_position = 0.0;
unsigned long last_time = 0;

short pidLineFollowing(short base_vel) {
    // Leggi la posizione della linea dal sensorBoard
    int16_t line_position = IR_board.line();
    
    // Calcola il tempo trascorso da ultimo calcolo (in millisecondi)
    unsigned long current_time = millis();
    float delta_time = (current_time - last_time) / 1000.0;  // Converti in secondi
    
    if (delta_time <= 0) delta_time = 0.001;  // Evita divisioni per 0
    
    // ===== CALCOLO ERRORE =====
    // L'errore è la distanza dalla linea rispetto al centro (0)
    float error = line_position;
    
    // ===== TERMINE PROPORZIONALE =====
    float p_term = KP * error;
    
    // ===== TERMINE INTEGRALE =====
    // Accumula l'errore nel tempo con anti-windup
    integral_error += error * delta_time;
    // Limita l'errore integrale per evitare wind-up
    if (integral_error > 100) integral_error = 100;
    if (integral_error < -100) integral_error = -100;
    float i_term = KI * integral_error;
    
    // ===== TERMINE DERIVATIVO =====
    float derivative = (error - last_line_position) / delta_time;
    float d_term = KD * derivative;
    
    // ===== CALCOLO OUTPUT PID =====
    float pid_output = p_term + i_term + d_term;
    
    // Limita l'output PID al massimo angolo di sterzata
    if (pid_output > MAX_STEERING) pid_output = MAX_STEERING;
    if (pid_output < -MAX_STEERING) pid_output = -MAX_STEERING;
    
    // Converti l'output PID a angolo di sterzata (range: -1750 a 1750)
    short steering_angle = (short)pid_output;
    
    // ===== APPLICA MOVIMENTO =====
    // Muovi il robot con velocità base e angolo calcolato dal PID
    motori.muovi(base_vel, steering_angle);
    
    // ===== AGGIORNA VARIABILI PER PROSSIMA ITERAZIONE =====
    last_line_position = error;
    last_time = current_time;
    
    return steering_angle;
}

void resetPID() {
    integral_error = 0.0;
    last_line_position = 0.0;
    last_time = millis();
}