# INSPECTAIR - Modularisierte Projektstruktur

## Übersicht

```
inspectair/
├── platformio.ini              # PlatformIO Konfiguration
├── include/                    # Header-Dateien (zentrale Defs)
│   ├── pins.h                 # Pin-Belegung
│   ├── colors.h               # Farben & Farbklassifizierung
│   ├── display_config.h       # Display ST7796S Konfiguration
│   └── sensor_types.h         # Sensor-Datenstrukturen
├── src/
│   ├── main.cpp               # Zentrale Loop & Setup
│   ├── display/
│   │   ├── ui_manager.h       # UI-Funktionen (Header)
│   │   └── ui_manager.c       # UI-Implementierung (Display-Rendering)
│   ├── sensors/
│   │   ├── aht_sgp.h/c        # AHT20 (Temp/Hum) + SGP40 (VOC)
│   │   ├── mhz19c.h/c         # MH-Z19C CO2-Sensor
│   │   ├── pms5003.h/c        # PMS5003 Feinstaub-Sensor
│   │   └── ld2410c.h/c        # LD2410C Radar-Sensor
│   └── utils/                 # (Reserviert für Hilfsfunktionen)
├── lib/
│   ├── TFT_eSPI/              # Display-Treiber (bei Bedarf)
│   └── ...externe Libs...
└── docs/backup/               # Backup des Codes (Zeitstempel)
```

## Modulare Architektur

### 1. **Header-Layer (include/)**
- **pins.h**: Zentrale Pin-Definitionen (kein Magic Numbers!)
- **colors.h**: RGB565 Farben + Klassifizierungsfunktionen
- **display_config.h**: LovyanGFX LGFX-Klasse
- **sensor_types.h**: Strukturen für Sensordaten

### 2. **Sensor-Treiber (src/sensors/)**
Jeder Sensor hat ein eigenes Modul mit `.h` und `.c`:

```c
// Beispiel: mhz19c.h
bool sensors_mhz19_init(void);        // Initialisierung
bool sensors_mhz19_read(MHZ19C_Data* data);  // Daten auslesen
```

**Vorteile:**
- Austauschbar: Sensor-Logik isoliert
- Wartbar: Änderungen beeinflussen nicht andere Module
- Testbar: Jedes Modul für sich testbar
- Reusable: Treiber können in anderen Projekten verwendet werden

### 3. **Display-Manager (src/display/)**
- `ui_init()`: Initialisierung & Startbildschirm
- `ui_drawSensorBox()`: Zeichnet eine Box mit Wert
- `ui_updateDisplay()`: Updated alle 6 Boxen mit aktuellen Werten
- `ui_showError()`: Fehlermeldungen

### 4. **main.cpp - Zentrale Schleife**

```cpp
void setup() {
  ui_init();
  sensors_i2c_init();
  sensors_pms_init();
  sensors_mhz19_init();
  sensors_radar_init();
}

void loop() {
  // Kontinuierlich: PMS-Daten lesen
  sensors_pms_read(&readings.pms);
  
  // Alle 2 Sekunden:
  if (millis() - lastUpdate > 2000) {
    sensors_aht20_read(&readings.aht);
    sensors_mhz19_read(&readings.mhz);
    sensors_sgp40_read(...);
    sensors_radar_read(&readings.radar);
    
    ui_updateDisplay(readings);  // Display aktualisieren
  }
}
```

## Datenfluss

```
Sensoren (Hardware)
    ↓
Sensor-Treiber (sensors/)
    ↓
SensorReadings (sensor_types.h)
    ↓
UI-Manager (display/ui_manager.c)
    ↓
Display (Hardware)
```

## Kompilation

PlatformIO sucht automatisch alle `.c` und `.h` Dateien:
- `include/` → Automatisch im Include-Pfad
- `src/` → Automatisch kompiliert
- `src/sensors/` → Automatisch kompiliert
- `src/display/` → Automatisch kompiliert

## Wie man ein Sensor-Modul hinzufügt

1. Datei erstellen: `src/sensors/neuer_sensor.h` & `.h`
2. Initialisierungsfunktion: `bool sensors_newsensor_init()`
3. Read-Funktion: `bool sensors_newsensor_read(NewSensorData* data)`
4. Datentyp in `sensor_types.h` definieren
5. In `main.cpp` aufrufen

Beispiel:
```c
// src/sensors/newsensor.h
typedef struct {
  uint16_t value;
} NewSensorData;

bool sensors_newsensor_init(void);
bool sensors_newsensor_read(NewSensorData* data);
```

## Debugging

Alle Module geben über `Serial.println()` aus:
```
[INIT] Display...
[INIT] I2C-Sensoren (AHT20, SGP40)...
  AHT20: OK
  SGP40: OK
[INIT] UART-Sensoren...
  MH-Z19C: OK
  PMS5003: OK
  LD2410C: OK
```

Mit `Serial.printf()` im Loop:
```
[1234] T:22.5 H:45.0 CO2:450 VOC:50 PM2.5:8 PM10:12 Radar:1
```

## Erweiterung: Button-Handling

Im `main.cpp` könnten wir Button-Interrupt hinzufügen:
```c
void handleButton() {
  // Umschalte zwischen verschiedenen Display-Seiten
}

attachInterrupt(PIN_BUTTON, handleButton, FALLING);
```

---

**Version:** 2.0 (Modularisiert)  
**Stand:** Januar 2026
