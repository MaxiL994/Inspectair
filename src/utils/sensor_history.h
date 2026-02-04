/**
 * ═══════════════════════════════════════════════════════════════════════════
 * INSPECTAIR - SENSOR HISTORY (24h Storage)
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Stores aggregated per-minute values for the last 24 hours.
 * Uses ESP32 Preferences (NVS) for persistent storage.
 *
 * Memory usage: ~35KB for 24h (1440 minutes * 24 bytes per entry)
 */

#ifndef SENSOR_HISTORY_H
#define SENSOR_HISTORY_H

#include <Arduino.h>
#include <Preferences.h>
#include "sensor_types.h"

// ═══════════════════════════════════════════════════════════════════════════
// CONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════

#define HISTORY_ENTRIES         1440    // 24h * 60 minutes
#define HISTORY_SAVE_INTERVAL   60000   // Save every 60 seconds
#define HISTORY_PERSIST_INTERVAL 300000 // Write to flash every 5 minutes

// ═══════════════════════════════════════════════════════════════════════════
// DATA STRUCTURE FOR A HISTORY ENTRY
// ═══════════════════════════════════════════════════════════════════════════

struct HistoryEntry {
    uint32_t timestamp;     // Unix timestamp or millis()
    int16_t temp_x10;       // Temperature * 10 (for 0.1° resolution)
    uint8_t humidity;       // Humidity (0-100%)
    uint16_t co2;           // CO2 in ppm
    uint16_t pm25;          // PM2.5 in µg/m³
    uint16_t voc;           // VOC index
    uint8_t reserved;       // Padding for alignment
};

// ═══════════════════════════════════════════════════════════════════════════
// SENSOR HISTORY CLASS
// ═══════════════════════════════════════════════════════════════════════════

class SensorHistory {
private:
    // Ring buffer for 24h history
    HistoryEntry* history = nullptr;
    int head = 0;
    int count = 0;
    
    // Accumulators for per-minute average
    float tempSum = 0;
    float humSum = 0;
    int32_t co2Sum = 0;
    int32_t vocSum = 0;
    int32_t pm25Sum = 0;
    int sampleCount = 0;
    
    // Timing
    unsigned long lastSave = 0;
    unsigned long lastPersist = 0;
    
    // Preferences for flash storage
    Preferences prefs;
    bool initialized = false;

    // Internal storage helper
    void saveToFlash();
    void loadFromFlash();

public:
    /**
     * Initializes history and loads stored data
     */
    bool begin();
    
    /**
     * Cleans up and frees memory
     */
    void end();
    
    /**
     * Adds a new measurement (will be accumulated)
     */
    void addMeasurement(float temp, float hum, int32_t co2, int32_t voc, int32_t pm25);
    
    /**
     * Must be called regularly (in loop)
     * Saves per-minute values and persists periodically
     */
    void update();
    
    /**
     * Returns the number of stored entries
     */
    int getEntryCount() const { return count; }
    
    /**
     * Returns a history entry
     * @param index 0 = oldest, count-1 = newest
     */
    bool getEntry(int index, HistoryEntry& entry) const;
    
    /**
     * Returns the latest entry
     */
    bool getLatestEntry(HistoryEntry& entry) const;
    
    /**
     * Calculates average over the last N minutes
     */
    bool getAverage(int minutes, float& temp, float& hum, 
                    int32_t& co2, int32_t& voc, int32_t& pm25) const;
    
    /**
     * Calculates min/max over the last N minutes
     */
    bool getMinMax(int minutes, 
                   float& tempMin, float& tempMax,
                   float& humMin, float& humMax,
                   int32_t& co2Min, int32_t& co2Max) const;
    
    /**
     * Clears all stored data
     */
    void clear();
    
    /**
     * Debug output
     */
    void printStatus();
    void printLastHours(int hours);
};

// Global instance
extern SensorHistory sensorHistory;

#endif // SENSOR_HISTORY_H
