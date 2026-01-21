#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// Projekt-Header
#include "pins.h"
#include "colors.h"
#include "display_config.h"
#include "sensor_types.h"
#include "ui_manager.h"
#include "aht_sgp.h"
#include "mhz19c.h"
#include "pms5003.h"
#include "ld2410c.h"

// ============================================
// INSPECTAIR - LUFTQUALITÄTSMESSGERÄT
// Modulare Architektur v2.0
// ============================================

void setup() {
  // Backlight als Output
  pinMode(PIN_TFT_BL, OUTPUT);
  digitalWrite(PIN_TFT_BL, HIGH);

  delay(1000);
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n\n=== INSPECTAIR START ===");
  Serial.println("[INIT] System initialisiert sich...\n");

  // === DISPLAY INITIALISIEREN ===
  Serial.println("[INIT] Display...");
  ui_init();

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

  Serial.println("\n[INFO] Alle Sensoren initialisiert!");
  Serial.println("[INFO] Display-Update alle 2 Sekunden\n");
  
  delay(1000);
  tft.fillScreen(COLOR_BG);
  tft.setTextColor(COLOR_TITLE);
  tft.setTextSize(2);
  tft.setCursor(10, 8);
  tft.print("INSPECTAIR");
}

void loop() {
  static unsigned long lastUpdate = 0;
  static SensorReadings readings = {0};

  // === KONTINUIERLICHE SENSOR-LESEVORGÄNGE ===
  // PMS5003 kontinuierlich auslesen (asynchron)
  sensors_pms_read(&readings.pms);

  // === PERIODISCHE UPDATES (alle 2 Sekunden) ===
  if (millis() - lastUpdate > 2000) {
    lastUpdate = millis();

    // Aktuelle Zeit speichern
    readings.timestamp = millis();

    // === SENSOR AUSLESEN ===
    bool aht_ok = sensors_aht20_read(&readings.aht);
    bool mhz_ok = sensors_mhz19_read(&readings.mhz);
    bool radar_ok = sensors_radar_read(&readings.radar);

    // SGP40 benötigt aktuelle Temp/Humidity
    if (aht_ok) {
      sensors_sgp40_read(readings.aht.temperature, 
                         readings.aht.humidity, 
                         &readings.sgp);
    }

    // === DISPLAY UPDATE ===
    if (aht_ok && mhz_ok) {
      ui_updateDisplay(readings);
    } else {
      ui_showError("Sensor-Fehler!");
    }

    // === SERIAL AUSGABE (für Debugging) ===
    Serial.printf("[%lu] T:%.1f H:%.0f CO2:%ld VOC:%ld PM2.5:%u PM10:%u Radar:%d\n",
                  readings.timestamp,
                  readings.aht.temperature,
                  readings.aht.humidity,
                  readings.mhz.co2_ppm,
                  readings.sgp.voc_index,
                  readings.pms.PM_AE_UG_2_5,
                  readings.pms.PM_AE_UG_10_0,
                  readings.radar.presence);
  }
}
