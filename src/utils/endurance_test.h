/**
 * ═══════════════════════════════════════════════════════════════════════════
 * INSPECTAIR - 24h ENDURANCE TEST
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Automatischer Dauertest-Monitor für 24h Betrieb.
 * Erfasst und analysiert:
 *   - Uptime & Boot-Zähler (NVS-persistent)
 *   - Heap-Speicher (aktuell, minimum, Fragmentierung)
 *   - Loop-Timing (min, max, avg, Watchdog-Warnungen)
 *   - Sensor-Fehler pro Sensor (AHT, SGP, MHZ, PMS, Radar)
 *   - WiFi-Disconnects & Reconnects
 *   - Anomale Sensorwerte (CO2-Spikes, Temp-Ausreißer)
 *   - Periodischer Health-Report (alle 5 Min)
 *   - JSON-Endpoint für Webapp (/api/health)
 */

#ifndef ENDURANCE_TEST_H
#define ENDURANCE_TEST_H

#include <Arduino.h>
#include <Preferences.h>

class EnduranceTest {
public:
    // Sensor-Index-Konstanten
    static const int SENSOR_AHT   = 0;
    static const int SENSOR_SGP   = 1;
    static const int SENSOR_MHZ   = 2;
    static const int SENSOR_PMS   = 3;
    static const int SENSOR_RADAR = 4;
    static const int SENSOR_COUNT = 5;

    EnduranceTest();
    
    /**
     * Initialisiert den Endurance-Test. 
     * Lädt Boot-Count aus NVS, setzt alle Zähler zurück.
     */
    void begin();
    
    /**
     * Periodisch aus loop() aufrufen.
     * Prüft WiFi-Status, druckt Health-Reports, etc.
     */
    void update();
    
    /**
     * Loop-Dauer erfassen (jeder loop()-Durchlauf)
     * @param loopMs Dauer des loop()-Durchlaufs in ms
     */
    void recordLoop(unsigned long loopMs);
    
    /**
     * Sensor-Fehler zählen
     * @param sensorIdx SENSOR_AHT..SENSOR_RADAR
     */
    void recordSensorError(int sensorIdx);
    
    /**
     * Anomale Sensorwerte prüfen und zählen
     * Wird automatisch aus update() aufgerufen wenn Werte vorliegen
     */
    void checkAnomalies(float temp, float hum, int32_t co2, int32_t voc, int32_t pm25);
    
    /**
     * Health-Report auf Serial ausgeben
     */
    void printReport();
    
    /**
     * JSON-String für /api/health Endpoint generieren
     */
    String getJsonReport();
    
    // Getter
    uint32_t getUptimeSeconds() const { return (millis() - _startMillis) / 1000; }
    uint32_t getBootCount() const { return _bootCount; }
    uint32_t getLoopCount() const { return _loopCount; }
    uint32_t getHeapFree() const { return ESP.getFreeHeap(); }
    uint32_t getHeapMin() const { return _heapMin; }
    uint32_t getLoopMaxMs() const { return _loopMaxMs; }
    float getLoopAvgMs() const { return _loopCount > 0 ? (float)_loopTotalMs / _loopCount : 0; }
    uint32_t getSensorErrors(int idx) const { return (idx >= 0 && idx < SENSOR_COUNT) ? _sensorErrors[idx] : 0; }
    uint32_t getTotalSensorErrors() const;
    uint32_t getWifiDisconnects() const { return _wifiDisconnects; }
    uint32_t getWatchdogWarnings() const { return _watchdogWarnings; }
    uint32_t getAnomalyCount() const { return _anomalyCount; }
    bool isHealthy() const;
    const char* getHealthRating() const;

private:
    // Boot-persistent (NVS)
    Preferences _prefs;
    uint32_t _bootCount;
    
    // Timing
    unsigned long _startMillis;
    unsigned long _lastReport;
    unsigned long _lastWifiCheck;
    
    // Loop-Performance
    uint32_t _loopCount;
    uint64_t _loopTotalMs;
    uint32_t _loopMaxMs;
    uint32_t _loopMinMs;
    
    // Speicher
    uint32_t _heapMin;
    uint32_t _heapAtStart;
    
    // Fehler-Zähler
    uint32_t _sensorErrors[SENSOR_COUNT];
    uint32_t _wifiDisconnects;
    uint32_t _watchdogWarnings;  // Loop > 5s
    uint32_t _anomalyCount;
    
    // WiFi-Status-Tracking
    bool _lastWifiConnected;
    
    // Report-Intervall
    static const unsigned long REPORT_INTERVAL_MS = 300000;  // 5 Minuten
    static const unsigned long WIFI_CHECK_MS = 10000;        // 10 Sekunden
    
    void _formatUptime(char* buf, size_t len, uint32_t seconds);
};

extern EnduranceTest enduranceTest;

#endif // ENDURANCE_TEST_H
