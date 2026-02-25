/**
 * ═══════════════════════════════════════════════════════════════════════════
 * INSPECTAIR - 24h ENDURANCE TEST
 * ═══════════════════════════════════════════════════════════════════════════
 */

#include "endurance_test.h"
#include "../include/debug_log.h"
#include <WiFi.h>

// Globale Instanz
EnduranceTest enduranceTest;

// Sensor-Namen für Reports
static const char* sensorNames[] = {"AHT20", "SGP40", "MH-Z19C", "PMS5003", "LD2410C"};

EnduranceTest::EnduranceTest()
    : _bootCount(0), _startMillis(0), _lastReport(0), _lastWifiCheck(0),
      _loopCount(0), _loopTotalMs(0), _loopMaxMs(0), _loopMinMs(UINT32_MAX),
      _heapMin(UINT32_MAX), _heapAtStart(0),
      _wifiDisconnects(0), _watchdogWarnings(0), _anomalyCount(0),
      _lastWifiConnected(false) {
    memset(_sensorErrors, 0, sizeof(_sensorErrors));
}

void EnduranceTest::begin() {
    _startMillis = millis();
    _heapAtStart = ESP.getFreeHeap();
    _heapMin = _heapAtStart;
    _lastWifiConnected = (WiFi.status() == WL_CONNECTED);
    
    // Boot-Count aus NVS laden und inkrementieren
    _prefs.begin("endurance", false);
    _bootCount = _prefs.getUInt("boots", 0) + 1;
    _prefs.putUInt("boots", _bootCount);
    _prefs.end();
    
    LOG_I("TEST", "Endurance-Test gestartet (Boot #%d, Heap: %dK)",
          _bootCount, _heapAtStart / 1024);
}

void EnduranceTest::update() {
    unsigned long now = millis();
    
    // Heap-Minimum tracken
    uint32_t currentHeap = ESP.getFreeHeap();
    if (currentHeap < _heapMin) {
        _heapMin = currentHeap;
    }
    
    // WiFi-Status prüfen (alle 10s)
    if (now - _lastWifiCheck >= WIFI_CHECK_MS) {
        _lastWifiCheck = now;
        bool connected = (WiFi.status() == WL_CONNECTED);
        
        if (_lastWifiConnected && !connected) {
            _wifiDisconnects++;
            LOG_W("TEST", "WiFi Disconnect #%d", _wifiDisconnects);
        }
        _lastWifiConnected = connected;
    }
    
    // Periodischer Health-Report (alle 5 Min)
    if (now - _lastReport >= REPORT_INTERVAL_MS) {
        _lastReport = now;
        printReport();
    }
}

void EnduranceTest::recordLoop(unsigned long loopMs) {
    _loopCount++;
    _loopTotalMs += loopMs;
    
    if (loopMs > _loopMaxMs) _loopMaxMs = loopMs;
    if (loopMs < _loopMinMs) _loopMinMs = loopMs;
    
    // Watchdog-Warnung bei > 5s
    if (loopMs > 5000) {
        _watchdogWarnings++;
        LOG_W("TEST", "Loop-Dauer %dms > 5s! (Warnung #%d)", loopMs, _watchdogWarnings);
    }
}

void EnduranceTest::recordSensorError(int sensorIdx) {
    if (sensorIdx >= 0 && sensorIdx < SENSOR_COUNT) {
        _sensorErrors[sensorIdx]++;
        
        // Nur bei jedem 10. Fehler loggen (gegen Spam)
        if (_sensorErrors[sensorIdx] <= 3 || _sensorErrors[sensorIdx] % 10 == 0) {
            LOG_W("TEST", "%s Fehler #%d", sensorNames[sensorIdx], _sensorErrors[sensorIdx]);
        }
    }
}

void EnduranceTest::checkAnomalies(float temp, float hum, int32_t co2, int32_t voc, int32_t pm25) {
    bool anomaly = false;
    
    // Temperatur-Ausreißer
    if (temp < -10 || temp > 60) {
        LOG_W("TEST", "Temp-Anomalie: %.1f°C", temp);
        anomaly = true;
    }
    
    // CO2-Spike (> 5000 ppm ist unrealistisch für Innenraum)
    if (co2 > 5000) {
        LOG_W("TEST", "CO2-Anomalie: %ld ppm", co2);
        anomaly = true;
    }
    
    // VOC-Ausreißer
    if (voc > 500) {
        LOG_W("TEST", "VOC-Anomalie: %ld", voc);
        anomaly = true;
    }
    
    // PM2.5-Ausreißer (> 500 µg/m³)
    if (pm25 > 500) {
        LOG_W("TEST", "PM2.5-Anomalie: %ld µg/m³", pm25);
        anomaly = true;
    }
    
    if (anomaly) _anomalyCount++;
}

uint32_t EnduranceTest::getTotalSensorErrors() const {
    uint32_t total = 0;
    for (int i = 0; i < SENSOR_COUNT; i++) {
        total += _sensorErrors[i];
    }
    return total;
}

bool EnduranceTest::isHealthy() const {
    // Gesund = kein Speicherleck, wenig Fehler, keine Watchdog-Probleme
    uint32_t uptimeSec = (millis() - _startMillis) / 1000;
    if (uptimeSec < 60) return true; // Zu früh zum Beurteilen
    
    uint32_t heapLoss = (_heapAtStart > _heapMin) ? _heapAtStart - _heapMin : 0;
    float heapLossPerHour = (float)heapLoss / (uptimeSec / 3600.0f);
    
    if (_watchdogWarnings > 5) return false;
    if (heapLossPerHour > 10000) return false; // > 10KB/h Speicherleck
    if (getTotalSensorErrors() > uptimeSec / 10) return false; // > 10% Fehlerrate
    
    return true;
}

const char* EnduranceTest::getHealthRating() const {
    if (!isHealthy()) return "KRITISCH";
    
    uint32_t totalErrors = getTotalSensorErrors();
    if (totalErrors == 0 && _wifiDisconnects == 0 && _anomalyCount == 0) return "PERFEKT";
    if (totalErrors < 10 && _wifiDisconnects < 3) return "GUT";
    return "AKZEPTABEL";
}

void EnduranceTest::_formatUptime(char* buf, size_t len, uint32_t seconds) {
    uint32_t d = seconds / 86400;
    uint32_t h = (seconds % 86400) / 3600;
    uint32_t m = (seconds % 3600) / 60;
    uint32_t s = seconds % 60;
    
    if (d > 0) {
        snprintf(buf, len, "%dd %02d:%02d:%02d", d, h, m, s);
    } else {
        snprintf(buf, len, "%02d:%02d:%02d", h, m, s);
    }
}

void EnduranceTest::printReport() {
    uint32_t uptimeSec = getUptimeSeconds();
    char uptimeStr[20];
    _formatUptime(uptimeStr, sizeof(uptimeStr), uptimeSec);
    
    uint32_t heapFree = ESP.getFreeHeap();
    uint32_t heapLoss = (_heapAtStart > heapFree) ? _heapAtStart - heapFree : 0;
    float avgLoop = getLoopAvgMs();
    
    Serial.println();
    Serial.println("╔══════════════════════════════════════════╗");
    Serial.println("║       ENDURANCE TEST REPORT              ║");
    Serial.println("╠══════════════════════════════════════════╣");
    Serial.printf( "║ Uptime:    %s (Boot #%d)\n", uptimeStr, _bootCount);
    Serial.printf( "║ Heap:      %dK frei (%dK min, -%dB verloren)\n", 
                   heapFree/1024, _heapMin/1024, heapLoss);
    Serial.printf( "║ Loop:      %.1fms avg, %dms max (%d Zyklen)\n",
                   avgLoop, _loopMaxMs, _loopCount);
    Serial.printf( "║ Sensoren:  ");
    for (int i = 0; i < SENSOR_COUNT; i++) {
        Serial.printf("%s:%d ", sensorNames[i], _sensorErrors[i]);
    }
    Serial.println();
    Serial.printf( "║ WiFi:      %d Disconnects\n", _wifiDisconnects);
    Serial.printf( "║ Warnungen: %d WDT, %d Anomalien\n", _watchdogWarnings, _anomalyCount);
    Serial.printf( "║ Bewertung: %s\n", getHealthRating());
    Serial.println("╚══════════════════════════════════════════╝");
    Serial.println();
}

String EnduranceTest::getJsonReport() {
    uint32_t uptimeSec = getUptimeSeconds();
    char uptimeStr[20];
    _formatUptime(uptimeStr, sizeof(uptimeStr), uptimeSec);
    
    uint32_t heapFree = ESP.getFreeHeap();
    uint32_t heapLoss = (_heapAtStart > heapFree) ? _heapAtStart - heapFree : 0;
    
    String json = "{";
    json += "\"uptime\":\"" + String(uptimeStr) + "\",";
    json += "\"uptimeSec\":" + String(uptimeSec) + ",";
    json += "\"bootCount\":" + String(_bootCount) + ",";
    json += "\"heap\":{";
    json += "\"free\":" + String(heapFree) + ",";
    json += "\"min\":" + String(_heapMin) + ",";
    json += "\"start\":" + String(_heapAtStart) + ",";
    json += "\"lost\":" + String(heapLoss);
    json += "},";
    json += "\"loop\":{";
    json += "\"count\":" + String(_loopCount) + ",";
    json += "\"avgMs\":" + String(getLoopAvgMs(), 1) + ",";
    json += "\"maxMs\":" + String(_loopMaxMs) + ",";
    json += "\"minMs\":" + String(_loopMinMs == UINT32_MAX ? 0 : _loopMinMs);
    json += "},";
    json += "\"errors\":{";
    json += "\"aht\":" + String(_sensorErrors[SENSOR_AHT]) + ",";
    json += "\"sgp\":" + String(_sensorErrors[SENSOR_SGP]) + ",";
    json += "\"mhz\":" + String(_sensorErrors[SENSOR_MHZ]) + ",";
    json += "\"pms\":" + String(_sensorErrors[SENSOR_PMS]) + ",";
    json += "\"radar\":" + String(_sensorErrors[SENSOR_RADAR]) + ",";
    json += "\"total\":" + String(getTotalSensorErrors());
    json += "},";
    json += "\"wifiDisconnects\":" + String(_wifiDisconnects) + ",";
    json += "\"watchdogWarnings\":" + String(_watchdogWarnings) + ",";
    json += "\"anomalies\":" + String(_anomalyCount) + ",";
    json += "\"rating\":\"" + String(getHealthRating()) + "\",";
    json += "\"healthy\":" + String(isHealthy() ? "true" : "false");
    json += "}";
    
    return json;
}
