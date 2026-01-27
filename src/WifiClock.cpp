#include "WifiClock.h"

// NTP Server und Zeitzone für Deutschland
const char* ntpServer = "pool.ntp.org";
// Zeitzone: CET-1CEST,M3.5.0,M10.5.0/3 (Berlin)
const char* timeZone = "CET-1CEST,M3.5.0,M10.5.0/3";

WifiClock::WifiClock() {
}

void WifiClock::begin(const char* ssid, const char* password) {
    Serial.print("Verbinde mit WiFi: ");Serial.println(ssid);
    WiFi.begin(ssid, password);
    
    // Wir warten hier nicht blockierend ewig, damit der Rest booten kann,
    // aber NTP braucht eine Verbindung.
    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 20) {
        delay(500);
        Serial.print(".");
        retry++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi verbunden!");
        // Zeit konfigurieren
        configTzTime(timeZone, ntpServer);
    } else {
        Serial.println("\nWiFi Verbindung fehlgeschlagen (läuft im Hintergrund weiter).");
    }
}

void WifiClock::update() {
    // Hier könnte man Reconnect-Logik einbauen, falls gewünscht
}

String WifiClock::getFormattedTime() {
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        return "--:--";
    }
    char timeStringBuff[10];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%H:%M", &timeinfo);
    return String(timeStringBuff);
}

String WifiClock::getFormattedDate() {
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        return "--.--.----";
    }
    char dateStringBuff[20];
    strftime(dateStringBuff, sizeof(dateStringBuff), "%d.%m.%Y", &timeinfo);
    return String(dateStringBuff);
}