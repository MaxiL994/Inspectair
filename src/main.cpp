/**
 * ═══════════════════════════════════════════════════════════════════════════
 * INSPECTAIR - LUFTQUALITÄTSMESSGERÄT
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Modulare Architektur v3.0 mit LVGL UI
 * Display: 480x320 (ST7796S) via LovyanGFX
 * Framework: LVGL 9.x
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
#define WIFI_SSID     "AndroidAP3a99     j    vb h 7  b"
#define WIFI_PASSWORD "12345678"

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

void setup() {
    // Serial für Debugging
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n═══════════════════════════════════════════════════════════════");
    Serial.println("              INSPECTAIR v3.0 - LVGL UI");
    Serial.println("═══════════════════════════════════════════════════════════════\n");
    
    // === LVGL UND DISPLAY INITIALISIEREN ===
    Serial.println("[INIT] LVGL Display initialisieren...");
    lvgl_init();
    
    // === UI ERSTELLEN ===
    Serial.println("[INIT] UI erstellen...");
    ui_init();
    
    // Erstes Screen-Update um UI anzuzeigen
    lvgl_loop();
    
    // === WLAN UND UHRZEIT ===
    Serial.println("[INIT] WLAN verbinden...");
    Serial.printf("       SSID: %s\n", WIFI_SSID);
    myClock.begin(WIFI_SSID, WIFI_PASSWORD);
    
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
    Serial.println("[INFO] Zeit-Update jede Sekunde (äquidistant)\n");
}

void loop() {
    // === LVGL LOOP (WICHTIG!) ===
    // Muss regelmäßig aufgerufen werden für UI Updates
    lvgl_loop();
    
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
