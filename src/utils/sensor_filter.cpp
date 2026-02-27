/**
 * INSPECTAIR - SENSOR FILTER & SMOOTHING
 */

#include "sensor_filter.h"
#include "../include/debug_log.h"

SensorFilter sensorFilter;

void SensorFilter::begin() {
    tempBuffer.reset();
    humBuffer.reset();
    co2Buffer.reset();
    vocBuffer.reset();
    pm25Buffer.reset();
    
    lastClimateMeasure = 0;
    lastAirMeasure = 0;
    lastClimateDisplay = 0;
    lastAirDisplay = 0;
    
    displayTemp = 0;
    displayHum = 0;
    displayCO2 = 0;
    displayVOC = 0;
    displayPM25 = 0;
    
    LOG_I("FILTER", "Init (Klima: %ds/%ds, Luft: %ds/%ds)",
          MEASURE_INTERVAL_CLIMATE/1000, DISPLAY_INTERVAL_CLIMATE/1000,
          MEASURE_INTERVAL_AIR/1000, DISPLAY_INTERVAL_AIR/1000);
}

void SensorFilter::addClimateMeasurement(float temp, float humidity) {
    unsigned long now = millis();
    
    // Prüfe ob Messintervall erreicht
    if (now - lastClimateMeasure >= MEASURE_INTERVAL_CLIMATE || lastClimateMeasure == 0) {
        lastClimateMeasure = now;
        
        // Werte zum Ringpuffer hinzufügen
        tempBuffer.add(temp);
        humBuffer.add(humidity);
        
        // Debug (optional)
        // Serial.printf("[FILTER] Klima: T=%.1f H=%.0f (Samples: %d)\n", 
        //               temp, humidity, tempBuffer.getCount());
    }
}

void SensorFilter::addAirMeasurement(int32_t co2, int32_t voc, int32_t pm25) {
    unsigned long now = millis();
    
    // Prüfe ob Messintervall erreicht
    if (now - lastAirMeasure >= MEASURE_INTERVAL_AIR || lastAirMeasure == 0) {
        lastAirMeasure = now;
        
        // Werte zum Ringpuffer hinzufügen
        co2Buffer.add(co2);
        vocBuffer.add(voc);
        pm25Buffer.add(pm25);
        
        // Debug (optional)
        // Serial.printf("[FILTER] Luft: CO2=%ld VOC=%ld PM=%ld (Samples: %d)\n", 
        //               co2, voc, pm25, co2Buffer.getCount());
    }
}

bool SensorFilter::shouldUpdateClimateDisplay() {
    unsigned long now = millis();
    
    // Prüfe ob Display-Update-Intervall erreicht
    if (now - lastClimateDisplay >= DISPLAY_INTERVAL_CLIMATE || lastClimateDisplay == 0) {
        // Nur updaten wenn wir Daten haben
        if (tempBuffer.getCount() > 0) {
            lastClimateDisplay = now;
            
            // Geglättete Werte berechnen
            displayTemp = tempBuffer.getAverageFloat();
            displayHum = humBuffer.getAverageFloat();
            
            return true;
        }
    }
    return false;
}

bool SensorFilter::shouldUpdateAirDisplay() {
    unsigned long now = millis();
    
    // Prüfe ob Display-Update-Intervall erreicht
    if (now - lastAirDisplay >= DISPLAY_INTERVAL_AIR || lastAirDisplay == 0) {
        // Nur updaten wenn wir Daten haben
        if (co2Buffer.getCount() > 0) {
            lastAirDisplay = now;
            
            // Geglättete Werte berechnen
            displayCO2 = co2Buffer.getAverage();
            displayVOC = vocBuffer.getAverage();
            displayPM25 = pm25Buffer.getAverage();
            
            return true;
        }
    }
    return false;
}

void SensorFilter::fillSmoothedReadings(SensorReadings& readings) {
    readings.aht.temperature = displayTemp;
    readings.aht.humidity = displayHum;
    readings.mhz.co2_ppm = displayCO2;
    readings.sgp.voc_index = displayVOC;
    readings.pms.PM_AE_UG_2_5 = displayPM25;
}

void SensorFilter::printStatus() {
    Serial.printf("── Filter: T=%.1f/%.1f H=%.0f/%.0f CO2=%ld/%ld VOC=%ld/%ld PM=%ld/%ld ──\n",
                  tempBuffer.getLatest(), tempBuffer.getAverageFloat(),
                  humBuffer.getLatest(), humBuffer.getAverageFloat(),
                  co2Buffer.getLatest(), co2Buffer.getAverage(),
                  vocBuffer.getLatest(), vocBuffer.getAverage(),
                  pm25Buffer.getLatest(), pm25Buffer.getAverage());
}
