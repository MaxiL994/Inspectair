#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>

class PowerManager {
public:
    // Konfiguration
    static const uint32_t PRESENCE_TIMEOUT_MS = 45000; // 45 Sekunden
    static const uint8_t BRIGHTNESS_ACTIVE = 255;      // 100%
    static const uint8_t BRIGHTNESS_DIMMED = 30;       // ~12% (Angenehm dunkel, aber lesbar)
    static const uint8_t BRIGHTNESS_OFF = 0;           // (Optional ganz aus)

    PowerManager();
    
    // Muss im setup() aufgerufen werden
    void begin();
    
    // Muss im loop() aufgerufen werden
    // presenceDetected: true wenn Radar Bewegung meldet (oder Button gedrückt wurde)
    void update(bool presenceDetected);
    
    // Manuelles Aufwecken (z.B. bei Button-Druck)
    void wakeUp();

private:
    unsigned long _lastActivityTime;
    bool _isDimmed;
    
    // Non-blocking Fade (blockiert loop() nicht mehr!)
    uint8_t _fadeCurrent;
    uint8_t _fadeTarget;
    unsigned long _lastFadeStep;
    uint8_t _fadeStepInterval;  // ms zwischen Schritten
    bool _isFading;
    
    void _startFade(uint8_t target);
    void _updateFade();
};

extern PowerManager powerManager;

#endif
