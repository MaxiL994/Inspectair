/**
 * ═══════════════════════════════════════════════════════════════════════════
 * INSPECTAIR - SENSOR HISTORY (24h Speicherung)
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Speichert aggregierte Minutenwerte der letzten 24 Stunden.
 * Nutzt ESP32 Preferences (NVS) für persistente Speicherung.
 *
 * Speicherverbrauch: ~35KB für 24h (1440 Minuten * 24 Bytes pro Eintrag)
 */

#ifndef SENSOR_HISTORY_H
#define SENSOR_HISTORY_H

#include <Arduino.h>
#include <Preferences.h>
#include "sensor_types.h"

// ═══════════════════════════════════════════════════════════════════════════
// KONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════

#define HISTORY_ENTRIES         1440    // 24h * 60 Minuten
#define HISTORY_SAVE_INTERVAL   60000   // Alle 60 Sekunden speichern
#define HISTORY_PERSIST_INTERVAL 300000 // Alle 5 Minuten in Flash schreiben

// ═══════════════════════════════════════════════════════════════════════════
// DATENSTRUKTUR FÜR EINEN HISTORIEN-EINTRAG
// ═══════════════════════════════════════════════════════════════════════════

struct HistoryEntry {
    uint32_t timestamp;     // Unix-Timestamp oder millis()
    int16_t temp_x10;       // Temperatur * 10 (für 0.1° Auflösung)
    uint8_t humidity;       // Luftfeuchtigkeit (0-100%)
    uint16_t co2;           // CO2 in ppm
    uint16_t pm25;          // PM2.5 in µg/m³
    uint16_t voc;           // VOC Index
    uint8_t reserved;       // Padding für Alignment
};

// ═══════════════════════════════════════════════════════════════════════════
// SENSOR HISTORY KLASSE
// ═══════════════════════════════════════════════════════════════════════════

class SensorHistory {
private:
    // Ringpuffer für 24h Historie
    HistoryEntry* history = nullptr;
    int head = 0;
    int count = 0;
    
    // Akkumulatoren für Minutenmittelwert
    float tempSum = 0;
    float humSum = 0;
    int32_t co2Sum = 0;
    int32_t vocSum = 0;
    int32_t pm25Sum = 0;
    int sampleCount = 0;
    
    // Timing
    unsigned long lastSave = 0;
    unsigned long lastPersist = 0;
    
    // Preferences für Flash-Speicherung
    Preferences prefs;
    bool initialized = false;

    // Interner Speicher-Helper
    void saveToFlash();
    void loadFromFlash();

public:
    /**
     * Initialisiert die Historie und lädt gespeicherte Daten
     */
    bool begin();
    
    /**
     * Beendet und gibt Speicher frei
     */
    void end();
    
    /**
     * Fügt einen neuen Messwert hinzu (wird akkumuliert)
     */
    void addMeasurement(float temp, float hum, int32_t co2, int32_t voc, int32_t pm25);
    
    /**
     * Muss regelmäßig aufgerufen werden (in loop)
     * Speichert Minutenwerte und persistiert periodisch
     */
    void update();
    
    /**
     * Gibt die Anzahl der gespeicherten Einträge zurück
     */
    int getEntryCount() const { return count; }
    
    /**
     * Gibt einen Historien-Eintrag zurück
     * @param index 0 = ältester, count-1 = neuester
     */
    bool getEntry(int index, HistoryEntry& entry) const;
    
    /**
     * Gibt den neuesten Eintrag zurück
     */
    bool getLatestEntry(HistoryEntry& entry) const;
    
    /**
     * Berechnet Durchschnitt über die letzten N Minuten
     */
    bool getAverage(int minutes, float& temp, float& hum, 
                    int32_t& co2, int32_t& voc, int32_t& pm25) const;
    
    /**
     * Berechnet Min/Max über die letzten N Minuten
     */
    bool getMinMax(int minutes, 
                   float& tempMin, float& tempMax,
                   float& humMin, float& humMax,
                   int32_t& co2Min, int32_t& co2Max) const;
    
    /**
     * Löscht alle gespeicherten Daten
     */
    void clear();
    
    /**
     * Debug-Ausgabe
     */
    void printStatus();
    void printLastHours(int hours);
};

// Globale Instanz
extern SensorHistory sensorHistory;

#endif // SENSOR_HISTORY_H
