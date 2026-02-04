# INSPECTAIR - Coding Standards & Rules (v2.0)

> **Version:** 2.0  
> **Datum:** Februar 2026  
> **Autor:** Team InspectAir

---

## 1. Allgemeine Regeln

- **Sprache:** C++ (Arduino Framework via PlatformIO)
- **Einrückung:** 4 Spaces (keine Tabs)
- **Zeilenlänge:** max. 120 Zeichen
- **Encoding:** UTF-8
- Alle Dateien enden mit einer Leerzeile

---

## 2. Namenskonventionen

| Element | Konvention | Beispiel |
|---------|------------|----------|
| Variablen | snake_case | `sensor_data` |
| Konstanten | UPPER_SNAKE_CASE | `CO2_THRESHOLD` |
| Funktionen | snake_case | `sensors_init()` |
| Structs/Enums | PascalCase | `SensorData` |
| Dateien | snake_case.cpp/.h | `sensors.cpp` |
| Defines | UPPER_SNAKE_CASE | `#define PIN_SDA 8` |

---

## 3. Dokumentation (Doxygen)

Alle öffentlichen Funktionen und Structs müssen dokumentiert werden:

```cpp
/**
 * @brief Initialisiert alle Sensoren
 * @param config Zeiger auf Konfigurationsstruktur
 * @return true wenn mindestens ein Sensor verfügbar
 */
bool sensors_init(const SensorConfig* config);
```

**Header-Dateien** benötigen einen File-Header:

```cpp
/**
 * @file sensor_types.h
 * @brief Datenstrukturen für alle Sensoren
 * @author Team InspectAir
 * @date Januar 2026
 */
```

---

## 4. Fehlerbehandlung

- Jeder Sensor hat ein eigenes ok-Flag im SensorData Struct
- Sensor-Fehler werden geloggt aber nicht als fatal behandelt
- Display zeigt "--" für nicht verfügbare Werte
- Kritische Fehler (Display-Init) führen zu Halt

---

## 5. Git Workflow

- **Main-Branch:** Nur getesteter, funktionierender Code
- **Feature-Branches:** `feature/sensor-module`, `feature/ui-detailed`
- **Commit-Messages:** Präfix + Beschreibung
  - `feat:` Neue Funktionalität
  - `fix:` Bugfix
  - `docs:` Dokumentation
  - `refactor:` Code-Umstrukturierung

---

## 6. Verbotene Praktiken

| Regel | Grund |
|-------|-------|
| ❌ Keine Magic Numbers | Immer `#define` oder `const` verwenden |
| ❌ Keine globalen Variablen ohne `static` | Datei-Scope begrenzen |
| ❌ Keine `delay()` in loop() | `millis()` oder `yield()` verwenden |
| ❌ Keine `String`-Klasse | `char[]` Buffer verwenden (Heap-Fragmentierung) |
| ❌ Keine dynamische Allokation in loop() | Nur in `setup()`/`begin()` erlaubt |
| ❌ Kein `Serial.print()` in ISRs | Nur Flags setzen |

---

## 7. ESP32/FreeRTOS-spezifische Regeln

- `volatile` für alle Variablen die in ISRs gelesen/geschrieben werden
- `IRAM_ATTR` für Interrupt Service Routines
- Task-Stack-Größe beachten (Standard: 4KB, bei viel lokalem Speicher erhöhen)
- `portENTER_CRITICAL()` / `portEXIT_CRITICAL()` für shared resources zwischen Tasks

**Beispiel ISR:**
```cpp
volatile bool buttonPressed = false;

void IRAM_ATTR buttonISR() {
    buttonPressed = true;  // Nur Flag setzen, keine Serial-Ausgabe!
}
```

---

## 8. Const Correctness

- Pointer-Parameter die nicht geändert werden: `const char* text`
- Methoden die Objekt nicht ändern: `int getValue() const`
- Lokale Variablen die nicht geändert werden: `const int max_retries = 10`

**Beispiel:**
```cpp
void printMessage(const char* msg);           // msg wird nicht geändert
bool parseData(const uint8_t* data, size_t len);  // data wird nur gelesen
```

---

## 9. Defensive Programmierung

- Null-Pointer-Checks vor Dereferenzierung
- Array-Bounds validieren
- Timeout bei blockierenden Operationen (WiFi, Sensor-Reads)
- Sinnvolle Default-Werte bei fehlgeschlagenen Reads

**Beispiel:**
```cpp
bool sensors_read(SensorData* data) {
    if (data == nullptr) return false;  // Null-Check
    
    // Timeout bei Sensor-Read
    unsigned long start = millis();
    while (!sensor.available()) {
        if (millis() - start > SENSOR_TIMEOUT_MS) return false;
        yield();
    }
    // ...
}
```

---

## 10. Speicher-Management

- Statische Allokation bevorzugen
- Dynamische Allokation nur in `setup()` / `begin()` Funktionen
- Buffer-Größen als Konstanten definieren
- Bei Arrays: `sizeof()` statt hardcoded Größen

**Beispiel:**
```cpp
// Gut:
static const size_t BUFFER_SIZE = 64;
char buffer[BUFFER_SIZE];
snprintf(buffer, sizeof(buffer), "Temp: %.1f", temp);

// Schlecht:
char buffer[64];
snprintf(buffer, 64, "Temp: %.1f", temp);  // Magic Number!
```

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| 2.0 | Feb 2026 | ESP32/FreeRTOS-Regeln, Const Correctness, Defensive Programming |
| 1.0 | Jan 2026 | Initiale Version |
