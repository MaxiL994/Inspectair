#include "power_manager.h"
#include "../display/lvgl_driver.h"
#include "../include/debug_log.h"
#include <esp_sleep.h>
#include <esp_wifi.h>

PowerManager powerManager;

PowerManager::PowerManager() 
    : _state(STATE_ACTIVE), _lastActivityTime(0),
      _fadeCurrent(BRIGHTNESS_ACTIVE), _fadeTarget(BRIGHTNESS_ACTIVE),
      _lastFadeStep(0), _fadeStepInterval(5), _isFading(false),
      _lightSleepEnabled(true), _lastSleepWake(0) {}

void PowerManager::begin() {
    _lastActivityTime = millis();
    _state = STATE_ACTIVE;
    _fadeCurrent = BRIGHTNESS_ACTIVE;
    _fadeTarget = BRIGHTNESS_ACTIVE;
    _isFading = false;
    _lightSleepEnabled = true;
    lvgl_setBrightness(BRIGHTNESS_ACTIVE);
    LOG_I("POWER", "Init (Dim: 45s, Off: 5min, Sleep: ein)");
}

void PowerManager::wakeUp() {
    _lastActivityTime = millis();
    if (_state != STATE_ACTIVE) {
        LOG_I("POWER", "Wake from %s", getStateString());
        _startFade(BRIGHTNESS_ACTIVE);
        _setState(STATE_ACTIVE);
    }
}

void PowerManager::dim() {
    LOG_I("POWER", "Dim (manuell)");
    _startFade(BRIGHTNESS_DIMMED);
    _setState(STATE_DIMMED);
    // Timer so setzen, dass Dimm-Timeout schon abgelaufen ist
    _lastActivityTime = millis() - PRESENCE_TIMEOUT_DIM_MS - 1000;
}

void PowerManager::displayOff() {
    LOG_I("POWER", "Display AUS (manuell)");
    _startFade(BRIGHTNESS_OFF);
    _setState(STATE_OFF);
}

void PowerManager::enterLightSleep() {
    _lightSleepEnabled = true;
    LOG_I("POWER", "Light Sleep EIN");
}

void PowerManager::exitLightSleep() {
    _lightSleepEnabled = false;
    LOG_I("POWER", "Light Sleep AUS");
    if (_state == STATE_SLEEPING) {
        _setState(STATE_OFF);
    }
}

const char* PowerManager::getStateString() const {
    switch (_state) {
        case STATE_ACTIVE:   return "Active";
        case STATE_DIMMED:   return "Dimmed";
        case STATE_OFF:      return "Off";
        case STATE_SLEEPING: return "Sleep";
        default:             return "?";
    }
}

void PowerManager::_setState(DisplayState newState) {
    if (_state != newState) {
        LOG_D("POWER", "%s → %s", getStateString(),
              newState == STATE_ACTIVE ? "Active" :
              newState == STATE_DIMMED ? "Dimmed" :
              newState == STATE_OFF ? "Off" : "Sleep");
        _state = newState;
    }
}

void PowerManager::update(bool presenceDetected) {
    // Non-blocking Fade fortsetzen
    _updateFade();
    
    if (presenceDetected) {
        wakeUp();
        return;
    }
    
    unsigned long elapsed = millis() - _lastActivityTime;
    
    switch (_state) {
        case STATE_ACTIVE:
            if (elapsed > PRESENCE_TIMEOUT_DIM_MS) {
                LOG_D("POWER", "Keine Präsenz → Dimmen");
                _startFade(BRIGHTNESS_DIMMED);
                _setState(STATE_DIMMED);
            }
            break;
            
        case STATE_DIMMED:
            if (_lightSleepEnabled && elapsed > PRESENCE_TIMEOUT_OFF_MS) {
                LOG_D("POWER", "Lange Abwesenheit → Display aus + Sleep");
                _startFade(BRIGHTNESS_OFF);
                _setState(STATE_SLEEPING);
            }
            break;
            
        case STATE_OFF:
            // Manuell ausgeschaltet - bleibt aus bis wakeUp()
            break;
            
        case STATE_SLEEPING:
            // Light Sleep: CPU pausiert kurz zwischen Zyklen für Stromersparnis
            // WiFi bleibt aktiv (für Webapp), Sensoren laufen weiter
            if (!_isFading && millis() - _lastSleepWake > 1000) {
                _lastSleepWake = millis();
                // Kurzer Light Sleep (100ms) - WiFi/BT bleiben aktiv
                esp_sleep_enable_timer_wakeup(100000); // 100ms in Mikrosekunden
                esp_light_sleep_start();
            }
            break;
    }
}

void PowerManager::_startFade(uint8_t target) {
    _fadeTarget = target;
    _isFading = (_fadeCurrent != _fadeTarget);
    _lastFadeStep = millis();
    _fadeStepInterval = (target < _fadeCurrent) ? 10 : 5;
}

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
