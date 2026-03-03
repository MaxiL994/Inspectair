# INSPECTAIR — Testplan & Ausführungsschema v3.0

> **Version:** 3.0 | **Datum:** März 2026 | **Projekt:** InspectAir

---

## 1. Überblick

Dieses Dokument beschreibt **wie** die **28 Testfälle** der Testspezifikation v3.0 systematisch
abgetestet, aufgezeichnet und dokumentiert werden.

### Benötigte Dateien

| Datei | Zweck |
|-------|-------|
| `docs/testing/TESTPLAN_V3.md` | Dieses Dokument — Reihenfolge & Vorgehen |
| `docs/testing/TESTPROTOKOLL_V3.md` | Ausfüllbares Protokoll (pro Testdurchlauf kopieren) |
| `docs/testing/capture_serial.ps1` | PowerShell-Script: Serial-Log automatisch mitschneiden |
| `src/utils/endurance_test.h/.cpp` | Automatischer Health-Monitor (bereits integriert) |
| `/api/health` | JSON-Endpoint für automatisierte Auswertung |

---

## 2. Testphasen & Abhängigkeiten (6 Phasen)

```
Phase 1: Einzeltests Sensoren        ┐
  TC-01  Display ST7796S             │
  TC-02  AHT20 Temp/Feuchte          │ Basis-Funktionalität
  TC-03  SGP40 VOC                   │
  TC-04  MH-Z19C CO₂                 │
  TC-05  PMS5003 Feinstaub           │
  TC-06  LD2410C Radar               │
                                     ┘
Phase 2: Hardware-Tests              ┐
  TC-07  Backlight PWM (GPIO 3)      │
  TC-08  Stützkondensatoren          │ Voraussetzung für Dauerbetrieb
  TC-09  Alle Sensoren parallel      │
                                     ┘
Phase 3: UI-Tests (5 Themes)         ┐
  TC-10  Tree Screen                 │
  TC-11  Overview Screen             │
  TC-12  Detail Screen               │ Setzt Phase 1+2 voraus
  TC-13  Analog Screen               │
  TC-14  Bubble Screen               │
  TC-15  Button-Wechsel              │
  TC-16  Werte-Konsistenz            │
                                     ┘
Phase 4: WebApp & 24h-Diagramm       ┐
  TC-17  WebServer Init              │
  TC-18  API /api/sensors            │
  TC-19  API /api/history            │
  TC-20  Datensammlung 5min          │ Setzt WiFi-Verbindung voraus
  TC-21  Theme via API               │
  TC-22  Chart.js Diagramm           │
  TC-23  Responsive Design           │
  TC-24  Ringpuffer 288 Punkte       │
                                     ┘
Phase 5: Power Management            ┐
  TC-25  Display-Dimming             │
  TC-26  Aufwecken per Radar         │ Setzt Phase 3 voraus
  TC-27  Light Sleep                 │
                                     ┘
Phase 6: Systemtest                  ┐
  TC-28  24h Komplett-Szenario       │ Finale Validierung
                                     ┘
```

---

## 3. Vorbereitung (einmalig)

### 3.1 Benötigte Werkzeuge

| Werkzeug | Zweck | Vorhanden? |
|----------|-------|:----------:|
| USB-Kabel (Daten) | Firmware + Serial-Monitor | ☐ |
| Stoppuhr / Handy-Timer | Reaktionszeiten messen | ☐ |
| Multimeter | Spannungsmessung 5V-Rail (TC-08), Stromaufnahme (TC-27) | ☐ |
| Kamera / Handy | Screenshots der 5 Screens | ☐ |
| Laptop mit PlatformIO | Serial-Log erfassen | ☐ |
| Zweites Gerät im WLAN | WebApp + API testen | ☐ |
| Browser DevTools | Responsive Design testen | ☐ |

### 3.2 Firmware vorbereiten

```powershell
# Build
pio run

# Serial-Log mitschneiden starten
powershell -ExecutionPolicy Bypass -File ".\docs\testing\capture_serial.ps1" -ComPort COM8 -Duration 480
```

### 3.3 Log-Verzeichnis

```
docs/testing/runs/
  2026-03-01_run1/
    TESTPROTOKOLL_V3.md       ← Ausgefülltes Protokoll
    serial_log.txt            ← Kompletter Serial-Mitschnitt
    health_start.json         ← /api/health bei Testbeginn
    health_end.json           ← /api/health bei Testende
    photos/                   ← Screenshots der 5 Themes
      tc10_tree.jpg
      tc11_overview.jpg
      tc12_detail.jpg
      tc13_analog.jpg
      tc14_bubble.jpg
```

---

## 4. Ablauf pro Phase

### Phase 1: Einzeltests Sensoren (~30 Min)

| Schritt | Aktion | Aufzeichnung |
|---------|--------|-------------|
| 1 | Serial-Capture starten | `serial_log.txt` läuft |
| 2 | System einschalten | Init-Meldungen prüfen |
| 3 | **TC-01** Display: Farbtest | PASS/FAIL notieren |
| 4 | **TC-02** AHT20: Temp/Feuchte prüfen | Werte notieren |
| 5 | **TC-03** SGP40: VOC-Index prüfen | Werte notieren |
| 6 | **TC-04** MH-Z19C: CO₂ bei Frischluft | Werte notieren |
| 7 | **TC-05** PMS5003: PM2.5 prüfen | Werte notieren |
| 8 | **TC-06** LD2410C: Präsenz testen | Annäherung/Entfernung |

### Phase 2: Hardware-Tests (~30 Min)

| Schritt | Aktion | Aufzeichnung |
|---------|--------|-------------|
| 1 | **TC-07** PWM-Dimming: 0%, 12%, 100% testen | Helligkeit beobachten |
| 2 | **TC-08** Multimeter: 5V-Rail messen | Spannung notieren |
| 3 | **TC-09** 5 Min alle Sensoren parallel | `[E]`-Meldungen zählen |

### Phase 3: UI-Tests 5 Themes (~45 Min)

| Schritt | Aktion | Aufzeichnung |
|---------|--------|-------------|
| 1 | **TC-10** Tree Screen fotografieren | Animation prüfen |
| 2 | **TC-11** Overview Screen fotografieren | AQI + 2 Kacheln |
| 3 | **TC-12** Detail Screen fotografieren | AQI + 4 Kacheln |
| 4 | **TC-13** Analog Screen fotografieren | Zeiger prüfen |
| 5 | **TC-14** Bubble Screen fotografieren | Dynamik prüfen |
| 6 | **TC-15** Button-Wechsel: Zyklus + Stresstest | 20x schnell drücken |
| 7 | **TC-16** Werte auf allen Screens vergleichen | Tabelle ausfüllen |

### Phase 4: WebApp & API (~45 Min)

| Schritt | Aktion | Aufzeichnung |
|---------|--------|-------------|
| 1 | **TC-17** Browser: http://IP öffnen | Erreichbarkeit prüfen |
| 2 | **TC-18** GET /api/sensors | JSON-Response kopieren |
| 3 | **TC-19** GET /api/history | Datenpunkte zählen |
| 4 | **TC-20** 10 Min warten, /api/history erneut | 2 neue Punkte? |
| 5 | **TC-21** POST /api/theme | Theme-Wechsel auf Display |
| 6 | **TC-22** Chart.js Diagramm prüfen | Linien sichtbar? |
| 7 | **TC-23** Responsive: Desktop, Tablet, Mobile | DevTools nutzen |
| 8 | **TC-24** Nach 24h: max. 288 Punkte? | (Teil von TC-28) |

### Phase 5: Power Management (~30 Min)

| Schritt | Aktion | Aufzeichnung |
|---------|--------|-------------|
| 1 | **TC-25** Dimming: 60s warten ohne Bewegung | Stoppuhr nutzen |
| 2 | **TC-26** Aufwecken: Annäherung, Reaktionszeit | < 500ms? |
| 3 | **TC-27** Light Sleep: Stromaufnahme messen | Multimeter: ~0.8mA |

### Phase 6: Systemtest (~24h)

| Schritt | Aktion | Aufzeichnung |
|---------|--------|-------------|
| 1 | System starten, Serial-Capture mit Timestamp | `serial_24h.txt` |
| 2 | 24h laufen lassen (ununterbrochen) | EnduranceTest aktiv |
| 3 | Nach 24h: /api/health abrufen | `health_24h_final.json` |
| 4 | Auswertung: Heap-Trend, Fehlerrate, Anomalien | Ins Protokoll |

---

## 5. Zeitplanung

| Phase | Dauer | Abhängigkeit |
|-------|-------|--------------|
| Phase 1: Einzeltests Sensoren | ~30 Min | - |
| Phase 2: Hardware-Tests | ~30 Min | Phase 1 |
| Phase 3: UI-Tests | ~45 Min | Phase 2 |
| Phase 4: WebApp & API | ~45 Min | Phase 3 + WiFi |
| Phase 5: Power Management | ~30 Min | Phase 3 |
| Phase 6: 24h Systemtest | 24h | Phase 1-5 |
| **Gesamt (ohne 24h)** | **~3h** | |
| **Gesamt (mit 24h)** | **~27h** | |

---

## 6. Bewertungskriterien

### Pass/Fail-Regeln

| Ergebnis | Bedeutung | Bedingung |
|----------|-----------|-----------|
| **PASS** ✅ | Test bestanden | Alle Pass-Kriterien erfüllt |
| **PASS (Einschränkung)** ⚠️ | Bestanden mit Anmerkung | Pass-Kriterien erfüllt, aber Beobachtung |
| **FAIL** ❌ | Test nicht bestanden | Mind. 1 Pass-Kriterium verletzt |
| **BLOCKED** 🚫 | Nicht durchführbar | Abhängigkeit nicht erfüllt |
| **SKIPPED** ⏭️ | Übersprungen | Feature nicht implementiert |

### Gesamtbewertung

| Bedingung | Ergebnis |
|-----------|----------|
| 28/28 PASS | **BESTANDEN** |
| ≥ 24 PASS, kein FAIL bei Muss-Requirements | **BESTANDEN (bedingt)** |
| Jedes FAIL bei Muss-Requirement | **NICHT BESTANDEN** |

### Muss-Requirements (Blocker bei FAIL)

- **Sensoren:** TC-01 bis TC-06, TC-09
- **UI:** TC-10, TC-15
- **WebApp:** TC-17, TC-18
- **System:** TC-28

### Soll-Requirements

- **UI:** TC-11 bis TC-14, TC-16
- **WebApp:** TC-19 bis TC-24
- **Power:** TC-25, TC-26

### Kann-Requirements

- **Power:** TC-27 (Light Sleep)

---

## 7. Traceability-Matrix (V3)

| Requirement | Beschreibung | Testfall(e) | Priorität |
|-------------|-------------|-------------|-----------|
| REQ-F-001 | CO₂-Messung | TC-04, TC-09 | Muss |
| REQ-F-002 | Feinstaub-Messung | TC-05, TC-09 | Muss |
| REQ-F-003 | VOC-Messung | TC-03, TC-09 | Muss |
| REQ-F-004 | Temperatur-Messung | TC-02, TC-09 | Muss |
| REQ-F-005 | Luftfeuchte-Messung | TC-02, TC-09 | Muss |
| REQ-F-006 | Präsenzerkennung | TC-06, TC-26 | Muss |
| REQ-F-007 | Display-Anzeige | TC-01, TC-10 bis TC-14 | Muss |
| REQ-F-008 | Farbcodierung | TC-10, TC-16 | Muss |
| REQ-F-009 | Screen-Wechsel | TC-15 | Muss |
| REQ-F-010 | WebApp | TC-17 bis TC-24 | Muss |
| REQ-F-011 | 24h-Diagramm | TC-19, TC-22, TC-24 | Soll |
| REQ-F-012 | Theme via API | TC-21 | Soll |
| REQ-F-013 | Display-Dimming | TC-25 | Soll |
| REQ-F-014 | Display-Aufwecken | TC-26 | Soll |
| REQ-F-015 | Light Sleep | TC-27 | Kann |
| REQ-NF-001 | Update-Rate (2s) | TC-09, TC-28 | Muss |
| REQ-NF-002 | Reaktionszeit UI | TC-15, TC-26 | Muss |
| REQ-NF-003 | Dauerbetrieb 24h | TC-28 | Muss |
| REQ-HW-001 | PWM Backlight GPIO 3 | TC-07 | Muss |
| REQ-HW-002 | Stützkondensatoren | TC-08 | Muss |

---

## 8. Grenzwerte zur Bewertung

| Parameter | Gut (Grün) | Mäßig (Gelb) | Schlecht (Orange) | Gefährlich (Rot) |
|-----------|------------|--------------|-------------------|------------------|
| CO₂ | < 800 ppm | 800–1000 | 1000–1500 | > 1500 ppm |
| PM2.5 | ≤ 5 µg/m³ | 5–15 | 15–25 | > 25 µg/m³ |
| VOC-Index | ≤ 100 | 100–200 | 200–300 | > 300 |

---

## 9. Health-Bewertung (24h-Test)

| Rating | Heap-Leak | Watchdog | Sensor-Fehler |
|--------|-----------|----------|---------------|
| **PERFEKT** | < 10 KB/h | < 5 | < 10% |
| **GUT** | < 50 KB/h | < 20 | < 25% |
| **AKZEPTABEL** | < 100 KB/h | < 50 | < 50% |
| **KRITISCH** | > 100 KB/h | > 50 | > 50% |
