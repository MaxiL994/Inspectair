#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>

class PowerManager {
public:
    // Konfiguration - Timeouts
    static const uint32_t PRESENCE_TIMEOUT_DIM_MS  = 45000;   // 45s → Dimmen
    static const uint32_t PRESENCE_TIMEOUT_OFF_MS   = 46800000;  // 13h → Display aus + Light Sleep (fuer Langzeittest)
    
    // Helligkeit
    static const uint8_t BRIGHTNESS_ACTIVE = 255;      // 100%
    static const uint8_t BRIGHTNESS_DIMMED = 30;        // ~12%
    static const uint8_t BRIGHTNESS_OFF = 0;            // Komplett aus

    PowerManager();
    
    void begin();
    void update(bool presenceDetected);
    void wakeUp();
    void dim();                  // Manuell dimmen
    
    // Webapp-Steuerung
    void displayOff();           // Display sofort aus
    void enterLightSleep();      // Light Sleep aktivieren
    void exitLightSleep();       // Light Sleep deaktivieren
    
    // Status-Abfragen
    bool isDimmed() const { return _state == STATE_DIMMED; }
    bool isDisplayOff() const { return _state == STATE_OFF || _state == STATE_SLEEPING; }
    bool isSleeping() const { return _state == STATE_SLEEPING; }
    const char* getStateString() const;

private:
    enum DisplayState {
        STATE_ACTIVE,
        STATE_DIMMED,
        STATE_OFF,
        STATE_SLEEPING
    };
    
    DisplayState _state;
    unsigned long _lastActivityTime;
    
    // Non-blocking Fade
    uint8_t _fadeCurrent;
    uint8_t _fadeTarget;
    unsigned long _lastFadeStep;
    uint8_t _fadeStepInterval;
    bool _isFading;
    
    // Light Sleep
    bool _lightSleepEnabled;
    unsigned long _lastSleepWake;
    
    void _startFade(uint8_t target);
    void _updateFade();
    void _setState(DisplayState newState);
};

extern PowerManager powerManager;

#endif
