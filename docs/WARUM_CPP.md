# Erklärung: Warum `.cpp` und nicht `.c`?

## 🔴 Das Problem mit `.c` Dateien in Arduino/C++

### Was `.c` bedeutet:
- **C-Dateien** (älter, klassisches C ohne C++-Features)
- Compiler erkennt: "Das ist C-Code, nicht C++!"
- **Problem:** Arduino ist C++!

### Unterschiede:

| Merkmal | `.c` (C) | `.cpp` (C++) |
|--------|---------|------------|
| **Namespaces** | ❌ Nicht unterstützt | ✅ Unterstützt |
| **Klassen** | ❌ Nicht unterstützt | ✅ Unterstützt |
| **Operator Overloading** | ❌ Nein | ✅ Ja |
| **Arduino-Libs** | ⚠️ Mit Problemen | ✅ Vollständig |
| **Name Mangling** | ❌ Keins | ✅ Ja (wichtig!) |

### Unser Code nutzt C++ Features:

```cpp
// ❌ Funktioniert mit .c NICHT korrekt:

#include <Adafruit_AHTX0.h>  // C++ Klasse!
#include <Wire.h>             // Arduino C++ Library

Adafruit_AHTX0 aht;            // ← C++ Objekt
bool sensors_aht20_read(...) { // ← Funktion mit Pointern
    aht.getEvent(...);         // ← C++ Methode
}
```

### Der Mix-Fehler:
```
❌ FALSCH:
main.cpp (C++)  →  aht_sgp.c (C)  ← Compiler-Konflikt!
                                    Name Mangling Problem

✅ RICHTIG:
main.cpp (C++)  →  aht_sgp.cpp (C++)  ← Konsistent!
```

---

## 🟢 Lösung: `.cpp` für alle

### Arduino/ESP32 Convention:
```
main.ino  oder  main.cpp    ✅ Hauptprogramm
aht_sgp.h  +  aht_sgp.cpp   ✅ Module (Header + Implementation)
```

### PlatformIO automatisch:
```
src/
├── main.cpp              ← Automatisch kompiliert
├── display/
│   └── ui_manager.cpp    ← Automatisch kompiliert
└── sensors/
    ├── aht_sgp.cpp       ← Automatisch kompiliert
    ├── mhz19c.cpp        ← Automatisch kompiliert
    ├── pms5003.cpp       ← Automatisch kompiliert
    └── ld2410c.cpp       ← Automatisch kompiliert
```

**Keine Makefile-Anpassungen nötig!** ✅

---

## 📋 Was wurde umgewandelt?

```bash
# Konvertierung durchgeführt:
ui_manager.c    → ui_manager.cpp    ✅
aht_sgp.c       → aht_sgp.cpp       ✅
mhz19c.c        → mhz19c.cpp        ✅
pms5003.c       → pms5003.cpp       ✅
ld2410c.c       → ld2410c.cpp       ✅
```

### Includes bleiben gleich:
```cpp
// Keine Änderung nötig!
#include "src/sensors/aht_sgp.h"    ← Bezieht sich auf .h, nicht .c/.cpp
#include "src/display/ui_manager.h"
```

---

## 🎯 Best Practice

### Arduino-Standard in der Industrie:
- **Adafruit** (professionelle Library) → `.cpp`
- **SparkFun** → `.cpp`
- **Arduino Official** → `.ino` (Beginner) oder `.cpp` (Professionell)
- **ESP-IDF** → `.cpp`

### Unser Projekt:
```
✅ Folgt Arduino-Best-Practices
✅ Konsistent mit professionellen Projekten
✅ Kompatibel mit allen Compiler-Flags
✅ Keine Kompatibilitätsprobleme
```

---

## 🔧 Technischer Hintergrund

### Name Mangling:
```cpp
// C++ macht Namensmangling für Funktionen
_Z18sensors_aht20_readP8AHT20_Data

// C macht das nicht
sensors_aht20_read

// Wenn .c und .cpp gemischt:
// Linker findet Funktion nicht → Fehler!
```

### Lösung:
```cpp
// Externe C-Funktionen in C++ aufrufen:
extern "C" {
    // C-Code hier
}

// Aber: Nicht nötig, wenn alles .cpp ist! ✅
```

---

## ✅ Ergebnis

Nach Konvertierung zu `.cpp`:
- ✅ Konsistent mit Arduino-Standard
- ✅ Keine Compiler-Warnungen
- ✅ PlatformIO findet alles automatisch
- ✅ Professioneller Code-Standard
- ✅ Zukünftig wartbar & erweiterbar

---

**Wichtig:** Die `.c` → `.cpp` Konvertierung ändert **KEINE Code-Logik**, nur die Dateiendung!
