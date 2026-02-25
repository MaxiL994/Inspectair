# INSPECTAIR — Testplan & Ausführungsschema

> **Version:** 1.0 | **Datum:** Februar 2026 | **Projekt:** InspectAir

---

## 1. Überblick

Dieses Dokument beschreibt **wie** die 17 Testfälle der Testspezifikation v2.0 systematisch
abgetestet, aufgezeichnet und dokumentiert werden.

### Benötigte Dateien

| Datei | Zweck |
|-------|-------|
| `docs/testing/TESTPLAN.md` | Dieses Dokument — Reihenfolge & Vorgehen |
| `docs/testing/TESTPROTOKOLL.md` | Ausfüllbares Protokoll (pro Testdurchlauf kopieren) |
| `docs/testing/capture_serial.ps1` | PowerShell-Script: Serial-Log automatisch mitschneiden |
| `src/utils/endurance_test.h/.cpp` | Automatischer Health-Monitor (bereits integriert) |
| `/api/health` | JSON-Endpoint für automatisierte Auswertung |

---

## 2. Testreihenfolge & Abhängigkeiten

Die Tests werden in **4 Phasen** durchgeführt. Jede Phase baut auf der vorherigen auf.

```
Phase 1: Hardware-Verifikation      ┐
  TC-07  BS170 MOSFET                │ Voraussetzung für alles
  TC-08  Stützkondensatoren          │
                                     ┘
Phase 2: Sensor-Integration          ┐
  TC-01–06  (bereits bestanden ✓)    │ Einzeltests schon erledigt
  TC-09  Alle Sensoren gleichzeitig  │
                                     ┘
Phase 3: UI & Interaktion            ┐
  TC-10  Screen-Darstellung          │
  TC-11  Screen-spezifische Features │ Setzt Phase 2 voraus
  TC-12  Button-Wechsel              │
  TC-13  Messwerte-Konsistenz        │
                                     ┘
Phase 4: Power & System              ┐
  TC-14  Display-Dimming             │
  TC-15  Display-Aufwecken           │ Setzt Phase 3 voraus
  TC-16  ESP32 Light Sleep           │
  TC-17  Komplett-Szenario           │
                                     ┘
Bonus:   24h-Dauertest               ┐
  Automatisch via EnduranceTest      │ Nach Phase 4
  Serial-Log + /api/health           │
                                     ┘
```

---

## 3. Vorbereitung (einmalig)

### 3.1 Benötigte Werkzeuge

| Werkzeug | Zweck | Vorhanden? |
|----------|-------|:----------:|
| USB-Kabel (Daten) | Firmware + Serial-Monitor | ☐ |
| Stoppuhr / Handy-Timer | Reaktionszeiten messen | ☐ |
| Multimeter | Spannungsmessung 5V-Rail (TC-08) | ☐ |
| Kamera / Handy | Screenshots der Screens | ☐ |
| Laptop mit PlatformIO | Serial-Log erfassen | ☐ |
| Zweites Gerät im WLAN | Webapp `/api/health` abrufen | ☐ |

### 3.2 Firmware vorbereiten

```bash
# Build (nicht flashen — wird selbst gemacht)
pio run

# Serial-Log mitschneiden starten (siehe capture_serial.ps1)
.\docs\testing\capture_serial.ps1 -ComPort COM5 -BaudRate 115200
```

### 3.3 Log-Verzeichnis

Alle Testergebnisse werden im Verzeichnis `docs/testing/runs/` abgelegt:

```
docs/testing/runs/
  2026-02-23_run1/
    TESTPROTOKOLL.md          ← Ausgefülltes Protokoll
    serial_log.txt            ← Kompletter Serial-Mitschnitt
    health_start.json         ← /api/health bei Testbeginn
    health_end.json           ← /api/health bei Testende
    photos/                   ← Screenshots der Screens
      tc10_tree.jpg
      tc10_overview.jpg
      ...
```

---

## 4. Ablauf pro Phase

### Phase 1: Hardware-Verifikation (~30 Min)

| Schritt | Aktion | Aufzeichnung |
|---------|--------|-------------|
| 1 | Serial-Capture starten | `serial_log.txt` läuft |
| 2 | System einschalten | Init-Meldungen prüfen |
| 3 | **TC-07** durchführen (MOSFET PWM) | PASS/FAIL + Beobachtungen notieren |
| 4 | **TC-08** durchführen (Kondensatoren) | Multimeter-Werte notieren |
| 5 | Phase-1-Ergebnisse ins Protokoll eintragen | |

### Phase 2: Sensor-Integration (~15 Min)

| Schritt | Aktion | Aufzeichnung |
|---------|--------|-------------|
| 1 | TC-01 bis TC-06 als "PASS (Jan 2026)" bestätigen | Vorherige Ergebnisse übernehmen |
| 2 | **TC-09** durchführen | 5 Min Sensor-Log laufen lassen |
| 3 | Serial-Log auf Fehlermeldungen prüfen | `[E]`-Zeilen zählen |
| 4 | `/api/health` abrufen → `health_phase2.json` speichern | |

### Phase 3: UI & Interaktion (~45 Min)

| Schritt | Aktion | Aufzeichnung |
|---------|--------|-------------|
| 1 | **TC-10**: Jeden Screen fotografieren | 5 Fotos in `photos/` |
| 2 | **TC-11**: Screen-Features einzeln prüfen | Notizen pro Screen |
| 3 | **TC-12**: Button 20x schnell drücken | Serial-Log auf Crash prüfen |
| 4 | **TC-13**: Werte auf allen Screens vergleichen | Tabelle ausfüllen |

### Phase 4: Power & System (~60 Min)

| Schritt | Aktion | Aufzeichnung |
|---------|--------|-------------|
| 1 | **TC-14**: Dimming-Timeout beobachten | Stoppuhr: tatsächliche Zeit |
| 2 | **TC-15**: Aufweck-Reaktionszeit messen | Stoppuhr: ms-Schätzung |
| 3 | **TC-16**: Light Sleep Stromaufnahme | Multimeter-Wert |
| 4 | **TC-17**: 30-Min-Komplett-Szenario | Serial-Log + Health-Report |
| 5 | `/api/health` → `health_end.json` speichern | |

### Bonus: 24h-Dauertest

| Schritt | Aktion | Aufzeichnung |
|---------|--------|-------------|
| 1 | System starten, Serial-Capture mit Timestamp | `serial_24h.txt` |
| 2 | 24h laufen lassen (ununterbrochen) | EnduranceTest druckt alle 5 Min |
| 3 | Stündlich `/api/health` automatisch abrufen (optional) | `health_hourly/` |
| 4 | Nach 24h: Finalen Health-Report sichern | `health_24h_final.json` |
| 5 | Auswertung: Heap-Trend, Fehlerrate, Anomalien | Ins Protokoll eintragen |

**Automatische Bewertung durch EnduranceTest:**
- **PERFEKT**: Heap-Leak < 10 KB/h, < 5 Watchdog-Warnungen, Sensor-Fehlerrate < 10%
- **GUT**: Heap-Leak < 50 KB/h, < 20 Watchdog-Warnungen, Fehlerrate < 25%
- **AKZEPTABEL**: Heap-Leak < 100 KB/h, < 50 Watchdog-Warnungen, Fehlerrate < 50%
- **KRITISCH**: Darüber hinaus

---

## 5. Bewertungskriterien

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
| 17/17 PASS | **BESTANDEN** |
| ≥ 14 PASS, kein FAIL bei Muss-Requirements | **BESTANDEN (bedingt)** |
| Jedes FAIL bei Muss-Requirement | **NICHT BESTANDEN** |

### Muss-Requirements (Blocker bei FAIL)

TC-01–TC-06 (Sensoren), TC-09 (Integration), TC-10 (Screen-Darstellung), TC-12 (Button), TC-17 (System)

---

## 6. Traceability-Matrix

| Requirement | Beschreibung | Testfall(e) | Priorität |
|-------------|-------------|-------------|-----------|
| REQ-F-001 | CO₂-Messung | TC-04, TC-09 | Muss |
| REQ-F-002 | Feinstaub-Messung | TC-05, TC-09 | Muss |
| REQ-F-003 | VOC-Messung | TC-03, TC-09 | Muss |
| REQ-F-004 | Temperatur-Messung | TC-02, TC-09 | Muss |
| REQ-F-005 | Luftfeuchte-Messung | TC-02, TC-09 | Muss |
| REQ-F-006 | Präsenzerkennung | TC-06, TC-15, TC-17 | Muss |
| REQ-F-007 | Display-Anzeige | TC-01, TC-10, TC-11 | Muss |
| REQ-F-008 | Farbcodierung | TC-10, TC-11, TC-13 | Muss |
| REQ-F-009 | Screen-Wechsel | TC-12 | Muss |
| REQ-F-010 | Display-Dimming | TC-14 | Soll |
| REQ-F-011 | Display-Aufwecken | TC-15 | Soll |
| REQ-F-012 | Light Sleep | TC-16 | Kann |
| REQ-NF-001 | Update-Rate (2s) | TC-09, TC-17 | Muss |
| REQ-NF-002 | Reaktionszeit UI | TC-12, TC-17 | Muss |
| REQ-NF-003 | Dauerbetrieb 24h | TC-17, 24h-Test | Muss |
| REQ-HW-009 | Stützkondensatoren | TC-08 | Muss |
