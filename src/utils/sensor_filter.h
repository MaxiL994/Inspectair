/**
 * ═══════════════════════════════════════════════════════════════════════════
 * INSPECTAIR - SENSOR FILTER & SMOOTHING
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Moving average filter for all sensor values.
 * Separates measurement, filtering and display logic.
 *
 * Configuration:
 * - Temperature/Humidity: Sluggish (update every 60s)
 * - CO2/VOC/PM: Volatile (smoothing over 30-60s, update every 10-15s)
 */

#ifndef SENSOR_FILTER_H
#define SENSOR_FILTER_H

#include <Arduino.h>
#include "sensor_types.h"

// ═══════════════════════════════════════════════════════════════════════════
// CONFIGURATION - Adjustable as needed
// ═══════════════════════════════════════════════════════════════════════════

// Measurement intervals (in milliseconds)
#define MEASURE_INTERVAL_CLIMATE    10000   // Temp/Humidity: measure every 10 seconds
#define MEASURE_INTERVAL_AIR        3000    // CO2/VOC/PM: measure every 3 seconds

// Display update intervals (in milliseconds)
#define DISPLAY_INTERVAL_CLIMATE    60000   // Temp/Humidity: display every 60 seconds
#define DISPLAY_INTERVAL_AIR        12000   // CO2/VOC/PM: display every 12 seconds

// Ring buffer sizes for moving average
#define BUFFER_SIZE_CLIMATE         6       // 6 measurements * 10s = 60s window
#define BUFFER_SIZE_AIR             20      // 20 measurements * 3s = 60s window

// ═══════════════════════════════════════════════════════════════════════════
// RING BUFFER TEMPLATE FOR MOVING AVERAGE
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
        // Subtract old value from sum (if buffer is full)
        if (count == SIZE) {
            sum -= buffer[head];
        }
        
        // Store new value
        buffer[head] = value;
        sum += value;
        
        // Move head forward
        head = (head + 1) % SIZE;
        
        // Increase count (max SIZE)
        if (count < SIZE) {
            count++;
        }
    }

    T getAverage() const {
        if (count == 0) return 0;
        return sum / count;
    }

    // For float types
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
// SENSOR FILTER CLASS
// ═══════════════════════════════════════════════════════════════════════════

class SensorFilter {
private:
    // Ring buffers for climate values (sluggish)
    RingBuffer<float, BUFFER_SIZE_CLIMATE> tempBuffer;
    RingBuffer<float, BUFFER_SIZE_CLIMATE> humBuffer;
    
    // Ring buffers for air quality (volatile)
    RingBuffer<int32_t, BUFFER_SIZE_AIR> co2Buffer;
    RingBuffer<int32_t, BUFFER_SIZE_AIR> vocBuffer;
    RingBuffer<int32_t, BUFFER_SIZE_AIR> pm25Buffer;
    
    // Timing for measurements
    unsigned long lastClimateMeasure = 0;
    unsigned long lastAirMeasure = 0;
    
    // Timing for display updates
    unsigned long lastClimateDisplay = 0;
    unsigned long lastAirDisplay = 0;
    
    // Last displayed (smoothed) values
    float displayTemp = 0;
    float displayHum = 0;
    int32_t displayCO2 = 0;
    int32_t displayVOC = 0;
    int32_t displayPM25 = 0;
    
    // Flags for display update notification
    bool climateNeedsUpdate = false;
    bool airNeedsUpdate = false;

public:
    /**
     * Initializes the filter
     */
    void begin();
    
    /**
     * Adds new raw measurements
     * Internally filtered based on interval
     */
    void addClimateMeasurement(float temp, float humidity);
    void addAirMeasurement(int32_t co2, int32_t voc, int32_t pm25);
    
    /**
     * Checks if a display update is needed
     */
    bool shouldUpdateClimateDisplay();
    bool shouldUpdateAirDisplay();
    
    /**
     * Returns smoothed values for display
     */
    float getSmoothedTemp() const { return displayTemp; }
    float getSmoothedHum() const { return displayHum; }
    int32_t getSmoothedCO2() const { return displayCO2; }
    int32_t getSmoothedVOC() const { return displayVOC; }
    int32_t getSmoothedPM25() const { return displayPM25; }
    
    /**
     * Returns current raw values (for debugging)
     */
    float getRawTemp() const { return tempBuffer.getLatest(); }
    float getRawHum() const { return humBuffer.getLatest(); }
    int32_t getRawCO2() const { return co2Buffer.getLatest(); }
    int32_t getRawVOC() const { return vocBuffer.getLatest(); }
    int32_t getRawPM25() const { return pm25Buffer.getLatest(); }
    
    /**
     * Fills a SensorReadings struct with smoothed values
     */
    void fillSmoothedReadings(SensorReadings& readings);
    
    /**
     * Debug output
     */
    void printStatus();
};

// Global instance
extern SensorFilter sensorFilter;

#endif // SENSOR_FILTER_H
