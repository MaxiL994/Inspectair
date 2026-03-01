# INSPECTAIR — Testprotokoll v3.0

> **Durchlauf:** #___ | **Datum:** __________ | **Tester:** __________ | **Firmware-Version:** __________

---

## Testumgebung

| Parameter | Wert |
|-----------|------|
| **ESP32-Board** | ESP32-S3 DevKitC-1 (Waveshare) |
| **Firmware-Commit** | `git rev-parse --short HEAD` → ________ |
| **Branch** | testing/manual-tests |
| **PlatformIO Build** | ☐ OK ☐ Fehler |
| **COM-Port** | COM___ |
| **Serial-Log aktiv** | ☐ Ja ☐ Nein |
| **Raum-Temperatur** | _____ °C |
| **Stützkondensatoren** | ☐ Verbaut ☐ Nicht verbaut |

---

## Übersicht (28 Testfälle)

| Kategorie | Anzahl | Bestanden | Status |
|-----------|:------:|:---------:|:------:|
| Einzeltests Sensoren | 6 | ___ | ☐ |
| Hardware-Tests | 3 | ___ | ☐ |
| UI-Tests (5 Themes) | 7 | ___ | ☐ |
| WebApp & 24h-Diagramm | 8 | ___ | ☐ |
| Power Management | 3 | ___ | ☐ |
| Systemtest | 1 | ___ | ☐ |
| **GESAMT** | **28** | ___ | ☐ |

---

## Phase 1: Einzeltests Sensoren (TC-01 bis TC-06)

### TC-01 — Display ST7796S

| Feld | Eintrag |
|------|---------|
| **Vorbedingung erfüllt?** | ☐ Ja ☐ Nein — SPI verkabelt, 480x320 Display |
| **Schritt 1:** Display initialisiert? | ☐ Ja ☐ Nein |
| **Schritt 2:** Hintergrundfarbe sichtbar? | ☐ Ja ☐ Nein |
| **Schritt 3:** Text lesbar? | ☐ Ja ☐ Nein |
| **Schritt 4:** Farbdarstellung korrekt (rot/grün/blau)? | ☐ Ja ☐ Nein |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-02 — AHT20 Temperatur/Feuchte

| Feld | Eintrag |
|------|---------|
| **Vorbedingung erfüllt?** | ☐ Ja ☐ Nein — I2C 0x38 |
| **Schritt 1:** `[I][I2C] AHT20 OK` im Serial? | ☐ Ja ☐ Nein |
| **Schritt 2:** Temperatur plausibel? | _____ °C (Soll: 15–30) ☐ OK ☐ Ausreißer |
| **Schritt 3:** Feuchte plausibel? | _____ % (Soll: 30–70) ☐ OK ☐ Ausreißer |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-03 — SGP40 VOC

| Feld | Eintrag |
|------|---------|
| **Vorbedingung erfüllt?** | ☐ Ja ☐ Nein — I2C 0x59 |
| **Schritt 1:** `[I][I2C] SGP40 OK` im Serial? | ☐ Ja ☐ Nein |
| **Schritt 2:** SRAW-Wert lesbar? | ☐ Ja ☐ Nein |
| **Schritt 3:** VOC-Index plausibel? | _____ (Soll: < 200 normal) ☐ OK ☐ Ausreißer |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-04 — MH-Z19C CO₂

| Feld | Eintrag |
|------|---------|
| **Vorbedingung erfüllt?** | ☐ Ja ☐ Nein — UART 9600, Stützkondensator |
| **Schritt 1:** `[I][CO2] MH-Z19C OK` im Serial? | ☐ Ja ☐ Nein |
| **Schritt 2:** CO₂-Wert bei Frischluft? | _____ ppm (Soll: 350–500) ☐ OK ☐ Ausreißer |
| **Schritt 3:** Anstieg nach Anhauchen? | ☐ Ja ☐ Nein |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-05 — PMS5003 Feinstaub

| Feld | Eintrag |
|------|---------|
| **Vorbedingung erfüllt?** | ☐ Ja ☐ Nein — UART verbunden |
| **Schritt 1:** `[I][PMS] PMS5003 OK` im Serial? | ☐ Ja ☐ Nein |
| **Schritt 2:** PM2.5-Wert in Ruhe? | _____ µg/m³ (Soll: 0–50 Innen) ☐ OK ☐ Ausreißer |
| **Schritt 3:** Reaktion auf Staubquelle? | ☐ Ja ☐ Nein |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-06 — LD2410C Radar

| Feld | Eintrag |
|------|---------|
| **Vorbedingung erfüllt?** | ☐ Ja ☐ Nein — Level Shifter 5V→3,3V an GPIO 7, Stützkondensator |
| **Schritt 1:** `[I][RAD] LD2410C OK` im Serial? | ☐ Ja ☐ Nein |
| **Schritt 2:** Präsenz erkannt bei Annäherung? | ☐ Ja ☐ Nein |
| **Schritt 3:** Präsenz verloren bei Entfernung > 2m? | ☐ Ja ☐ Nein |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

## Phase 2: Hardware-Tests (TC-07 bis TC-09)

### TC-07 — Backlight PWM (GPIO 3)

| Feld | Eintrag |
|------|---------|
| **Vorbedingung erfüllt?** | ☐ Ja ☐ Nein — PWM direkt über GPIO 3 (44,1 kHz) |
| **Schritt 1:** PWM 0% → Display dunkel? | ☐ Ja ☐ Nein |
| **Schritt 2:** PWM ~12% → gedimmt sichtbar? | ☐ Ja ☐ Nein |
| **Schritt 3:** PWM 100% → volle Helligkeit? | ☐ Ja ☐ Nein |
| **Schritt 4:** Fade-Übergang smooth? | ☐ Ja ☐ Nein |
| **Flackern beobachtet?** | ☐ Nein ☐ Ja (Beschreibung: ___) |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-08 — Stützkondensatoren Stabilität

| Feld | Eintrag |
|------|---------|
| **Vorbedingung erfüllt?** | ☐ Ja ☐ Nein — 220µF 25V an MH-Z19C und LD2410C |
| **Schritt 1:** 5V-Rail Spannung gemessen | _____ V (Soll: 4.8–5.2V) |
| **Schritt 2:** System MIT Kondensatoren → 10 Min stabil? | ☐ Ja ☐ Nein |
| **Schritt 3:** Keine Spannungseinbrüche bei Sensoraktivität? | ☐ Ja ☐ Nein |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-09 — Alle Sensoren parallel

| Feld | Eintrag |
|------|---------|
| **Vorbedingung erfüllt?** | ☐ Ja ☐ Nein — Alle Sensoren verkabelt |
| **Schritt 1:** System gestartet, alle Init [OK]? | ☐ Ja ☐ Nein |
| **Init-Meldungen aus Serial-Log:** | |
| `[I][I2C] AHT20 OK` | ☐ Ja ☐ Nein |
| `[I][I2C] SGP40 OK` | ☐ Ja ☐ Nein |
| `[I][CO2] MH-Z19C OK` | ☐ Ja ☐ Nein |
| `[I][PMS] PMS5003 OK` | ☐ Ja ☐ Nein |
| `[I][RAD] LD2410C OK` | ☐ Ja ☐ Nein |
| **Schritt 2:** 5 Min Sensor-Log ohne Fehler? | ☐ Ja ☐ Nein |
| **Anzahl `[E]`-Meldungen in 5 Min** | _____ (Soll: < 3) |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

## Phase 3: UI-Tests — 5 Themes (TC-10 bis TC-16)

### TC-10 — Tree Screen

| Feld | Eintrag |
|------|---------|
| **Vorbedingung erfüllt?** | ☐ Ja ☐ Nein — Display + Sensoren funktional |
| **Schritt 1:** Baum-Animation sichtbar? | ☐ Ja ☐ Nein |
| **Schritt 2:** Animation flüssig (kein Ruckeln)? | ☐ Ja ☐ Nein |
| **Schritt 3:** Farbe ändert sich bei AQI-Änderung? | ☐ Ja ☐ Nein |
| **Foto gemacht?** | ☐ Ja ☐ Nein |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-11 — Overview Screen

| Feld | Eintrag |
|------|---------|
| **Schritt 1:** Große AQI-Zahl sichtbar? | ☐ Ja ☐ Nein |
| **Schritt 2:** 2 Kacheln lesbar? | ☐ Ja ☐ Nein |
| **Schritt 3:** Werte korrekt formatiert? | ☐ Ja ☐ Nein |
| **Foto gemacht?** | ☐ Ja ☐ Nein |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-12 — Detail Screen

| Feld | Eintrag |
|------|---------|
| **Schritt 1:** Kleine AQI + 4 Kacheln sichtbar? | ☐ Ja ☐ Nein |
| **Schritt 2:** Alle 6 Werte angezeigt? | ☐ Ja ☐ Nein |
| **Schritt 3:** Einheiten korrekt (ppm, µg/m³, %)? | ☐ Ja ☐ Nein |
| **Foto gemacht?** | ☐ Ja ☐ Nein |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-13 — Analog Screen

| Feld | Eintrag |
|------|---------|
| **Schritt 1:** Cockpit-Instrumente sichtbar? | ☐ Ja ☐ Nein |
| **Schritt 2:** Zeiger bewegen sich passend zu Werten? | ☐ Ja ☐ Nein |
| **Schritt 3:** Skala lesbar? | ☐ Ja ☐ Nein |
| **Foto gemacht?** | ☐ Ja ☐ Nein |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-14 — Bubble Screen

| Feld | Eintrag |
|------|---------|
| **Schritt 1:** Dynamische Kreise sichtbar? | ☐ Ja ☐ Nein |
| **Schritt 2:** Kreise ändern Größe dynamisch? | ☐ Ja ☐ Nein |
| **Schritt 3:** Kreise ändern Farbe dynamisch? | ☐ Ja ☐ Nein |
| **Foto gemacht?** | ☐ Ja ☐ Nein |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-15 — Button-Wechsel zyklisch

| Feld | Eintrag |
|------|---------|
| **Vorbedingung erfüllt?** | ☐ Ja ☐ Nein — Button an GPIO 1 |
| **Zyklus-Test:** | |
| Start → Tree | ☐ OK |
| Button → Overview | ☐ OK |
| Button → Detail | ☐ OK |
| Button → Analog | ☐ OK |
| Button → Bubble | ☐ OK |
| Button → Tree (Zyklus) | ☐ OK |
| **Übergangszeit geschätzt** | _____ ms (Soll: < 300ms) |
| **Stresstest: 20x schnell drücken** | ☐ Kein Absturz ☐ Absturz |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-16 — Werte-Konsistenz alle UIs

Werte auf jedem Screen notieren und vergleichen:

| Messwert | Overview | Detail | Analog | Bubble | Konsistent? |
|----------|:--------:|:------:|:------:|:------:|:-----------:|
| CO₂ (ppm) | | | | | ☐ |
| PM2.5 (µg/m³) | | | | | ☐ |
| VOC-Index | | | | | ☐ |
| Temperatur (°C) | | | | | ☐ |
| Feuchte (%) | | | | | ☐ |

| Feld | Eintrag |
|------|---------|
| **Max. Abweichung** | _____ % (Soll: < 1%) |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

## Phase 4: WebApp & 24h-Diagramm (TC-17 bis TC-24)

### TC-17 — WebServer Init

| Feld | Eintrag |
|------|---------|
| **Vorbedingung erfüllt?** | ☐ Ja ☐ Nein — WiFi verbunden |
| **Schritt 1:** ESP32 IP-Adresse im Serial? | ___.___.___.___ |
| **Schritt 2:** http://IP erreichbar im Browser? | ☐ Ja ☐ Nein |
| **Schritt 3:** HTTP Port 80 antwortet? | ☐ Ja ☐ Nein |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-18 — API /api/sensors

| Feld | Eintrag |
|------|---------|
| **Schritt 1:** GET /api/sensors → JSON? | ☐ Ja ☐ Nein |
| **Schritt 2:** Enthält CO2, PM25, VOC, Temp, Hum? | ☐ Ja ☐ Nein |
| **Schritt 3:** Werte matchen Display? | ☐ Ja ☐ Nein |
| **Beispiel-Response:** | `{...}` |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-19 — API /api/history

| Feld | Eintrag |
|------|---------|
| **Schritt 1:** GET /api/history → JSON Array? | ☐ Ja ☐ Nein |
| **Schritt 2:** Enthält Verlaufsdaten? | ☐ Ja ☐ Nein |
| **Schritt 3:** Zeitstempel vorhanden? | ☐ Ja ☐ Nein |
| **Anzahl Datenpunkte** | _____ |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-20 — Datensammlung 5min Intervall

| Feld | Eintrag |
|------|---------|
| **Schritt 1:** 10 Min warten | Startzeit: ___:___ |
| **Schritt 2:** /api/history prüfen | Endzeit: ___:___ |
| **Schritt 3:** 2 neue Datenpunkte hinzugekommen? | ☐ Ja ☐ Nein |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-21 — Theme via API

| Feld | Eintrag |
|------|---------|
| **Schritt 1:** POST /api/theme mit theme=1 | ☐ OK ☐ Fehler |
| **Schritt 2:** Display wechselt zu Overview? | ☐ Ja ☐ Nein |
| **Schritt 3:** POST /api/theme mit theme=2 | ☐ OK ☐ Fehler |
| **Schritt 4:** Display wechselt zu Detail? | ☐ Ja ☐ Nein |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-22 — Chart.js Diagramm

| Feld | Eintrag |
|------|---------|
| **Schritt 1:** WebApp im Browser öffnen | ☐ OK |
| **Schritt 2:** Liniendiagramm wird gerendert? | ☐ Ja ☐ Nein |
| **Schritt 3:** Achsenbeschriftungen lesbar? | ☐ Ja ☐ Nein |
| **Schritt 4:** Datenpunkte sichtbar? | ☐ Ja ☐ Nein |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-23 — Responsive Design

| Feld | Eintrag |
|------|---------|
| **Schritt 1:** WebApp auf Desktop (>1024px)? | ☐ OK ☐ Layout-Fehler |
| **Schritt 2:** WebApp auf Tablet (~768px)? | ☐ OK ☐ Layout-Fehler |
| **Schritt 3:** WebApp auf Mobile (~375px)? | ☐ OK ☐ Layout-Fehler |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-24 — Ringpuffer 288 Punkte

| Feld | Eintrag |
|------|---------|
| **Vorbedingung:** 24h laufen lassen | ☐ Erledigt |
| **Schritt 1:** /api/history prüfen | |
| **Schritt 2:** Max. 288 Datenpunkte? | _____ (Soll: ≤ 288) |
| **Schritt 3:** Älteste Daten werden überschrieben? | ☐ Ja ☐ Nein |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

## Phase 5: Power Management (TC-25 bis TC-27)

### TC-25 — Display-Dimming

| Feld | Eintrag |
|------|---------|
| **Vorbedingung erfüllt?** | ☐ Ja ☐ Nein — DIM_TIMEOUT = 60s |
| **Schritt 1:** Präsenz auslösen → Display hell (100%)? | ☐ Ja ☐ Nein |
| **Schritt 2:** Keine Bewegung, Stoppuhr gestartet | |
| **Schritt 3:** Dimming eingetreten nach | _____ s (Soll: 60 ±5s) |
| **Schritt 4:** Helligkeit auf ~12%? | ☐ Ja ☐ Nein |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-26 — Aufwecken per Radar

| Feld | Eintrag |
|------|---------|
| **Vorbedingung erfüllt?** | ☐ Ja ☐ Nein — Display im Dim-Modus |
| **Schritt 1:** Person nähert sich (< 2m) | |
| **Schritt 2:** Reaktionszeit geschätzt | _____ ms (Soll: < 500ms) |
| **Schritt 3:** Helligkeit nach Aufwecken = 100%? | ☐ Ja ☐ Nein |
| **3x wiederholt?** | ☐ Ja ☐ Nein |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-27 — Light Sleep

| Feld | Eintrag |
|------|---------|
| **Vorbedingung erfüllt?** | ☐ Ja ☐ Nein — SLEEP_TIMEOUT = 5 Min |
| **Schritt 1:** Keine Präsenz für 5 Min | |
| **Schritt 2:** Display aus? | ☐ Ja ☐ Nein |
| **Schritt 3:** Stromaufnahme gemessen | _____ mA (Soll: ~0.8mA) |
| **Schritt 4:** Wake-Zeit bei Präsenz | _____ ms (Soll: ~2ms) |
| **Schritt 5:** Alle Sensoren nach Wake OK? | ☐ Ja ☐ Nein |
| **Ergebnis** | ☐ PASS ☐ FAIL ☐ SKIPPED |
| **Anmerkungen** | |

---

## Phase 6: Systemtest (TC-28)

### TC-28 — 24h Komplett-Szenario

| Feld | Eintrag |
|------|---------|
| **Startzeit** | ____-__-__ ___:___ Uhr |
| **Endzeit** | ____-__-__ ___:___ Uhr |
| **Tatsächliche Dauer** | _____ h _____ min |
| **Abstürze / Reboots** | ☐ 0 ☐ 1–3 ☐ > 3 |
| **Boot-Count (NVS)** | _____ (Soll: 1 = kein Reboot) |

### Health-Report nach 24h

| Metrik | Wert | Bewertung |
|--------|------|-----------|
| Uptime | _____ h | Soll: ≥ 24h |
| Heap aktuell | _____ KB | |
| Heap Minimum | _____ KB | |
| Heap-Verlust/h | _____ KB/h | Soll: < 10 KB/h |
| Loop-Count | _____ | |
| Loop max (ms) | _____ ms | Soll: < 500ms |
| Loop avg (ms) | _____ ms | Soll: < 100ms |
| Watchdog-Warnungen | _____ | Soll: < 5 |
| Sensor-Fehler AHT | _____ | |
| Sensor-Fehler SGP | _____ | |
| Sensor-Fehler MHZ | _____ | |
| Sensor-Fehler PMS | _____ | |
| Sensor-Fehler Radar | _____ | |
| WiFi-Disconnects | _____ | Soll: < 5 |
| Anomalien | _____ | Soll: < 10 |
| **Health-Rating** | _____ | Soll: PERFEKT oder GUT |

### Stündliche Heap-Entwicklung (optional)

| Stunde | Heap (KB) | Δ zum Start |
|:------:|:---------:|:-----------:|
| 0h | | — |
| 1h | | |
| 2h | | |
| 4h | | |
| 8h | | |
| 12h | | |
| 16h | | |
| 20h | | |
| 24h | | |

---

## Gesamtergebnis

### Einzeltests Sensoren

| TC | Testname | Status | Datum | Tester |
|----|----------|:------:|-------|--------|
| TC-01 | Display ST7796S | | | |
| TC-02 | AHT20 Temp/Feuchte | | | |
| TC-03 | SGP40 VOC | | | |
| TC-04 | MH-Z19C CO₂ | | | |
| TC-05 | PMS5003 Feinstaub | | | |
| TC-06 | LD2410C Radar | | | |

### Hardware-Tests

| TC | Testname | Status | Datum | Tester |
|----|----------|:------:|-------|--------|
| TC-07 | Backlight PWM GPIO 3 | | | |
| TC-08 | Stützkondensatoren | | | |
| TC-09 | Alle Sensoren parallel | | | |

### UI-Tests (5 Themes)

| TC | Testname | Status | Datum | Tester |
|----|----------|:------:|-------|--------|
| TC-10 | Tree Screen | | | |
| TC-11 | Overview Screen | | | |
| TC-12 | Detail Screen | | | |
| TC-13 | Analog Screen | | | |
| TC-14 | Bubble Screen | | | |
| TC-15 | Button-Wechsel | | | |
| TC-16 | Werte-Konsistenz | | | |

### WebApp & 24h-Diagramm

| TC | Testname | Status | Datum | Tester |
|----|----------|:------:|-------|--------|
| TC-17 | WebServer Init | | | |
| TC-18 | API /api/sensors | | | |
| TC-19 | API /api/history | | | |
| TC-20 | Datensammlung 5min | | | |
| TC-21 | Theme via API | | | |
| TC-22 | Chart.js Diagramm | | | |
| TC-23 | Responsive Design | | | |
| TC-24 | Ringpuffer 288 Punkte | | | |

### Power Management

| TC | Testname | Status | Datum | Tester |
|----|----------|:------:|-------|--------|
| TC-25 | Display-Dimming | | | |
| TC-26 | Aufwecken per Radar | | | |
| TC-27 | Light Sleep | | | |

### Systemtest

| TC | Testname | Status | Datum | Tester |
|----|----------|:------:|-------|--------|
| TC-28 | 24h Komplett-Szenario | | | |

---

**Tests bestanden:** _____ / 28  
**Tests nicht bestanden:** _____ / 28  
**Gesamtergebnis:** ☐ BESTANDEN ☐ BESTANDEN (bedingt) ☐ NICHT BESTANDEN

---

**Anmerkungen / Auffälligkeiten:**

_Freitext hier eintragen..._

---

**Unterschrift:** _________________________ **Datum:** _____________
