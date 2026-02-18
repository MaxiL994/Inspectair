#include "power_manager.h"
#include "../display/lvgl_driver.h"

PowerManager powerManager;

PowerManager::PowerManager() : _lastActivityTime(0), _isDimmed(false) {}

void PowerManager::begin() {
    _lastActivityTime = millis();
    _isDimmed = false;
    lvgl_setBrightness(BRIGHTNESS_ACTIVE);
    Serial.println("[POWER] Manager initialized. Timeout: 10s");
}

void PowerManager::wakeUp() {
    _lastActivityTime = millis();
    if (_isDimmed) {
        Serial.println("[POWER] Waking up display!");
        _setBrightnessSmooth(BRIGHTNESS_DIMMED, BRIGHTNESS_ACTIVE);
        _isDimmed = false;
    }
}

void PowerManager::update(bool presenceDetected) {
    if (presenceDetected) {
        // Bei Bewegung/Präsenz Timer zurücksetzen
        wakeUp();
    } else {
        // Keine Bewegung -> Prüfen ob Timeout abgelaufen
        if (!_isDimmed && (millis() - _lastActivityTime > PRESENCE_TIMEOUT_MS)) {
            Serial.println("[POWER] No presence detected. Dimming display...");
            _setBrightnessSmooth(BRIGHTNESS_ACTIVE, BRIGHTNESS_DIMMED);
            _isDimmed = true;
        }
    }
}

// Weicher Übergang für Edles Look & Feel
void PowerManager::_setBrightnessSmooth(uint8_t start, uint8_t target) {
    if (start < target) {
        // Aufblenden
        for (int i = start; i <= target; i += 5) {
            lvgl_setBrightness(i);
            delay(5);
        }
    } else {
        // Abblenden
        for (int i = start; i >= target; i -= 5) {
            lvgl_setBrightness(i);
            delay(10);
        }
    }
    lvgl_setBrightness(target); // Zielsicherer Abschluss
}
