/*
  MultiClickButton.cpp - Implementation
*/

#include "MultiClickButton.h"

// Global array initialization
MultiClickButton* buttonInstances[MAX_BUTTONS] = {nullptr, nullptr, nullptr, nullptr};
int buttonInstanceCount = 0;

// Constructor
MultiClickButton::MultiClickButton(int pin, unsigned long clickTimeout, unsigned long debounceDelay)
    : _pin(pin),
      _clickTimeout(clickTimeout),
      _debounceDelay(debounceDelay),
      _lastInterruptTime(0),
      _clickCount(0),
      _clickReady(false),
      _buttonPressed(false),
      _systemPaused(false),
      _singleClickCallback(nullptr),
      _doubleClickCallback(nullptr),
      _tripleClickCallback(nullptr) {
}

// Initialize button
void MultiClickButton::begin() {
    pinMode(_pin, INPUT_PULLUP);
    
    if (buttonInstanceCount < MAX_BUTTONS) {
        buttonInstances[buttonInstanceCount] = this;
        
        int interruptNum = digitalPinToInterrupt(_pin);
        switch(buttonInstanceCount) {
            case 0:
                attachInterrupt(interruptNum, buttonISR0, FALLING);
                break;
            case 1:
                attachInterrupt(interruptNum, buttonISR1, FALLING);
                break;
            case 2:
                attachInterrupt(interruptNum, buttonISR2, FALLING);
                break;
            case 3:
                attachInterrupt(interruptNum, buttonISR3, FALLING);
                break;
        }
        buttonInstanceCount++;
    }
}

// Handle interrupt
void MultiClickButton::handleInterrupt() {
    unsigned long currentTime = millis();
    
    // Every FALLING interrupt is a button press
    // But we only accept presses that arrive after at least 150ms from the last one
    // (this eliminates bounce from the previous click release)
    if (currentTime - _lastInterruptTime > 150) {
        _clickCount++;
        _clickReady = true;
        _lastInterruptTime = currentTime;
    }
}

// Update button state
void MultiClickButton::update() {
    if (_clickReady && (millis() - _lastInterruptTime > _clickTimeout)) {
        processClicks();
        _clickCount = 0;
        _clickReady = false;
    }
}

// Process detected clicks
void MultiClickButton::processClicks() {
    switch(_clickCount) {
        case 1:
            // Single click: automatically toggle pause
            togglePause();
            if (_singleClickCallback != nullptr) {
                _singleClickCallback();
            }
            break;
        case 2:
            if (_doubleClickCallback != nullptr) {
                _doubleClickCallback();
            }
            break;
        case 3:
            if (_tripleClickCallback != nullptr) {
                _tripleClickCallback();
            }
            break;
    }
}

// Set callbacks
void MultiClickButton::onSingleClick(CallbackFunction callback) {
    _singleClickCallback = callback;
}

void MultiClickButton::onDoubleClick(CallbackFunction callback) {
    _doubleClickCallback = callback;
}

void MultiClickButton::onTripleClick(CallbackFunction callback) {
    _tripleClickCallback = callback;
}

// Getters
int MultiClickButton::getClickCount() const {
    return _clickCount;
}

bool MultiClickButton::isPaused() const {
    return _systemPaused;
}

// Setters
void MultiClickButton::setPaused(bool paused) {
    _systemPaused = paused;
}

void MultiClickButton::togglePause() {
    _systemPaused = !_systemPaused;
}

// Global ISR functions
void buttonISR0() {
    if (buttonInstances[0] != nullptr) {
        buttonInstances[0]->handleInterrupt();
    }
}

void buttonISR1() {
    if (buttonInstances[1] != nullptr) {
        buttonInstances[1]->handleInterrupt();
    }
}

void buttonISR2() {
    if (buttonInstances[2] != nullptr) {
        buttonInstances[2]->handleInterrupt();
    }
}

void buttonISR3() {
    if (buttonInstances[3] != nullptr) {
        buttonInstances[3]->handleInterrupt();
    }
}