/**
 * ═══════════════════════════════════════════════════════════════════════════
 * INSPECTAIR - SENSOR HISTORY (24h Speicherung)
 * ═══════════════════════════════════════════════════════════════════════════
 */

#include "sensor_history.h"
#include <time.h>

// Globale Instanz
SensorHistory sensorHistory;

bool SensorHistory::begin() {
    // Speicher allozieren (nur einmal!)
    if (history == nullptr) {
        history = new HistoryEntry[HISTORY_ENTRIES];
        if (history == nullptr) {
            Serial.println("[HISTORY] FEHLER: Konnte Speicher nicht allozieren!");
            return false;
        }
        // Speicher initialisieren
        memset(history, 0, sizeof(HistoryEntry) * HISTORY_ENTRIES);
    }
    
    head = 0;
    count = 0;
    
    // Akkumulatoren zurücksetzen
    tempSum = 0;
    humSum = 0;
    co2Sum = 0;
    vocSum = 0;
    pm25Sum = 0;
    sampleCount = 0;
    
    // Timing initialisieren
    lastSave = millis();
    lastPersist = millis();
    
    // Versuche gespeicherte Daten zu laden
    loadFromFlash();
    
    initialized = true;
    
    Serial.println("[HISTORY] Sensor-Historie initialisiert");
    Serial.printf("          Speicher: %d Einträge à %d Bytes = %d KB\n",
                  HISTORY_ENTRIES, sizeof(HistoryEntry),
                  (HISTORY_ENTRIES * sizeof(HistoryEntry)) / 1024);
    Serial.printf("          Geladene Einträge: %d\n", count);
    
    return true;
}

void SensorHistory::end() {
    if (history != nullptr) {
        // Vor Beenden noch speichern
        saveToFlash();
        delete[] history;
        history = nullptr;
    }
    initialized = false;
}

void SensorHistory::addMeasurement(float temp, float hum, int32_t co2, int32_t voc, int32_t pm25) {
    if (!initialized) return;
    
    // Werte akkumulieren für Minutenmittelwert
    tempSum += temp;
    humSum += hum;
    co2Sum += co2;
    vocSum += voc;
    pm25Sum += pm25;
    sampleCount++;
}

void SensorHistory::update() {
    if (!initialized) return;
    
    unsigned long now = millis();
    
    // Alle 60 Sekunden Minutenwert speichern
    if (now - lastSave >= HISTORY_SAVE_INTERVAL && sampleCount > 0) {
        lastSave = now;
        
        // Mittelwert berechnen
        HistoryEntry entry;
        entry.timestamp = now / 1000;  // Sekunden seit Boot (oder Unix-Time wenn verfügbar)
        entry.temp_x10 = (int16_t)((tempSum / sampleCount) * 10);
        entry.humidity = (uint8_t)(humSum / sampleCount);
        entry.co2 = (uint16_t)(co2Sum / sampleCount);
        entry.voc = (uint16_t)(vocSum / sampleCount);
        entry.pm25 = (uint16_t)(pm25Sum / sampleCount);
        entry.reserved = 0;
        
        // Unix-Timestamp verwenden wenn verfügbar
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            time_t t = mktime(&timeinfo);
            entry.timestamp = (uint32_t)t;
        }
        
        // In Ringpuffer speichern
        history[head] = entry;
        head = (head + 1) % HISTORY_ENTRIES;
        if (count < HISTORY_ENTRIES) {
            count++;
        }
        
        // Akkumulatoren zurücksetzen
        tempSum = 0;
        humSum = 0;
        co2Sum = 0;
        vocSum = 0;
        pm25Sum = 0;
        sampleCount = 0;
        
        // Debug (optional)
        // Serial.printf("[HISTORY] Minutenwert gespeichert: T=%.1f H=%d CO2=%d (%d Einträge)\n",
        //               entry.temp_x10 / 10.0f, entry.humidity, entry.co2, count);
    }
    
    // Alle 5 Minuten in Flash persistieren
    if (now - lastPersist >= HISTORY_PERSIST_INTERVAL) {
        lastPersist = now;
        saveToFlash();
    }
}

void SensorHistory::saveToFlash() {
    if (!initialized || count == 0) return;
    
    prefs.begin("sensorhist", false);
    
    // Metadata speichern
    prefs.putInt("head", head);
    prefs.putInt("count", count);
    
    // Nur die neuesten 60 Einträge (1h) im Flash speichern
    // (Vollständige 24h würde zu viel Flash-Wear verursachen)
    int saveCount = min(count, 60);
    int startIdx = (head - saveCount + HISTORY_ENTRIES) % HISTORY_ENTRIES;
    
    // Als Blob speichern
    HistoryEntry tempBuffer[60];
    for (int i = 0; i < saveCount; i++) {
        int idx = (startIdx + i) % HISTORY_ENTRIES;
        tempBuffer[i] = history[idx];
    }
    
    prefs.putBytes("data", tempBuffer, saveCount * sizeof(HistoryEntry));
    prefs.putInt("saved", saveCount);
    
    prefs.end();
    
    // Serial.printf("[HISTORY] %d Einträge in Flash gespeichert\n", saveCount);
}

void SensorHistory::loadFromFlash() {
    prefs.begin("sensorhist", true);  // read-only
    
    int savedCount = prefs.getInt("saved", 0);
    if (savedCount > 0 && savedCount <= 60) {
        HistoryEntry tempBuffer[60];
        size_t readBytes = prefs.getBytes("data", tempBuffer, savedCount * sizeof(HistoryEntry));
        
        if (readBytes == savedCount * sizeof(HistoryEntry)) {
            // Daten in Ringpuffer laden
            for (int i = 0; i < savedCount; i++) {
                history[i] = tempBuffer[i];
            }
            head = savedCount % HISTORY_ENTRIES;
            count = savedCount;
            
            Serial.printf("[HISTORY] %d Einträge aus Flash geladen\n", savedCount);
        }
    }
    
    prefs.end();
}

bool SensorHistory::getEntry(int index, HistoryEntry& entry) const {
    if (!initialized || index < 0 || index >= count) {
        return false;
    }
    
    // Index 0 = ältester Eintrag
    int actualIdx = (head - count + index + HISTORY_ENTRIES) % HISTORY_ENTRIES;
    entry = history[actualIdx];
    return true;
}

bool SensorHistory::getLatestEntry(HistoryEntry& entry) const {
    if (!initialized || count == 0) {
        return false;
    }
    
    int latestIdx = (head - 1 + HISTORY_ENTRIES) % HISTORY_ENTRIES;
    entry = history[latestIdx];
    return true;
}

bool SensorHistory::getAverage(int minutes, float& temp, float& hum,
                               int32_t& co2, int32_t& voc, int32_t& pm25) const {
    if (!initialized || count == 0 || minutes <= 0) {
        return false;
    }
    
    int entriesToUse = min(minutes, count);
    
    float tempSum = 0;
    float humSum = 0;
    int32_t co2Sum = 0;
    int32_t vocSum = 0;
    int32_t pm25Sum = 0;
    
    for (int i = count - entriesToUse; i < count; i++) {
        HistoryEntry e;
        if (getEntry(i, e)) {
            tempSum += e.temp_x10 / 10.0f;
            humSum += e.humidity;
            co2Sum += e.co2;
            vocSum += e.voc;
            pm25Sum += e.pm25;
        }
    }
    
    temp = tempSum / entriesToUse;
    hum = humSum / entriesToUse;
    co2 = co2Sum / entriesToUse;
    voc = vocSum / entriesToUse;
    pm25 = pm25Sum / entriesToUse;
    
    return true;
}

bool SensorHistory::getMinMax(int minutes,
                              float& tempMin, float& tempMax,
                              float& humMin, float& humMax,
                              int32_t& co2Min, int32_t& co2Max) const {
    if (!initialized || count == 0 || minutes <= 0) {
        return false;
    }
    
    int entriesToUse = min(minutes, count);
    
    // Initialisieren mit erstem Wert
    HistoryEntry first;
    if (!getEntry(count - entriesToUse, first)) {
        return false;
    }
    
    tempMin = tempMax = first.temp_x10 / 10.0f;
    humMin = humMax = first.humidity;
    co2Min = co2Max = first.co2;
    
    // Rest durchgehen
    for (int i = count - entriesToUse + 1; i < count; i++) {
        HistoryEntry e;
        if (getEntry(i, e)) {
            float t = e.temp_x10 / 10.0f;
            if (t < tempMin) tempMin = t;
            if (t > tempMax) tempMax = t;
            
            if (e.humidity < humMin) humMin = e.humidity;
            if (e.humidity > humMax) humMax = e.humidity;
            
            if (e.co2 < co2Min) co2Min = e.co2;
            if (e.co2 > co2Max) co2Max = e.co2;
        }
    }
    
    return true;
}

void SensorHistory::clear() {
    if (!initialized) return;
    
    head = 0;
    count = 0;
    memset(history, 0, sizeof(HistoryEntry) * HISTORY_ENTRIES);
    
    // Flash löschen
    prefs.begin("sensorhist", false);
    prefs.clear();
    prefs.end();
    
    Serial.println("[HISTORY] Alle Daten gelöscht");
}

void SensorHistory::printStatus() {
    if (!initialized) {
        Serial.println("[HISTORY] Nicht initialisiert!");
        return;
    }
    
    Serial.println("\n╔═══════════════════════════════════════════════════════════╗");
    Serial.println("║              SENSOR HISTORY STATUS                        ║");
    Serial.println("╠═══════════════════════════════════════════════════════════╣");
    Serial.printf("║ Gespeicherte Einträge: %d / %d (%.1f%%)\n", 
                  count, HISTORY_ENTRIES, (count * 100.0f) / HISTORY_ENTRIES);
    Serial.printf("║ Speicherverbrauch: %d KB\n",
                  (count * sizeof(HistoryEntry)) / 1024);
    
    if (count > 0) {
        HistoryEntry latest;
        if (getLatestEntry(latest)) {
            Serial.printf("║ Letzter Eintrag: T=%.1f°C H=%d%% CO2=%dppm\n",
                          latest.temp_x10 / 10.0f, latest.humidity, latest.co2);
        }
        
        // Durchschnitt letzte Stunde
        float avgT, avgH;
        int32_t avgCO2, avgVOC, avgPM;
        if (getAverage(60, avgT, avgH, avgCO2, avgVOC, avgPM)) {
            Serial.printf("║ Durchschnitt (1h): T=%.1f°C H=%.0f%% CO2=%ldppm\n",
                          avgT, avgH, avgCO2);
        }
    }
    
    Serial.println("╚═══════════════════════════════════════════════════════════╝\n");
}

void SensorHistory::printLastHours(int hours) {
    if (!initialized || count == 0) {
        Serial.println("[HISTORY] Keine Daten verfügbar");
        return;
    }
    
    int minutes = hours * 60;
    int entriesToShow = min(minutes, count);
    
    Serial.printf("\n[HISTORY] Letzte %d Stunden (%d Einträge):\n", hours, entriesToShow);
    Serial.println("Zeit          Temp   Hum   CO2    VOC   PM2.5");
    Serial.println("─────────────────────────────────────────────");
    
    // Nur jeden 10. Eintrag zeigen (alle 10 Minuten)
    for (int i = count - entriesToShow; i < count; i += 10) {
        HistoryEntry e;
        if (getEntry(i, e)) {
            // Timestamp formatieren wenn verfügbar
            char timeStr[20] = "??:??";
            if (e.timestamp > 1600000000) {  // Plausible Unix-Zeit
                time_t t = e.timestamp;
                struct tm* tm = localtime(&t);
                if (tm) {
                    strftime(timeStr, sizeof(timeStr), "%H:%M", tm);
                }
            }
            
            Serial.printf("%s        %5.1f  %3d%%  %4d   %3d   %3d\n",
                          timeStr,
                          e.temp_x10 / 10.0f,
                          e.humidity,
                          e.co2,
                          e.voc,
                          e.pm25);
        }
    }
    Serial.println();
}
