#include <braccio.h>

void Braccio::init() {
    pinMode(SENS_PINZA, INPUT);

    braccio.attach(BRACCIO_PIN);
    pinza.attach(PINZA_PIN);

    delay(50);

    braccio.write(BRACCIO_RIP);
    pinza.write(PINZA_RIP);

    delay(100);
}

void Braccio::riposo() {
    braccio.write(BRACCIO_RIP);
    pinza.write(PINZA_RIP);
}

void Braccio::abbassato() {
    braccio.write(74);
}

bool Braccio::prendiPallina() {
    /* Implementazione della funzione per prendere una pallina */
    return (SENS_PINZA == HIGH);
}