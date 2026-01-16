/*
  MultiClickButton.h - Library for multi-click button detection with interrupts
  Created for Arduino Mega 2560 and compatible boards
*/

#ifndef MULTICLICKBUTTON_H
#define MULTICLICKBUTTON_H

#include <Arduino.h>

// Callback function type
typedef void (*CallbackFunction)();

class MultiClickButton {
public:
    /**
     * Constructor
     * @param pin Arduino pin number where button is connected
     * @param clickTimeout Maximum time between clicks in milliseconds (default: 500ms)
     * @param debounceDelay Debounce delay in milliseconds (default: 50ms)
     */
    MultiClickButton(int pin, unsigned long clickTimeout = 500, unsigned long debounceDelay = 50);
    
    /**
     * Initialize the button (call this in setup())
     */
    void begin();
    
    /**
     * Update button state (call this in loop())
     */
    void update();
    
    /**
     * Set callback for single click
     * Note: Single click automatically toggles pause state before calling the callback
     * @param callback Function to call when single click is detected
     */
    void onSingleClick(CallbackFunction callback);
    
    /**
     * Set callback for double click
     * @param callback Function to call when double click is detected
     */
    void onDoubleClick(CallbackFunction callback);
    
    /**
     * Set callback for triple click
     * @param callback Function to call when triple click is detected
     */
    void onTripleClick(CallbackFunction callback);
    
    /**
     * Get current click count
     * @return Number of clicks detected
     */
    int getClickCount() const;
    
    /**
     * Check if system is paused
     * @return true if paused, false if active
     */
    bool isPaused() const;
    
    /**
     * Set pause state manually
     * @param paused true to pause, false to resume
     */
    void setPaused(bool paused);
    
    /**
     * Toggle pause state
     */
    void togglePause();
    
    /**
     * Handle interrupt (for internal use)
     */
    void handleInterrupt();
    
private:
    int _pin;
    unsigned long _clickTimeout;
    unsigned long _debounceDelay;
    
    volatile unsigned long _lastInterruptTime;
    volatile int _clickCount;
    volatile bool _clickReady;
    volatile bool _buttonPressed;  // Track if button is currently pressed
    
    bool _systemPaused;
    
    CallbackFunction _singleClickCallback;
    CallbackFunction _doubleClickCallback;
    CallbackFunction _tripleClickCallback;
    
    void processClicks();
};

// Maximum number of buttons supported
#define MAX_BUTTONS 4

// Global array for button instances
extern MultiClickButton* buttonInstances[MAX_BUTTONS];
extern int buttonInstanceCount;

// Global ISR functions
void buttonISR0();
void buttonISR1();
void buttonISR2();
void buttonISR3();

#endif