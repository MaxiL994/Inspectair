/**
 * ═══════════════════════════════════════════════════════════════════════════
 * INSPECTAIR - SENSOR FILTER & SMOOTHING
 * ═══════════════════════════════════════════════════════════════════════════
 */

#include "sensor_filter.h"

// Globale Instanz
SensorFilter sensorFilter;

void SensorFilter::begin() {
    // Puffer zurücksetzen
    tempBuffer.reset();
    humBuffer.reset();
    co2Buffer.reset();
    vocBuffer.reset();
    pm25Buffer.reset();
    
    // Timing initialisieren
    lastClimateMeasure = 0;
    lastAirMeasure = 0;
    lastClimateDisplay = 0;
    lastAirDisplay = 0;
    
    // Display-Werte auf 0
    displayTemp = 0;
    displayHum = 0;
    displayCO2 = 0;
    displayVOC = 0;
    displayPM25 = 0;
    
    Serial.println("[FILTER] Sensor-Filter initialisiert");
    Serial.printf("         Klima: Messung alle %ds, Anzeige alle %ds\n", 
                  MEASURE_INTERVAL_CLIMATE/1000, DISPLAY_INTERVAL_CLIMATE/1000);
    Serial.printf("         Luft:  Messung alle %ds, Anzeige alle %ds\n",
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
    Serial.println("\n╔═══════════════════════════════════════════════════════════╗");
    Serial.println("║              SENSOR FILTER STATUS                         ║");
    Serial.println("╠═══════════════════════════════════════════════════════════╣");
    Serial.printf("║ Temperatur: Raw=%.1f°C  Avg=%.1f°C  (%d/%d Samples)      \n",
                  tempBuffer.getLatest(), tempBuffer.getAverageFloat(),
                  tempBuffer.getCount(), BUFFER_SIZE_CLIMATE);
    Serial.printf("║ Feuchte:    Raw=%.0f%%   Avg=%.0f%%   (%d/%d Samples)      \n",
                  humBuffer.getLatest(), humBuffer.getAverageFloat(),
                  humBuffer.getCount(), BUFFER_SIZE_CLIMATE);
    Serial.printf("║ CO2:        Raw=%ld    Avg=%ld    (%d/%d Samples)      \n",
                  co2Buffer.getLatest(), co2Buffer.getAverage(),
                  co2Buffer.getCount(), BUFFER_SIZE_AIR);
    Serial.printf("║ VOC:        Raw=%ld    Avg=%ld    (%d/%d Samples)      \n",
                  vocBuffer.getLatest(), vocBuffer.getAverage(),
                  vocBuffer.getCount(), BUFFER_SIZE_AIR);
    Serial.printf("║ PM2.5:      Raw=%ld    Avg=%ld    (%d/%d Samples)      \n",
                  pm25Buffer.getLatest(), pm25Buffer.getAverage(),
                  pm25Buffer.getCount(), BUFFER_SIZE_AIR);
    Serial.println("╚═══════════════════════════════════════════════════════════╝\n");
}
