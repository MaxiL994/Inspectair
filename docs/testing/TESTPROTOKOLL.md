# INSPECTAIR — Testprotokoll

> **Durchlauf:** #___ | **Datum:** __________ | **Tester:** __________ | **Firmware-Version:** __________

---

## Testumgebung

| Parameter | Wert |
|-----------|------|
| **ESP32-Board** | ESP32-S3 DevKitC-1 (Waveshare) |
| **Firmware-Commit** | `git rev-parse --short HEAD` → ________ |
| **Branch** | display_ui |
| **PlatformIO Build** | ☐ OK ☐ Fehler |
| **COM-Port** | COM___ |
| **Serial-Log aktiv** | ☐ Ja ☐ Nein |
| **Raum-Temperatur** | _____ °C |
| **Stützkondensatoren** | ☐ Verbaut ☐ Nicht verbaut |

---

## Phase 1: Hardware-Verifikation

### TC-07 — BS170 MOSFET Backlight-Steuerung

| Feld | Eintrag |
|------|---------|
| **Vorbedingung erfüllt?** | ☐ Ja ☐ Nein — BS170: Gate→GPIO3 (1kΩ), Drain→BL-, Source→GND |
| **Schritt 1:** PWM 0% → Display dunkel? | ☐ Ja ☐ Nein |
| **Schritt 2:** PWM 50% → halbe Helligkeit? | ☐ Ja ☐ Nein |
| **Schritt 3:** PWM 100% → volle Helligkeit? | ☐ Ja ☐ Nein |
| **Schritt 4:** Fade 0→100% smooth? | ☐ Ja ☐ Nein |
| **Flackern beobachtet?** | ☐ Nein ☐ Ja (Beschreibung: ___) |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-08 — Stützkondensatoren Stabilität

| Feld | Eintrag |
|------|---------|
| **Vorbedingung erfüllt?** | ☐ Ja ☐ Nein — 220µF 25V an MH-Z19C und LD2410C |
| **Schritt 1:** System OHNE Kondensatoren → Absturz? | ☐ Ja (Absturz) ☐ Nein (stabil) |
| **Schritt 2:** System MIT Kondensatoren → 10 Min stabil? | ☐ Ja ☐ Nein |
| **Schritt 3:** 5V-Rail Spannung gemessen | _____ V (Soll: 4.8–5.2V) |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

## Phase 2: Sensor-Integration

### TC-01 bis TC-06 — Einzeltests (vorherige Ergebnisse)

| TC | Testname | Status | Datum | Tester |
|----|----------|--------|-------|--------|
| TC-01 | Display ST7796S | ✅ PASS | Jan 2026 | Team |
| TC-02 | AHT20 (Temp/Feuchte) | ✅ PASS | Jan 2026 | Team |
| TC-03 | SGP40 (VOC) | ✅ PASS | Jan 2026 | Team |
| TC-04 | MH-Z19C (CO₂) | ✅ PASS | Jan 2026 | Team |
| TC-05 | PMS5003 (Feinstaub) | ✅ PASS | Jan 2026 | Team |
| TC-06 | LD2410C (Radar) | ✅ PASS | Jan 2026 | Team |

---

### TC-09 — Integration: Alle Sensoren gleichzeitig

| Feld | Eintrag |
|------|---------|
| **Vorbedingung erfüllt?** | ☐ Ja ☐ Nein — Alle Sensoren verkabelt, Kondensatoren installiert |
| **Schritt 1:** System gestartet, alle Init [OK]? | ☐ Ja ☐ Nein |
| **Init-Meldungen aus Serial-Log:** | |
| `[I][I2C] AHT20 OK` | ☐ Ja ☐ Nein |
| `[I][I2C] SGP40 OK` | ☐ Ja ☐ Nein |
| `[I][CO2] MH-Z19C OK` | ☐ Ja ☐ Nein |
| `[I][PMS] PMS5003 OK` | ☐ Ja ☐ Nein |
| `[I][RAD] LD2410C OK` | ☐ Ja ☐ Nein |
| **Schritt 2:** 5 Min Sensor-Log ohne Fehler? | ☐ Ja ☐ Nein |
| **Anzahl `[E]`-Meldungen in 5 Min** | _____ (Soll: < 3) |
| **Sensor-Werte plausibel?** | |
| Temperatur: _____ °C (Soll: 15–30) | ☐ OK ☐ Ausreißer |
| Feuchte: _____ % (Soll: 30–70) | ☐ OK ☐ Ausreißer |
| CO₂: _____ ppm (Soll: 350–600 Frischluft) | ☐ OK ☐ Ausreißer |
| VOC-Index: _____ (Soll: < 200 normal) | ☐ OK ☐ Ausreißer |
| PM2.5: _____ µg/m³ (Soll: 0–50 Innen) | ☐ OK ☐ Ausreißer |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

## Phase 3: UI & Interaktion

### TC-10 — Screen-Darstellung (alle 5 Screens)

| Feld | Eintrag |
|------|---------|
| **Vorbedingung erfüllt?** | ☐ Ja ☐ Nein — Display + Sensoren funktional |
| **Screen 1 — Tree-Animation** | ☐ Darstellung OK ☐ Artefakte ☐ Foto gemacht |
| **Screen 2 — Overview** | ☐ AQI sichtbar ☐ 2 Kacheln OK ☐ Foto gemacht |
| **Screen 3 — Detail** | ☐ Alle 6 Werte sichtbar ☐ 4 Kacheln OK ☐ Foto gemacht |
| **Screen 4 — Analog** | ☐ Instrumente OK ☐ Zeiger bewegen sich ☐ Foto gemacht |
| **Screen 5 — Bubble** | ☐ Kreise dargestellt ☐ Dynamisch ☐ Foto gemacht |
| **Farbcodierung korrekt?** | ☐ Grün bei guten Werten ☐ Gelb/Rot bei Grenzwerten |
| **Lesbar aus 1m Entfernung?** | ☐ Ja ☐ Nein |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-11 — Screen-spezifische Features

| Screen | Feature | OK? | Anmerkung |
|--------|---------|:---:|-----------|
| Tree | Animation läuft flüssig | ☐ | |
| Tree | Reagiert auf Luftqualität (Farbe) | ☐ | |
| Overview | Große AQI-Zahl korrekt | ☐ | |
| Overview | 2 Kacheln lesbar | ☐ | |
| Detail | Alle 6 Werte + Zusatzinfos | ☐ | |
| Detail | 4 Kacheln korrekt | ☐ | |
| Analog | Zeiger bewegen sich passend | ☐ | |
| Analog | Skala lesbar | ☐ | |
| Bubble | Kreise ändern Größe dynamisch | ☐ | |
| Bubble | Kreise ändern Farbe dynamisch | ☐ | |
| **Rendering-Artefakte?** | | ☐ Nein ☐ Ja: ___ | |
| **Ergebnis** | | ☐ PASS ☐ FAIL | |

---

### TC-12 — Button-Wechsel zwischen Screens

| Feld | Eintrag |
|------|---------|
| **Vorbedingung erfüllt?** | ☐ Ja ☐ Nein — Button an GPIO1, alle 5 Screens |
| **Zyklus-Test:** | |
| Start → Tree | ☐ OK |
| Button → Overview | ☐ OK |
| Button → Detail | ☐ OK |
| Button → Analog | ☐ OK |
| Button → Bubble | ☐ OK |
| Button → Tree (Zyklus) | ☐ OK |
| **Prellen beobachtet?** | ☐ Nein ☐ Ja |
| **Übergangszeit geschätzt** | _____ ms (Soll: < 300ms) |
| **Stresstest: 20x schnell drücken** | ☐ Kein Absturz ☐ Absturz |
| **Serial-Log Auffälligkeiten** | ☐ Keine ☐ Ja: ___ |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-13 — Messwerte-Konsistenz alle Screens

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
| **5x zu verschiedenen Zeiten geprüft?** | ☐ Ja ☐ Nein |
| **Max. Abweichung beobachtet** | _____ % (Soll: < 1%) |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

## Phase 4: Power Management & System

### TC-14 — Display-Dimming nach Timeout

| Feld | Eintrag |
|------|---------|
| **Vorbedingung erfüllt?** | ☐ Ja ☐ Nein — DIM_TIMEOUT = 45s, Radar aktiv, MOSFET OK |
| **Schritt 1:** Präsenz auslösen → Display hell (100%)? | ☐ Ja ☐ Nein |
| **Schritt 2:** Keine Bewegung, Stoppuhr gestartet | |
| **Schritt 3:** Dimming eingetreten nach** | _____ s (Soll: 45 ±5s) |
| **Schritt 4:** Helligkeit sichtbar reduziert? | ☐ Ja ☐ Nein |
| **Serial-Log zeigt Dimming?** | ☐ Ja ☐ Nein |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

### TC-15 — Display-Aufwecken per Radar

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

### TC-16 — ESP32 Light Sleep Modus

| Feld | Eintrag |
|------|---------|
| **Vorbedingung erfüllt?** | ☐ Ja ☐ Nein — SLEEP_TIMEOUT = 5 Min |
| **Schritt 1:** System normal, keine Präsenz für 5 Min | |
| **Schritt 2:** Display aus? | ☐ Ja ☐ Nein |
| **Schritt 3:** Stromaufnahme gemessen | _____ mA (Soll: ~0.8mA) |
| **Schritt 4:** Präsenz auslösen → Aufgewacht? | ☐ Ja ☐ Nein |
| **Schritt 5:** Alle Sensoren nach Wake OK? | ☐ Ja ☐ Nein |
| **Ergebnis** | ☐ PASS ☐ FAIL ☐ SKIPPED (Light Sleep nicht implementiert) |
| **Anmerkungen** | |

---

### TC-17 — System: Komplett-Szenario Alltagsbetrieb

| Feld | Eintrag |
|------|---------|
| **Vorbedingung erfüllt?** | ☐ Ja ☐ Nein — Komplettsystem aufgebaut |
| **Startzeit** | ___:___ Uhr |
| **Schritt 1:** Einschalten → Tree-Animation? | ☐ Ja ☐ Nein |
| **Schritt 2:** Alle 5 Screens durchgewechselt? | ☐ Ja ☐ Nein |
| **Schritt 3:** Werte auf allen Screens korrekt? | ☐ Ja ☐ Nein |
| **Schritt 4:** Raum verlassen → Dimming nach 45s? | ☐ Ja ☐ Nein |
| **Schritt 5:** Raum betreten → Aufwecken? | ☐ Ja ☐ Nein |
| **Schritt 6:** 30 Min laufen lassen | |
| **Endzeit** | ___:___ Uhr |
| **Abstürze während 30 Min?** | ☐ Nein ☐ Ja: ___ |
| **Werte nach 30 Min noch plausibel?** | ☐ Ja ☐ Nein |
| **Health-Report (`/api/health`):** | |
| Uptime: _____ s | Heap: _____ KB | Loops: _____ |
| Sensor-Fehler gesamt: _____ | WiFi-Disconnects: _____ |
| Health-Rating: _____ | |
| **Ergebnis** | ☐ PASS ☐ FAIL |
| **Anmerkungen** | |

---

## Bonus: 24h-Dauertest

| Feld | Eintrag |
|------|---------|
| **Startzeit** | ____-__-__ ___:___ Uhr |
| **Endzeit** | ____-__-__ ___:___ Uhr |
| **Tatsächliche Dauer** | _____ h _____ min |
| **Abstürze / Reboots** | ☐ 0 ☐ 1–3 ☐ > 3 |
| **Boot-Count (NVS)** | _____ (Soll: 1 = kein Reboot) |

### Health-Report nach 24h (aus `/api/health` oder Serial)

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

| TC | Testname | Status | Datum | Tester |
|----|----------|:------:|-------|--------|
| TC-01 | Display ST7796S | ✅ | Jan 2026 | Team |
| TC-02 | AHT20 (Temp/Feuchte) | ✅ | Jan 2026 | Team |
| TC-03 | SGP40 (VOC) | ✅ | Jan 2026 | Team |
| TC-04 | MH-Z19C (CO₂) | ✅ | Jan 2026 | Team |
| TC-05 | PMS5003 (Feinstaub) | ✅ | Jan 2026 | Team |
| TC-06 | LD2410C (Radar) | ✅ | Jan 2026 | Team |
| TC-07 | BS170 MOSFET Backlight | | | |
| TC-08 | Stützkondensatoren | | | |
| TC-09 | Integration alle Sensoren | | | |
| TC-10 | UI: Screen-Darstellung | | | |
| TC-11 | UI: Screen-Features | | | |
| TC-12 | UI: Button-Wechsel | | | |
| TC-13 | UI: Messwerte-Konsistenz | | | |
| TC-14 | Power: Display-Dimming | | | |
| TC-15 | Power: Aufwecken Radar | | | |
| TC-16 | Power: Light Sleep | | | |
| TC-17 | System: Komplett-Szenario | | | |

**Tests bestanden:** _____ / 17
**Tests nicht bestanden:** _____ / 17
**Gesamtergebnis:** ☐ BESTANDEN ☐ BESTANDEN (bedingt) ☐ NICHT BESTANDEN

---

**Anmerkungen / Auffälligkeiten:**

_Freitext hier eintragen..._

---

**Unterschrift:** _________________________ **Datum:** _____________
