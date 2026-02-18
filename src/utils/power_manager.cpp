#include "power_manager.h"
#include "../display/lvgl_driver.h"

PowerManager powerManager;

PowerManager::PowerManager() 
    : _lastActivityTime(0), _isDimmed(false),
      _fadeCurrent(BRIGHTNESS_ACTIVE), _fadeTarget(BRIGHTNESS_ACTIVE),
      _lastFadeStep(0), _fadeStepInterval(5), _isFading(false) {}

void PowerManager::begin() {
    _lastActivityTime = millis();
    _isDimmed = false;
    _fadeCurrent = BRIGHTNESS_ACTIVE;
    _fadeTarget = BRIGHTNESS_ACTIVE;
    _isFading = false;
    lvgl_setBrightness(BRIGHTNESS_ACTIVE);
    Serial.println("[POWER] Manager initialized. Timeout: 45s");
}

void PowerManager::wakeUp() {
    _lastActivityTime = millis();
    if (_isDimmed) {
        Serial.println("[POWER] Waking up display!");
        _startFade(BRIGHTNESS_ACTIVE);
        _isDimmed = false;
    }
}

void PowerManager::update(bool presenceDetected) {
    // Non-blocking Fade jeden loop()-Durchlauf fortsetzen
    _updateFade();
    
    if (presenceDetected) {
        wakeUp();
    } else {
        if (!_isDimmed && (millis() - _lastActivityTime > PRESENCE_TIMEOUT_MS)) {
            Serial.println("[POWER] No presence detected. Dimming display...");
            _startFade(BRIGHTNESS_DIMMED);
            _isDimmed = true;
        }
    }
}

// Startet einen weichen Übergang (non-blocking)
void PowerManager::_startFade(uint8_t target) {
    _fadeTarget = target;
    _isFading = (_fadeCurrent != _fadeTarget);
    _lastFadeStep = millis();
    // Abblenden langsamer (10ms/Step) als Aufblenden (5ms/Step)
    _fadeStepInterval = (target < _fadeCurrent) ? 10 : 5;
}

// Wird jeden loop()-Durchlauf aufgerufen, macht max. 1 Step
void PowerManager::_updateFade() {
    if (!_isFading) return;
    if (millis() - _lastFadeStep < _fadeStepInterval) return;
    _lastFadeStep = millis();
    
    if (_fadeCurrent < _fadeTarget) {
        _fadeCurrent = min((int)_fadeCurrent + 5, (int)_fadeTarget);
    } else {
        _fadeCurrent = max((int)_fadeCurrent - 5, (int)_fadeTarget);
    }
    lvgl_setBrightness(_fadeCurrent);
    
    if (_fadeCurrent == _fadeTarget) {
        _isFading = false;
    }
}
