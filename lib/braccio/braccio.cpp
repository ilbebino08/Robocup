#include <braccio.h>
#include <debug.h>

void Braccio::init() {
    pinMode(SENS_PINZA, INPUT);

    braccio.attach(BRACCIO_PIN);
    pinza.attach(PINZA_PIN);

    delay(50);

    braccio.write(BRACCIO_RIP);
    pinza.write(PINZA_RIP);

    delay(100);

    debug.println("Braccio inizializzato.");
}

void Braccio::riposo() {
    braccio.write(BRACCIO_RIP);
    pinza.write(PINZA_RIP);

    debug.println("Braccio in posizione di riposo.");
}

void Braccio::abbassato() {
    braccio.write(74);
    debug.println("Braccio abbassato.");
}

bool Braccio::prendiPallina() {
    /* Implementazione della funzione per prendere una pallina */
    debug.println("Prendo la pallina...");
    return (SENS_PINZA == HIGH);
}