/**
 * ═══════════════════════════════════════════════════════════════════════════
 * INSPECTAIR - SENSOR FILTER & SMOOTHING
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Gleitender Mittelwert (Moving Average) für alle Sensorwerte.
 * Trennung von Messung, Filterung und Anzeige.
 *
 * Konfiguration:
 * - Temperatur/Feuchte: Träge (Update alle 60s)
 * - CO2/VOC/PM: Volatil (Glättung über 30-60s, Update alle 10-15s)
 */

#ifndef SENSOR_FILTER_H
#define SENSOR_FILTER_H

#include <Arduino.h>
#include "sensor_types.h"

// ═══════════════════════════════════════════════════════════════════════════
// KONFIGURATION - Anpassbar je nach Bedarf
// ═══════════════════════════════════════════════════════════════════════════

// Messintervalle (in Millisekunden)
#define MEASURE_INTERVAL_CLIMATE    10000   // Temp/Feuchte: alle 10 Sekunden messen
#define MEASURE_INTERVAL_AIR        3000    // CO2/VOC/PM: alle 3 Sekunden messen

// Anzeige-Update-Intervalle (in Millisekunden)
#define DISPLAY_INTERVAL_CLIMATE    60000   // Temp/Feuchte: alle 60 Sekunden anzeigen
#define DISPLAY_INTERVAL_AIR        12000   // CO2/VOC/PM: alle 12 Sekunden anzeigen

// Ringpuffer-Größen für Moving Average
#define BUFFER_SIZE_CLIMATE         6       // 6 Messungen * 10s = 60s Fenster
#define BUFFER_SIZE_AIR             20      // 20 Messungen * 3s = 60s Fenster

// ═══════════════════════════════════════════════════════════════════════════
// RINGPUFFER-TEMPLATE FÜR GLEITENDEN MITTELWERT
// ═══════════════════════════════════════════════════════════════════════════

template<typename T, int SIZE>
class RingBuffer {
private:
    T buffer[SIZE];
    int head = 0;
    int count = 0;
    T sum = 0;

public:
    void reset() {
        head = 0;
        count = 0;
        sum = 0;
        for (int i = 0; i < SIZE; i++) {
            buffer[i] = 0;
        }
    }

    void add(T value) {
        // Alten Wert vom Summe abziehen (wenn Puffer voll)
        if (count == SIZE) {
            sum -= buffer[head];
        }
        
        // Neuen Wert speichern
        buffer[head] = value;
        sum += value;
        
        // Head weiterbewegen
        head = (head + 1) % SIZE;
        
        // Count erhöhen (max SIZE)
        if (count < SIZE) {
            count++;
        }
    }

    T getAverage() const {
        if (count == 0) return 0;
        return sum / count;
    }

    // Für float-Typen
    float getAverageFloat() const {
        if (count == 0) return 0.0f;
        return (float)sum / (float)count;
    }

    int getCount() const {
        return count;
    }

    bool isFull() const {
        return count == SIZE;
    }

    T getLatest() const {
        if (count == 0) return 0;
        int idx = (head - 1 + SIZE) % SIZE;
        return buffer[idx];
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// SENSOR FILTER KLASSE
// ═══════════════════════════════════════════════════════════════════════════

class SensorFilter {
private:
    // Ringpuffer für Klimawerte (träge)
    RingBuffer<float, BUFFER_SIZE_CLIMATE> tempBuffer;
    RingBuffer<float, BUFFER_SIZE_CLIMATE> humBuffer;
    
    // Ringpuffer für Luftqualität (volatil)
    RingBuffer<int32_t, BUFFER_SIZE_AIR> co2Buffer;
    RingBuffer<int32_t, BUFFER_SIZE_AIR> vocBuffer;
    RingBuffer<int32_t, BUFFER_SIZE_AIR> pm25Buffer;
    
    // Timing für Messungen
    unsigned long lastClimateMeasure = 0;
    unsigned long lastAirMeasure = 0;
    
    // Timing für Display-Updates
    unsigned long lastClimateDisplay = 0;
    unsigned long lastAirDisplay = 0;
    
    // Letzte angezeigte (geglättete) Werte
    float displayTemp = 0;
    float displayHum = 0;
    int32_t displayCO2 = 0;
    int32_t displayVOC = 0;
    int32_t displayPM25 = 0;
    
    // Flags für Display-Update-Benachrichtigung
    bool climateNeedsUpdate = false;
    bool airNeedsUpdate = false;

public:
    /**
     * Initialisiert den Filter
     */
    void begin();
    
    /**
     * Fügt neue Rohmesswerte hinzu
     * Wird intern je nach Intervall gefiltert
     */
    void addClimateMeasurement(float temp, float humidity);
    void addAirMeasurement(int32_t co2, int32_t voc, int32_t pm25);
    
    /**
     * Prüft ob ein Display-Update nötig ist
     */
    bool shouldUpdateClimateDisplay();
    bool shouldUpdateAirDisplay();
    
    /**
     * Gibt die geglätteten Werte für die Anzeige zurück
     */
    float getSmoothedTemp() const { return displayTemp; }
    float getSmoothedHum() const { return displayHum; }
    int32_t getSmoothedCO2() const { return displayCO2; }
    int32_t getSmoothedVOC() const { return displayVOC; }
    int32_t getSmoothedPM25() const { return displayPM25; }
    
    /**
     * Gibt die aktuellen Rohwerte zurück (für Debugging)
     */
    float getRawTemp() const { return tempBuffer.getLatest(); }
    float getRawHum() const { return humBuffer.getLatest(); }
    int32_t getRawCO2() const { return co2Buffer.getLatest(); }
    int32_t getRawVOC() const { return vocBuffer.getLatest(); }
    int32_t getRawPM25() const { return pm25Buffer.getLatest(); }
    
    /**
     * Füllt eine SensorReadings-Struktur mit geglätteten Werten
     */
    void fillSmoothedReadings(SensorReadings& readings);
    
    /**
     * Debug-Ausgabe
     */
    void printStatus();
};

// Globale Instanz
extern SensorFilter sensorFilter;

#endif // SENSOR_FILTER_H
