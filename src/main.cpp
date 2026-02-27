/**
 * ═══════════════════════════════════════════════════════════════════════════
 * INSPECTAIR - LUFTQUALITÄTSMESSGERÄT
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Modulare Architektur v3.1 mit LVGL UI (Multi-Screen)
 * Display: 480x320 (ST7796S) via LovyanGFX
 * Framework: LVGL 9.x
 */

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

// LVGL und Display
#include <lvgl.h>
#include "display/lvgl_driver.h"
#include "display/ui_manager.h"

// Projekt-Header
#include "debug_log.h"
#include "pins.h"
#include "sensor_types.h"
#include "sensors/aht_sgp.h"
#include "sensors/mhz19c.h"
#include "sensors/pms5003.h"
#include "sensors/ld2410c.h"
#include "WifiClock.h"
#include "utils/sensor_filter.h"
#include "utils/sensor_history.h"
#include "utils/power_manager.h"
#include "utils/endurance_test.h"

// Web Remote Control (Handy-Steuerung per Browser)
#define WEBREMOTE_ENABLED
#ifdef WEBREMOTE_ENABLED
#include "web_remote.h"
#endif

// Watchdog Timer
#include <esp_task_wdt.h>

// ============================================
// WLAN KONFIGURATION
// ============================================
#define WIFI_SSID_1     "AndroidAP3a99     j    vb h 7  b"
#define WIFI_PASSWORD_1 "12345678"

#define WIFI_SSID_2     "Vodafone-C414"
#define WIFI_PASSWORD_2 "MXAZZeReKZt2NMKE"

// ============================================
// UI BUTTON
// ============================================
#define UI_BUTTON_ENABLED

#ifdef UI_BUTTON_ENABLED
static unsigned long lastButtonPress = 0;
static const unsigned long BUTTON_DEBOUNCE_MS = 250;
static bool lastButtonState = HIGH;
#endif

// ============================================
// GLOBALE OBJEKTE
// ============================================
WifiClock myClock;
static SensorReadings readings = {0};

// Timing
static unsigned long lastSensorRead = 0;
static unsigned long lastTimeUpdate = 0;
static unsigned long lastStatusPrint = 0;
static unsigned long last_pms_ok = 0;
static unsigned long last_radar_ok = 0;

// Wochentags-Namen
const char* weekdays[] = {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};
const char* months[] = {"Jan", "Feb", "Mär", "Apr", "Mai", "Jun", 
                        "Jul", "Aug", "Sep", "Okt", "Nov", "Dez"};

/**
 * Formatiert das aktuelle Datum im Format "Di, 14. Jan 2026"
 * @param buf Ziel-Buffer für den formatierten String
 * @param len Größe des Buffers
 */
void getFormattedDateString(char* buf, size_t len) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        snprintf(buf, len, "--.--.----");
        return;
    }
    
    snprintf(buf, len, "%s, %d. %s %d", 
             weekdays[timeinfo.tm_wday],
             timeinfo.tm_mday,
             months[timeinfo.tm_mon],
             timeinfo.tm_year + 1900);
}

// ============================================
// UI BUTTON HANDLER
// ============================================
#ifdef UI_BUTTON_ENABLED
/**
 * Prüft Button (Active LOW mit Pullup) und wechselt Screen
 */
void checkUIButton() {
    bool currentState = digitalRead(PIN_UI_BUTTON);
    
    // Fallende Flanke (HIGH → LOW = gedrückt)
    if (lastButtonState == HIGH && currentState == LOW) {
        powerManager.wakeUp();
        
        if (millis() - lastButtonPress > BUTTON_DEBOUNCE_MS) {
            lastButtonPress = millis();
            ui_nextScreen();
            LOG_I("BTN", "Screen → %d", ui_getCurrentScreen());
        }
    }
    lastButtonState = currentState;
}
#endif

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    LOG_BANNER();
    
    // === UI BUTTON ===
    #ifdef UI_BUTTON_ENABLED
    pinMode(PIN_UI_BUTTON, INPUT_PULLUP);
    LOG_I("INIT", "Button GPIO %d", PIN_UI_BUTTON);
    #endif
    
    // === DISPLAY ===
    LOG_I("INIT", "Display + LVGL...");
    lvgl_init();
    ui_init();
    lvgl_loop();
    
    // === WLAN ===
    LOG_I("INIT", "WLAN: %s / %s", WIFI_SSID_1, WIFI_SSID_2);
    myClock.begin(WIFI_SSID_1, WIFI_PASSWORD_1, WIFI_SSID_2, WIFI_PASSWORD_2);
    
    // === SENSOREN ===
    LOG_I("INIT", "I2C-Sensoren (AHT20, SGP40)...");
    if (!sensors_i2c_init()) LOG_E("INIT", "I2C-Sensoren fehlgeschlagen!");
    
    LOG_I("INIT", "UART-Sensoren...");
    if (!sensors_pms_init())   LOG_E("INIT", "PMS5003 fehlgeschlagen!");
    if (!sensors_mhz19_init()) LOG_E("INIT", "MH-Z19C fehlgeschlagen!");
    if (!sensors_radar_init()) LOG_E("INIT", "LD2410C fehlgeschlagen!");
    
    // === FILTER & HISTORIE ===
    sensorFilter.begin();
    sensorHistory.begin();
    
    // === POWER MANAGER ===
    powerManager.begin();
    
    // Timing initialisieren
    lastSensorRead = millis();
    lastTimeUpdate = millis();
    
    // === WEB REMOTE ===
    #ifdef WEBREMOTE_ENABLED
    if (WiFi.status() == WL_CONNECTED) {
        webRemote_begin();
    }
    #endif
    
    // === WATCHDOG (8s) ===
    esp_task_wdt_init(8, true);
    esp_task_wdt_add(NULL);
    
    // === ENDURANCE TEST ===
    enduranceTest.begin();
    
    LOG_I("INIT", "Bereit! Sensoren: 2s | Klima-Display: 60s | Luft-Display: 12s");
    Serial.println();
}

void loop() {
    unsigned long loopStart = millis();
    
    // === WATCHDOG ===
    esp_task_wdt_reset();
    
    // === LVGL ===
    lvgl_loop();
    
    // === UI BUTTON ===
    #ifdef UI_BUTTON_ENABLED
    checkUIButton();
    #endif
    
    // === WEB REMOTE ===
    #ifdef WEBREMOTE_ENABLED
    webRemote_loop();
    #endif
    
    // === WIFI ===
    myClock.update();
    
    // === HISTORIE ===
    sensorHistory.update();
    
    // === SENSOREN (kontinuierlich) ===
    if (sensors_pms_read(&readings.pms)) {
        last_pms_ok = millis();
    }
    if (sensors_radar_read(&readings.radar)) {
        last_radar_ok = millis();
    }
    
    // === POWER MANAGEMENT ===
    static int lastDist = 0;
    static unsigned long lastMoveTime = 0;
    int currentDist = readings.radar.distance;
    bool presenceActive = readings.radar.presence;
    
    if (currentDist > 0 && currentDist < 400 && lastDist > 0) {
        int delta = abs(currentDist - lastDist);
        if (delta > 40 && delta < 250 && currentDist < 120) {
            lastMoveTime = millis();
            LOG_D("MOTION", "Wake! %dcm → %dcm (Δ%d)", lastDist, currentDist, delta);
        }
    }
    if (currentDist > 0) lastDist = currentDist;
    
    bool isMotionDetected = presenceActive || (millis() - lastMoveTime < 2000);
    powerManager.update(isMotionDetected);
    
    // === UHRZEIT (500ms) ===
    if (millis() - lastTimeUpdate >= 500) {
        lastTimeUpdate = millis();
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            ui_updateTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
            char dateBuf[32];
            getFormattedDateString(dateBuf, sizeof(dateBuf));
            ui_updateDate(dateBuf);
        }
    }
    
    // === SENSOR-AUSLESEN (2s, äquidistant) ===
    if (millis() - lastSensorRead >= 2000) {
        lastSensorRead += 2000;
        readings.timestamp = millis();
        
        bool aht_ok = sensors_aht20_read(&readings.aht);
        bool mhz_ok = sensors_mhz19_read(&readings.mhz);
        
        if (aht_ok) {
            sensors_sgp40_read(readings.aht.temperature,
                               readings.aht.humidity,
                               &readings.sgp);
        }
        
        // Endurance: Sensor-Fehler tracken
        if (!aht_ok) enduranceTest.recordSensorError(0);
        if (!mhz_ok) enduranceTest.recordSensorError(2);
        
        // Filter
        if (aht_ok) {
            sensorFilter.addClimateMeasurement(readings.aht.temperature, readings.aht.humidity);
        }
        sensorFilter.addAirMeasurement(readings.mhz.co2_ppm,
                                       readings.sgp.voc_index,
                                       readings.pms.PM_AE_UG_2_5);
        
        // Historie
        sensorHistory.addMeasurement(readings.aht.temperature,
                                     readings.aht.humidity,
                                     readings.mhz.co2_ppm,
                                     readings.sgp.voc_index,
                                     readings.pms.PM_AE_UG_2_5);
    }
    
    // === DISPLAY UPDATE ===
    bool needsUIUpdate = false;
    
    if (sensorFilter.shouldUpdateClimateDisplay()) {
        needsUIUpdate = true;
    }
    if (sensorFilter.shouldUpdateAirDisplay()) {
        needsUIUpdate = true;
    }
    
    if (needsUIUpdate) {
        ui_updateSensorValues(
            sensorFilter.getSmoothedTemp(),
            sensorFilter.getSmoothedHum(),
            sensorFilter.getSmoothedCO2(),
            sensorFilter.getSmoothedPM25(),
            sensorFilter.getSmoothedVOC()
        );
    }
    
    // === ENDURANCE TEST UPDATE ===
    unsigned long loopMs = millis() - loopStart;
    enduranceTest.recordLoop(loopMs);
    enduranceTest.update();
    
    // === STATUS (60s) ===
    if (millis() - lastStatusPrint > 60000) {
        lastStatusPrint = millis();
        
        Serial.printf("── Status ─ T:%.1f H:%.0f CO2:%ld VOC:%ld PM:%ld | Heap:%dK | %s ──\n",
                      sensorFilter.getSmoothedTemp(),
                      sensorFilter.getSmoothedHum(),
                      sensorFilter.getSmoothedCO2(),
                      sensorFilter.getSmoothedVOC(),
                      sensorFilter.getSmoothedPM25(),
                      ESP.getFreeHeap() / 1024,
                      powerManager.getStateString());
    }
    
    yield();
}
