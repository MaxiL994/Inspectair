/**
 * ═══════════════════════════════════════════════════════════════════════════
 * INSPECTAIR - WEB REMOTE CONTROL
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * HTTP-Webserver für Handy-Fernsteuerung.
 * Zeigt eine responsive Webseite mit Buttons für Screen-Wechsel
 * und aktuelle Sensorwerte.
 */

#include "web_remote.h"
#include <WebServer.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "display/ui_manager.h"
#include "utils/power_manager.h"
#include "utils/sensor_filter.h"

static WebServer server(80);

// ============================================
// SCREEN NAMES (müssen zu UIScreen enum passen)
// ============================================
static const char* screenNames[] = {
    "Baum",        // UI_SCREEN_TREE
    "Übersicht",   // UI_SCREEN_OVERVIEW
    "Detail",      // UI_SCREEN_DETAIL
    "Analog",      // UI_SCREEN_ANALOG
    "Bubbles"      // UI_SCREEN_BUBBLE
};

// ============================================
// HTML PAGE (embedded, keine externen Dateien)
// ============================================
static const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>InspectAir Remote</title>
<style>
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body {
    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
    background: #0a0e17; color: #e0e0e0;
    min-height: 100vh; padding: 16px;
  }
  h1 { text-align: center; font-size: 1.4em; color: #4fc3f7; margin-bottom: 16px; }
  h2 { font-size: 1.1em; color: #81d4fa; margin: 16px 0 8px; }
  .screen-info {
    text-align: center; padding: 8px;
    background: #1a237e; border-radius: 8px; margin-bottom: 12px;
    font-size: 1.1em;
  }
  .btn-grid {
    display: grid; grid-template-columns: 1fr 1fr; gap: 8px;
    margin-bottom: 12px;
  }
  .btn {
    display: block; padding: 14px 8px;
    background: #1e88e5; color: white; text-decoration: none;
    border-radius: 10px; text-align: center; font-size: 1em;
    border: none; cursor: pointer; transition: background 0.2s;
  }
  .btn:active { background: #1565c0; }
  .btn.next { grid-column: 1 / -1; background: #00c853; font-size: 1.2em; }
  .btn.next:active { background: #00a844; }
  .btn.wake { background: #ff9800; }
  .btn.wake:active { background: #e68900; }
  .values {
    display: grid; grid-template-columns: 1fr 1fr; gap: 8px;
  }
  .val-card {
    background: #1a1f2e; border-radius: 10px; padding: 12px;
    text-align: center;
  }
  .val-card .label { font-size: 0.8em; color: #90a4ae; }
  .val-card .value { font-size: 1.6em; font-weight: bold; color: #4fc3f7; }
  .val-card .unit { font-size: 0.7em; color: #607d8b; }
  .refresh-note { text-align: center; margin-top: 12px; font-size: 0.75em; color: #546e7a; }
</style>
</head>
<body>
<h1>🌿 InspectAir Remote</h1>

<div class="screen-info">Aktuell: <strong>%SCREEN%</strong></div>

<a class="btn next" href="/next">▶ Nächster Screen</a>

<h2>Screens</h2>
<div class="btn-grid">
  <a class="btn" href="/screen?id=0">🌳 Baum</a>
  <a class="btn" href="/screen?id=1">📊 Übersicht</a>
  <a class="btn" href="/screen?id=2">📋 Detail</a>
  <a class="btn" href="/screen?id=3">🎛 Analog</a>
  <a class="btn" href="/screen?id=4">🫧 Bubbles</a>
  <a class="btn wake" href="/wake">☀️ Display An</a>
</div>

<h2>Sensorwerte</h2>
<div class="values">
  <div class="val-card"><div class="label">Temperatur</div><div class="value">%TEMP%</div><div class="unit">°C</div></div>
  <div class="val-card"><div class="label">Feuchte</div><div class="value">%HUM%</div><div class="unit">%%</div></div>
  <div class="val-card"><div class="label">CO₂</div><div class="value">%CO2%</div><div class="unit">ppm</div></div>
  <div class="val-card"><div class="label">VOC</div><div class="value">%VOC%</div><div class="unit">Index</div></div>
  <div class="val-card"><div class="label">PM2.5</div><div class="value">%PM25%</div><div class="unit">µg/m³</div></div>
</div>

<div class="refresh-note">Seite neu laden für aktuelle Werte</div>
</body>
</html>
)rawliteral";

// ============================================
// HELPER: Platzhalter im HTML ersetzen
// ============================================
static String buildPage() {
    String page = FPSTR(HTML_PAGE);
    
    int screenIdx = (int)ui_getCurrentScreen();
    const char* name = (screenIdx >= 0 && screenIdx < UI_SCREEN_COUNT) 
                       ? screenNames[screenIdx] : "?";
    page.replace("%SCREEN%", name);
    
    page.replace("%TEMP%", String(sensorFilter.getSmoothedTemp(), 1));
    page.replace("%HUM%",  String(sensorFilter.getSmoothedHum(), 0));
    page.replace("%CO2%",  String(sensorFilter.getSmoothedCO2()));
    page.replace("%VOC%",  String(sensorFilter.getSmoothedVOC()));
    page.replace("%PM25%", String(sensorFilter.getSmoothedPM25()));
    
    return page;
}

// ============================================
// ROUTE HANDLERS
// ============================================
static void handleRoot() {
    server.send(200, "text/html", buildPage());
}

static void handleNext() {
    powerManager.wakeUp();
    ui_nextScreen();
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "OK");
    Serial.printf("[WEB] Screen -> %d (%s)\n", ui_getCurrentScreen(), 
                  screenNames[ui_getCurrentScreen()]);
}

static void handleScreen() {
    if (server.hasArg("id")) {
        int id = server.arg("id").toInt();
        if (id >= 0 && id < UI_SCREEN_COUNT) {
            powerManager.wakeUp();
            ui_setScreen((UIScreen)id);
            Serial.printf("[WEB] Screen set to %d (%s)\n", id, screenNames[id]);
        }
    }
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "OK");
}

static void handleWake() {
    powerManager.wakeUp();
    Serial.println("[WEB] Display wake up");
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "OK");
}

// ============================================
// PUBLIC API
// ============================================
void webRemote_begin() {
    server.on("/",       handleRoot);
    server.on("/next",   handleNext);
    server.on("/screen", handleScreen);
    server.on("/wake",   handleWake);
    server.begin();
    
    // mDNS: erreichbar über http://inspectair.local
    if (MDNS.begin("inspectair")) {
        MDNS.addService("http", "tcp", 80);
        Serial.println("[WEB] mDNS: http://inspectair.local");
    }
    
    Serial.println("[WEB] ═══════════════════════════════════════════");
    Serial.printf("[WEB]  Remote: http://%s\n", WiFi.localIP().toString().c_str());
    Serial.println("[WEB]  oder:  http://inspectair.local");
    Serial.println("[WEB] ═══════════════════════════════════════════");
}

void webRemote_loop() {
    server.handleClient();
}
