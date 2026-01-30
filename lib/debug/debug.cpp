#include "debug.h"
#include <stdarg.h>

// Istanza globale
Debug debug;

Debug::Debug() {
    outputMask = 0;
    bluetoothSerial = nullptr;
    sdEnabled = false;
    logFileName = "";
    lastMessage = "";
}

void Debug::begin(uint8_t outputs) {
    outputMask = outputs;
    
    if (outputMask & DEBUG_USB) {
        // Serial è già inizializzato nel main, ma verifichiamo
        if (!Serial) {
            Serial.begin(115200);
        }
    }
}

void Debug::setBluetoothSerial(HardwareSerial* serial) {
    bluetoothSerial = serial;
    if (bluetoothSerial && !(*bluetoothSerial)) {
        bluetoothSerial->begin(9600); // Velocità standard Bluetooth
    }
}

bool Debug::setSDCard(const char* fileName, uint8_t csPin) {
    logFileName = String(fileName);
    
    if (!SD.begin(csPin)) {
        if (outputMask & DEBUG_USB) {
            Serial.println("Errore: SD card non trovata!");
        }
        sdEnabled = false;
        return false;
    }
    
    // Apri il file in modalità append
    logFile = SD.open(logFileName.c_str(), FILE_WRITE);
    if (!logFile) {
        if (outputMask & DEBUG_USB) {
            Serial.println("Errore: impossibile aprire il file di log!");
        }
        sdEnabled = false;
        return false;
    }
    
    sdEnabled = true;
    logFile.println("=== Log avviato ===");
    logFile.flush();
    return true;
}

void Debug::setOutput(DebugOutput output, bool enable) {
    if (enable) {
        outputMask |= output;
    } else {
        outputMask &= ~output;
    }
}

void Debug::print(const String& message) {
    print(message.c_str());
}

void Debug::print(const char* message) {
    if (outputMask & DEBUG_USB) {
        Serial.print(message);
    }
    
    if ((outputMask & DEBUG_BLUETOOTH) && bluetoothSerial) {
        bluetoothSerial->print(message);
    }
    
    if ((outputMask & DEBUG_SD) && sdEnabled) {
        logFile.print(message);
    }
}

void Debug::print(int value) {
    if (outputMask & DEBUG_USB) {
        Serial.print(value);
    }
    
    if ((outputMask & DEBUG_BLUETOOTH) && bluetoothSerial) {
        bluetoothSerial->print(value);
    }
    
    if ((outputMask & DEBUG_SD) && sdEnabled) {
        logFile.print(value);
    }
}

void Debug::print(float value) {
    if (outputMask & DEBUG_USB) {
        Serial.print(value);
    }
    
    if ((outputMask & DEBUG_BLUETOOTH) && bluetoothSerial) {
        bluetoothSerial->print(value);
    }
    
    if ((outputMask & DEBUG_SD) && sdEnabled) {
        logFile.print(value);
    }
}

void Debug::print(double value) {
    if (outputMask & DEBUG_USB) {
        Serial.print(value);
    }
    
    if ((outputMask & DEBUG_BLUETOOTH) && bluetoothSerial) {
        bluetoothSerial->print(value);
    }
    
    if ((outputMask & DEBUG_SD) && sdEnabled) {
        logFile.print(value);
    }
}

void Debug::println(const String& message) {
    println(message.c_str());
}

void Debug::println(const char* message) {
    // Controlla se il messaggio è uguale all'ultimo inviato
    if (String(message) == lastMessage) {
        return; // Non inviare messaggi duplicati
    }
    
    lastMessage = String(message);
    
    if (outputMask & DEBUG_USB) {
        Serial.println(message);
    }
    
    if ((outputMask & DEBUG_BLUETOOTH) && bluetoothSerial) {
        bluetoothSerial->println(message);
    }
    
    if ((outputMask & DEBUG_SD) && sdEnabled) {
        logFile.println(message);
    }
}

void Debug::println(int value) {
    String valueStr = String(value);
    
    // Controlla se il messaggio è uguale all'ultimo inviato
    if (valueStr == lastMessage) {
        return; // Non inviare messaggi duplicati
    }
    
    lastMessage = valueStr;
    
    if (outputMask & DEBUG_USB) {
        Serial.println(value);
    }
    
    if ((outputMask & DEBUG_BLUETOOTH) && bluetoothSerial) {
        bluetoothSerial->println(value);
    }
    
    if ((outputMask & DEBUG_SD) && sdEnabled) {
        logFile.println(value);
    }
}

void Debug::println(float value) {
    String valueStr = String(value);
    
    // Controlla se il messaggio è uguale all'ultimo inviato
    if (valueStr == lastMessage) {
        return; // Non inviare messaggi duplicati
    }
    
    lastMessage = valueStr;
    
    if (outputMask & DEBUG_USB) {
        Serial.println(value);
    }
    
    if ((outputMask & DEBUG_BLUETOOTH) && bluetoothSerial) {
        bluetoothSerial->println(value);
    }
    
    if ((outputMask & DEBUG_SD) && sdEnabled) {
        logFile.println(value);
    }
}

void Debug::println(double value) {
    String valueStr = String(value);
    
    // Controlla se il messaggio è uguale all'ultimo inviato
    if (valueStr == lastMessage) {
        return; // Non inviare messaggi duplicati
    }
    
    lastMessage = valueStr;
    
    if (outputMask & DEBUG_USB) {
        Serial.println(value);
    }
    
    if ((outputMask & DEBUG_BLUETOOTH) && bluetoothSerial) {
        bluetoothSerial->println(value);
    }
    
    if ((outputMask & DEBUG_SD) && sdEnabled) {
        logFile.println(value);
    }
}

void Debug::println() {
    if (outputMask & DEBUG_USB) {
        Serial.println();
    }
    
    if ((outputMask & DEBUG_BLUETOOTH) && bluetoothSerial) {
        bluetoothSerial->println();
    }
    
    if ((outputMask & DEBUG_SD) && sdEnabled) {
        logFile.println();
    }
}

void Debug::printf(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    print(buffer);
}

void Debug::flush() {
    if (sdEnabled && logFile) {
        logFile.flush();
    }
}
