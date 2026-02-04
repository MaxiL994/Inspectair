/**
 * ═══════════════════════════════════════════════════════════════════════════
 * INSPECTAIR - AIR QUALITY MONITOR
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Modular architecture v3.0 with LVGL UI (Multi-Screen)
 * Display: 480x320 (ST7796S) via LovyanGFX
 * Framework: LVGL 9.x
 *
 * Screens:
 *   1. Overview: Large AQI + 2 tiles (Temp, Humidity)
 *   2. Detail: Small AQI + 4 tiles (all values)
 *
 * WiFi: AndroidAP3a99 / 12345678
 */

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

// LVGL und Display
#include <lvgl.h>
#include "display/lvgl_driver.h"
#include "display/ui_manager.h"

// Projekt-Header
#include "pins.h"
#include "sensor_types.h"
#include "sensors/aht_sgp.h"
#include "sensors/mhz19c.h"
#include "sensors/pms5003.h"
#include "sensors/ld2410c.h"
#include "WifiClock.h"
#include "utils/sensor_filter.h"
#include "utils/sensor_history.h"

// ============================================
// WIFI CONFIGURATION
// ============================================
#define WIFI_SSID_1     "AndroidAP3a99     j    vb h 7  b"
#define WIFI_PASSWORD_1 "12345678"

#define WIFI_SSID_2     "Vodafone-C414"
#define WIFI_PASSWORD_2 "MXAZZeReKZt2NMKE"

// ============================================
// UI BUTTON CONFIGURATION
// ============================================
// Button for switching between UI screens
// Enable once button is connected
#define UI_BUTTON_ENABLED

#ifdef UI_BUTTON_ENABLED
static unsigned long lastButtonPress = 0;
static const unsigned long BUTTON_DEBOUNCE_MS = 250;  // Debounce time
static bool lastButtonState = HIGH;  // Button is active LOW with pullup
#endif

// ============================================
// GLOBAL OBJECTS
// ============================================
WifiClock myClock;
static SensorReadings readings = {0};

// Timing
static unsigned long lastSensorRead = 0;       // Fast sensor reading
static unsigned long lastTimeUpdate = 0;
static unsigned long lastStatusPrint = 0;      // Status output every 30s
static unsigned long last_pms_ok = 0;
static unsigned long last_radar_ok = 0;

// Weekday names for formatted date (German locale)
const char* weekdays[] = {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};
const char* months[] = {"Jan", "Feb", "Mär", "Apr", "Mai", "Jun", 
                        "Jul", "Aug", "Sep", "Okt", "Nov", "Dez"};

/**
 * Formats the current date as "Di, 14. Jan 2026" (German format)
 * @param buf Target buffer for the formatted string
 * @param len Size of the buffer
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
static unsigned long lastButtonDebug = 0;  // For periodic button status output

/**
 * Checks the UI button and switches to the next screen on press.
 * Button is active LOW (pressed = LOW) with internal pullup.
 */
void checkUIButton() {
    bool currentState = digitalRead(PIN_UI_BUTTON);
    
    // Debug: Show button status every 5 seconds
    if (millis() - lastButtonDebug > 5000) {
        lastButtonDebug = millis();
        Serial.printf("[BUTTON] Status: GPIO%d = %s (expected: HIGH when not pressed)\n", 
                      PIN_UI_BUTTON, currentState ? "HIGH" : "LOW");
    }
    
    // Debug: Show every state change immediately
    if (currentState != lastButtonState) {
        Serial.printf("[BUTTON] >>> STATE CHANGE! GPIO%d: %s -> %s <<<\n", PIN_UI_BUTTON, 
                      lastButtonState ? "HIGH" : "LOW",
                      currentState ? "HIGH" : "LOW");
        Serial.flush();  // Output immediately before potential crash
    }
    
    // Detect falling edge (HIGH -> LOW = button pressed)
    if (lastButtonState == HIGH && currentState == LOW) {
        Serial.println("[BUTTON] *** BUTTON PRESSED - before ui_nextScreen() ***");
        Serial.flush();
        
        // Debounce check
        if (millis() - lastButtonPress > BUTTON_DEBOUNCE_MS) {
            lastButtonPress = millis();
            
            Serial.println("[BUTTON] Calling ui_nextScreen()...");
            Serial.flush();
            
            // Switch to next screen
            ui_nextScreen();
            
            Serial.printf("[BUTTON] Screen switched to: %d\n", ui_getCurrentScreen());
            Serial.flush();
        } else {
            Serial.println("[BUTTON] Debounced (ignored)");
        }
    }
    
    lastButtonState = currentState;
}
#endif

void setup() {
    // Serial for debugging
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n═══════════════════════════════════════════════════════════════");
    Serial.println("              INSPECTAIR v3.0 - Multi-Screen UI");
    Serial.println("═══════════════════════════════════════════════════════════════\n");
    
    // === INITIALIZE UI BUTTON ===
    #ifdef UI_BUTTON_ENABLED
    pinMode(PIN_UI_BUTTON, INPUT_PULLUP);
    Serial.printf("[INIT] UI button initialized on GPIO %d\n", PIN_UI_BUTTON);
    
    // Button test at startup
    Serial.println("[INIT] Button test: Press the button within 3 seconds...");
    bool buttonPressed = false;
    unsigned long testStart = millis();
    bool initialState = digitalRead(PIN_UI_BUTTON);
    Serial.printf("[INIT] Button initial state: %s\n", initialState ? "HIGH (not pressed)" : "LOW (pressed?)");
    
    while (millis() - testStart < 3000) {
      bool state = digitalRead(PIN_UI_BUTTON);
      if (state != initialState) {
        Serial.printf("[INIT] >>> Button change detected! %s -> %s <<<\n", 
                      initialState ? "HIGH" : "LOW", state ? "HIGH" : "LOW");
        buttonPressed = true;
        break;
      }
      delay(10);
    }
    
    if (!buttonPressed) {
      Serial.println("[INIT] No button change detected.");
      Serial.println("[INIT] Check wiring:");
      Serial.println("       - Button pin 1 or 2 -> GPIO 21");  
      Serial.println("       - Button pin 3 or 4 -> GND");
      Serial.println("       - IMPORTANT: Connect diagonally (1-4 or 2-3)!");
    } else {
      Serial.println("[INIT] Button works!");
    }
    #endif
    
    // === INITIALIZE LVGL AND DISPLAY ===
    Serial.println("[INIT] Initializing LVGL display...");
    lvgl_init();
    
    // === CREATE UI (both screens) ===
    Serial.println("[INIT] Creating multi-screen UI...");
    ui_init();
    
    // First screen update to show UI
    lvgl_loop();
    
    // === WIFI AND CLOCK ===
    Serial.println("[INIT] Connecting WiFi...");
    Serial.printf("       SSID 1: %s\n", WIFI_SSID_1);
    Serial.printf("       SSID 2: %s\n", WIFI_SSID_2);
    myClock.begin(WIFI_SSID_1, WIFI_PASSWORD_1, WIFI_SSID_2, WIFI_PASSWORD_2);
    
    // === I2C SENSORS ===
    Serial.println("[INIT] I2C sensors (AHT20, SGP40)...");
    if (!sensors_i2c_init()) {
        Serial.println("[ERROR] Could not initialize I2C sensors!");
    }
    
    // === UART SENSORS ===
    Serial.println("[INIT] UART sensors...");
    
    if (!sensors_pms_init()) {
        Serial.println("[ERROR] Could not initialize PMS5003!");
    }
    
    if (!sensors_mhz19_init()) {
        Serial.println("[ERROR] Could not initialize MH-Z19C!");
    }
    
    if (!sensors_radar_init()) {
        Serial.println("[ERROR] Could not initialize LD2410C!");
    }
    
    // === INITIALIZE SENSOR FILTER & HISTORY ===
    Serial.println("[INIT] Sensor filter & history...");
    sensorFilter.begin();
    sensorHistory.begin();
    
    // Initialize timing variables for equidistant intervals
    lastSensorRead = millis();
    lastTimeUpdate = millis();
    
    Serial.println("\n[INFO] Initialization complete!");
    Serial.println("[INFO] Sensor measurement every 2 seconds (equidistant)");
    Serial.println("[INFO] Display update: Climate every 60s, Air every 12s");
    Serial.println("[INFO] Time update every second (equidistant)");
    #ifdef UI_BUTTON_ENABLED
    Serial.printf("[INFO] UI button active on GPIO %d\n", PIN_UI_BUTTON);
    #else
    Serial.println("[INFO] UI button disabled (UI_BUTTON_ENABLED not defined)");
    #endif
    Serial.println();
}

void loop() {
    // === LVGL LOOP (IMPORTANT!) ===
    // Must be called regularly for UI updates
    lvgl_loop();
    
    // === UI BUTTON CHECK ===
    #ifdef UI_BUTTON_ENABLED
    checkUIButton();
    #endif
    
    // === WIFI RECONNECT CHECK ===
    myClock.update();
    
    // === SENSOR HISTORY UPDATE ===
    sensorHistory.update();
    
    // === CONTINUOUS SENSOR READING ===
    // Read PMS5003 continuously (asynchronous)
    if (sensors_pms_read(&readings.pms)) {
        last_pms_ok = millis();
    }
    
    // Read radar continuously
    if (sensors_radar_read(&readings.radar)) {
        last_radar_ok = millis();
    }
    
    // === TIME UPDATE (500ms for smooth seconds display) ===
    if (millis() - lastTimeUpdate >= 500) {
        lastTimeUpdate = millis();  // Not equidistant since seconds display is more important
        
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            // Update time in UI (with seconds)
            ui_updateTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
            
            // Update date
            char dateBuf[32];
            getFormattedDateString(dateBuf, sizeof(dateBuf));
            ui_updateDate(dateBuf);
        }
    }
    
    // === FAST SENSOR READING (every 2 seconds, equidistant) ===
    // Values are smoothed by filter, display only updated periodically
    if (millis() - lastSensorRead >= 2000) {
        lastSensorRead += 2000;  // Fixed intervals instead of drift
        readings.timestamp = millis();
        
        // Read sensors
        bool aht_ok = sensors_aht20_read(&readings.aht);
        bool mhz_ok = sensors_mhz19_read(&readings.mhz);
        
        // SGP40 requires current temp/humidity
        if (aht_ok) {
            sensors_sgp40_read(readings.aht.temperature,
                               readings.aht.humidity,
                               &readings.sgp);
        }
        
        // === PASS VALUES TO FILTER ===
        if (aht_ok) {
            sensorFilter.addClimateMeasurement(readings.aht.temperature, 
                                               readings.aht.humidity);
        }
        
        // Air quality values to filter
        sensorFilter.addAirMeasurement(readings.mhz.co2_ppm,
                                       readings.sgp.voc_index,
                                       readings.pms.PM_AE_UG_2_5);
        
        // === PASS VALUES TO HISTORY ===
        sensorHistory.addMeasurement(readings.aht.temperature,
                                     readings.aht.humidity,
                                     readings.mhz.co2_ppm,
                                     readings.sgp.voc_index,
                                     readings.pms.PM_AE_UG_2_5);
    }
    
    // === DISPLAY UPDATE (SMOOTHED) ===
    bool needsUIUpdate = false;
    
    // Check climate update (every 60s)
    if (sensorFilter.shouldUpdateClimateDisplay()) {
        needsUIUpdate = true;
        Serial.printf("[DISPLAY] Climate update: T=%.1f°C H=%.0f%%\n",
                      sensorFilter.getSmoothedTemp(),
                      sensorFilter.getSmoothedHum());
    }
    
    // Check air quality update (every 12s)
    if (sensorFilter.shouldUpdateAirDisplay()) {
        needsUIUpdate = true;
        Serial.printf("[DISPLAY] Air update: CO2=%ld VOC=%ld PM=%ld\n",
                      sensorFilter.getSmoothedCO2(),
                      sensorFilter.getSmoothedVOC(),
                      sensorFilter.getSmoothedPM25());
    }
    
    // Update UI if needed
    if (needsUIUpdate) {
        ui_updateSensorValues(
            sensorFilter.getSmoothedTemp(),
            sensorFilter.getSmoothedHum(),
            sensorFilter.getSmoothedCO2(),
            sensorFilter.getSmoothedPM25(),
            sensorFilter.getSmoothedVOC()
        );
    }
    
    // === STATUS OUTPUT (every 30 seconds) ===
    if (millis() - lastStatusPrint > 30000) {
        lastStatusPrint = millis();
        
        Serial.printf("\n[RAW]      T:%.1f H:%.0f CO2:%ld VOC:%ld PM2.5:%u Radar:%d\n",
                      readings.aht.temperature,
                      readings.aht.humidity,
                      readings.mhz.co2_ppm,
                      readings.sgp.voc_index,
                      readings.pms.PM_AE_UG_2_5,
                      readings.radar.presence);
        Serial.printf("[SMOOTHED] T:%.1f H:%.0f CO2:%ld VOC:%ld PM2.5:%ld\n",
                      sensorFilter.getSmoothedTemp(),
                      sensorFilter.getSmoothedHum(),
                      sensorFilter.getSmoothedCO2(),
                      sensorFilter.getSmoothedVOC(),
                      sensorFilter.getSmoothedPM25());
        Serial.printf("[HISTORY]  Entries: %d\n", sensorHistory.getEntryCount());
    }
    
    // Give CPU time to FreeRTOS tasks
    yield();
}
