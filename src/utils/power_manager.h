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
    
    void _setBrightnessSmooth(uint8_t start, uint8_t target);
};

extern PowerManager powerManager;

#endif
