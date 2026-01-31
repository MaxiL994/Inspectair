/**
 * ═══════════════════════════════════════════════════════════════════════════
 * INSPECTAIR - LUFTQUALITÄTSMESSGERÄT
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Modulare Architektur v3.0 mit LVGL UI (Multi-Screen)
 * Display: 480x320 (ST7796S) via LovyanGFX
 * Framework: LVGL 9.x
 *
 * Screens:
 *   1. Übersicht: Große AQI + 2 Kacheln (Temp, Feuchte)
 *   2. Detail: Kleine AQI + 4 Kacheln (alle Werte)
 *
 * WLAN: AndroidAP3a99 / 12345678
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
// WLAN KONFIGURATION
// ============================================
#define WIFI_SSID_1     "AndroidAP3a99     j    vb h 7  b"
#define WIFI_PASSWORD_1 "12345678"

#define WIFI_SSID_2     "Vodafone-C414"
#define WIFI_PASSWORD_2 "MXAZZeReKZt2NMKE"

// ============================================
// UI BUTTON KONFIGURATION (auskommentiert)
// ============================================
// Button zum Umschalten zwischen UI Screens
// Aktivieren sobald Button angeschlossen ist
#define UI_BUTTON_ENABLED

#ifdef UI_BUTTON_ENABLED
static unsigned long lastButtonPress = 0;
static const unsigned long BUTTON_DEBOUNCE_MS = 250;  // Entprell-Zeit
static bool lastButtonState = HIGH;  // Button ist Active LOW mit Pullup
#endif

// ============================================
// GLOBALE OBJEKTE
// ============================================
WifiClock myClock;
static SensorReadings readings = {0};

// Timing
static unsigned long lastSensorRead = 0;       // Schnelles Sensor-Auslesen
static unsigned long lastTimeUpdate = 0;
static unsigned long lastStatusPrint = 0;      // Status-Ausgabe alle 30s
static unsigned long last_pms_ok = 0;
static unsigned long last_radar_ok = 0;

// Wochentags-Namen für formatiertes Datum
const char* weekdays[] = {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};
const char* months[] = {"Jan", "Feb", "Mär", "Apr", "Mai", "Jun", 
                        "Jul", "Aug", "Sep", "Okt", "Nov", "Dez"};

/**
 * Formatiert das aktuelle Datum im Format "Di, 14. Jan"
 */
String getFormattedDateString() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "--.--.----";
    }
    
    char buf[32];
    snprintf(buf, sizeof(buf), "%s, %d. %s", 
             weekdays[timeinfo.tm_wday],
             timeinfo.tm_mday,
             months[timeinfo.tm_mon]);
    return String(buf);
}

// ============================================
// UI BUTTON HANDLER
// ============================================
#ifdef UI_BUTTON_ENABLED
static unsigned long lastButtonDebug = 0;  // Für periodische Button-Status-Ausgabe

/**
 * Prüft den UI-Button und wechselt bei Tastendruck zum nächsten Screen
 * Button ist Active LOW (gedrückt = LOW) mit internem Pullup
 */
void checkUIButton() {
    bool currentState = digitalRead(PIN_UI_BUTTON);
    
    // Debug: Zeige Button-Status alle 5 Sekunden
    if (millis() - lastButtonDebug > 5000) {
        lastButtonDebug = millis();
        Serial.printf("[BUTTON] Status: GPIO%d = %s (erwartet: HIGH wenn nicht gedrückt)\n", 
                      PIN_UI_BUTTON, currentState ? "HIGH" : "LOW");
    }
    
    // Debug: Zeige jeden Zustandswechsel sofort
    if (currentState != lastButtonState) {
        Serial.printf("[BUTTON] >>> WECHSEL! GPIO%d: %s -> %s <<<\n", PIN_UI_BUTTON, 
                      lastButtonState ? "HIGH" : "LOW",
                      currentState ? "HIGH" : "LOW");
        Serial.flush();  // Sofort ausgeben vor möglichem Crash
    }
    
    // Fallende Flanke erkennen (HIGH -> LOW = Button gedrückt)
    if (lastButtonState == HIGH && currentState == LOW) {
        Serial.println("[BUTTON] *** TASTE GEDRÜCKT - vor ui_nextScreen() ***");
        Serial.flush();
        
        // Entprell-Check
        if (millis() - lastButtonPress > BUTTON_DEBOUNCE_MS) {
            lastButtonPress = millis();
            
            Serial.println("[BUTTON] Rufe ui_nextScreen() auf...");
            Serial.flush();
            
            // Zum nächsten Screen wechseln
            ui_nextScreen();
            
            Serial.printf("[BUTTON] Screen gewechselt zu: %d\n", ui_getCurrentScreen());
            Serial.flush();
        } else {
            Serial.println("[BUTTON] Entprellt (ignoriert)");
        }
    }
    
    lastButtonState = currentState;
}
#endif

void setup() {
    // Serial für Debugging
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n═══════════════════════════════════════════════════════════════");
    Serial.println("              INSPECTAIR v3.0 - Multi-Screen UI");
    Serial.println("═══════════════════════════════════════════════════════════════\n");
    
    // === UI BUTTON INITIALISIEREN ===
    #ifdef UI_BUTTON_ENABLED
    pinMode(PIN_UI_BUTTON, INPUT_PULLUP);
    Serial.printf("[INIT] UI-Button auf GPIO %d initialisiert\n", PIN_UI_BUTTON);
    
    // Button-Test beim Start
    Serial.println("[INIT] Button-Test: Drücke den Button innerhalb 3 Sekunden...");
    bool buttonPressed = false;
    unsigned long testStart = millis();
    bool initialState = digitalRead(PIN_UI_BUTTON);
    Serial.printf("[INIT] Button Startzustand: %s\n", initialState ? "HIGH (nicht gedrückt)" : "LOW (gedrückt?)");
    
    while (millis() - testStart < 3000) {
      bool state = digitalRead(PIN_UI_BUTTON);
      if (state != initialState) {
        Serial.printf("[INIT] >>> Button-Wechsel erkannt! %s -> %s <<<\n", 
                      initialState ? "HIGH" : "LOW", state ? "HIGH" : "LOW");
        buttonPressed = true;
        break;
      }
      delay(10);
    }
    
    if (!buttonPressed) {
      Serial.println("[INIT] Kein Button-Wechsel erkannt.");
      Serial.println("[INIT] Prüfe Verkabelung:");
      Serial.println("       - Button Pin 1 oder 2 -> GPIO 21");  
      Serial.println("       - Button Pin 3 oder 4 -> GND");
      Serial.println("       - WICHTIG: Diagonal verbinden (1↔4 oder 2↔3)!");
    } else {
      Serial.println("[INIT] Button funktioniert!");
    }
    #endif
    
    // === LVGL UND DISPLAY INITIALISIEREN ===
    Serial.println("[INIT] LVGL Display initialisieren...");
    lvgl_init();
    
    // === UI ERSTELLEN (beide Screens) ===
    Serial.println("[INIT] Multi-Screen UI erstellen...");
    ui_init();
    
    // Erstes Screen-Update um UI anzuzeigen
    lvgl_loop();
    
    // === WLAN UND UHRZEIT ===
    Serial.println("[INIT] WLAN verbinden...");
    Serial.printf("       SSID 1: %s\n", WIFI_SSID_1);
    Serial.printf("       SSID 2: %s\n", WIFI_SSID_2);
    myClock.begin(WIFI_SSID_1, WIFI_PASSWORD_1, WIFI_SSID_2, WIFI_PASSWORD_2);
    
    // === I2C SENSOREN ===
    Serial.println("[INIT] I2C-Sensoren (AHT20, SGP40)...");
    if (!sensors_i2c_init()) {
        Serial.println("[ERROR] I2C-Sensoren konnten nicht initialisiert werden!");
    }
    
    // === UART SENSOREN ===
    Serial.println("[INIT] UART-Sensoren...");
    
    if (!sensors_pms_init()) {
        Serial.println("[ERROR] PMS5003 konnte nicht initialisiert werden!");
    }
    
    if (!sensors_mhz19_init()) {
        Serial.println("[ERROR] MH-Z19C konnte nicht initialisiert werden!");
    }
    
    if (!sensors_radar_init()) {
        Serial.println("[ERROR] LD2410C konnte nicht initialisiert werden!");
    }
    
    // === SENSOR FILTER & HISTORIE INITIALISIEREN ===
    Serial.println("[INIT] Sensor-Filter & Historie...");
    sensorFilter.begin();
    sensorHistory.begin();
    
    // Timing-Variablen initialisieren für äquidistante Intervalle
    lastSensorRead = millis();
    lastTimeUpdate = millis();
    
    Serial.println("\n[INFO] Initialisierung abgeschlossen!");
    Serial.println("[INFO] Sensor-Messung alle 2 Sekunden (äquidistant)");
    Serial.println("[INFO] Display-Update: Klima alle 60s, Luft alle 12s");
    Serial.println("[INFO] Zeit-Update jede Sekunde (äquidistant)");
    #ifdef UI_BUTTON_ENABLED
    Serial.println("[INFO] UI-Button aktiv auf GPIO " + String(PIN_UI_BUTTON));
    #else
    Serial.println("[INFO] UI-Button deaktiviert (UI_BUTTON_ENABLED nicht definiert)");
    #endif
    Serial.println();
}

void loop() {
    // === LVGL LOOP (WICHTIG!) ===
    // Muss regelmäßig aufgerufen werden für UI Updates
    lvgl_loop();
    
    // === UI BUTTON CHECK (auskommentiert) ===
    #ifdef UI_BUTTON_ENABLED
    checkUIButton();
    #endif
    
    // === WIFI RECONNECT CHECK ===
    myClock.update();
    
    // === SENSOR HISTORIE UPDATE ===
    sensorHistory.update();
    
    // === KONTINUIERLICHE SENSOR-LESEVORGÄNGE ===
    // PMS5003 kontinuierlich auslesen (asynchron)
    if (sensors_pms_read(&readings.pms)) {
        last_pms_ok = millis();
    }
    
    // Radar kontinuierlich auslesen
    if (sensors_radar_read(&readings.radar)) {
        last_radar_ok = millis();
    }
    
    // === UHRZEIT UPDATE (jede Sekunde, äquidistant) ===
    if (millis() - lastTimeUpdate >= 1000) {
        lastTimeUpdate += 1000;  // Feste Intervalle statt Drift
        
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            // Uhrzeit in UI aktualisieren (mit Sekunden)
            ui_updateTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
            
            // Datum aktualisieren
            ui_updateDate(getFormattedDateString().c_str());
        }
    }
    
    // === SCHNELLES SENSOR-AUSLESEN (alle 2 Sekunden, äquidistant) ===
    // Werte werden vom Filter geglättet, Display nur periodisch aktualisiert
    if (millis() - lastSensorRead >= 2000) {
        lastSensorRead += 2000;  // Feste Intervalle statt Drift
        readings.timestamp = millis();
        
        // Sensoren auslesen
        bool aht_ok = sensors_aht20_read(&readings.aht);
        bool mhz_ok = sensors_mhz19_read(&readings.mhz);
        
        // SGP40 benötigt aktuelle Temp/Humidity
        if (aht_ok) {
            sensors_sgp40_read(readings.aht.temperature,
                               readings.aht.humidity,
                               &readings.sgp);
        }
        
        // === WERTE AN FILTER ÜBERGEBEN ===
        if (aht_ok) {
            sensorFilter.addClimateMeasurement(readings.aht.temperature, 
                                               readings.aht.humidity);
        }
        
        // Luftqualitätswerte zum Filter
        sensorFilter.addAirMeasurement(readings.mhz.co2_ppm,
                                       readings.sgp.voc_index,
                                       readings.pms.PM_AE_UG_2_5);
        
        // === WERTE AN HISTORIE ÜBERGEBEN ===
        sensorHistory.addMeasurement(readings.aht.temperature,
                                     readings.aht.humidity,
                                     readings.mhz.co2_ppm,
                                     readings.sgp.voc_index,
                                     readings.pms.PM_AE_UG_2_5);
    }
    
    // === DISPLAY UPDATE (GEGLÄTTET) ===
    bool needsUIUpdate = false;
    
    // Klima-Update prüfen (alle 60s)
    if (sensorFilter.shouldUpdateClimateDisplay()) {
        needsUIUpdate = true;
        Serial.printf("[DISPLAY] Klima-Update: T=%.1f°C H=%.0f%%\n",
                      sensorFilter.getSmoothedTemp(),
                      sensorFilter.getSmoothedHum());
    }
    
    // Luftqualitäts-Update prüfen (alle 12s)
    if (sensorFilter.shouldUpdateAirDisplay()) {
        needsUIUpdate = true;
        Serial.printf("[DISPLAY] Luft-Update: CO2=%ld VOC=%ld PM=%ld\n",
                      sensorFilter.getSmoothedCO2(),
                      sensorFilter.getSmoothedVOC(),
                      sensorFilter.getSmoothedPM25());
    }
    
    // UI aktualisieren wenn nötig
    if (needsUIUpdate) {
        ui_updateSensorValues(
            sensorFilter.getSmoothedTemp(),
            sensorFilter.getSmoothedHum(),
            sensorFilter.getSmoothedCO2(),
            sensorFilter.getSmoothedPM25(),
            sensorFilter.getSmoothedVOC()
        );
    }
    
    // === STATUS-AUSGABE (alle 30 Sekunden) ===
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
        Serial.printf("[HISTORY]  Einträge: %d\n", sensorHistory.getEntryCount());
    }
    
    // Kleine Pause für Stabilität
    delay(5);
}
