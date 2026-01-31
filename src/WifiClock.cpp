#include "WifiClock.h"

// NTP Server und Zeitzone für Deutschland
const char* ntpServer = "pool.ntp.org";
// Zeitzone: CET-1CEST,M3.5.0,M10.5.0/3 (Berlin)
const char* timeZone = "CET-1CEST,M3.5.0,M10.5.0/3";

WifiClock::WifiClock() {
}

bool WifiClock::connectTo(const char* ssid, const char* password, int retries) {
    if (!ssid || !password) return false;
    
    Serial.print("Verbinde mit WiFi: ");
    Serial.println(ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < retries) {
        delay(500);
        Serial.print(".");
        retry++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.print("WiFi verbunden! IP: ");
        Serial.println(WiFi.localIP());
        return true;
    }
    
    Serial.println("\nWiFi Verbindung fehlgeschlagen!");
    Serial.printf("WiFi Status: %d\n", WiFi.status());
    return false;
}

void WifiClock::begin(const char* ssid, const char* password) {
    _ssid = ssid;
    _password = password;
    _ssid2 = nullptr;
    _password2 = nullptr;
    
    if (connectTo(ssid, password, 60)) {
        Serial.println("Konfiguriere NTP...");
        configTzTime(timeZone, ntpServer);
        
        Serial.print("Warte auf NTP Sync");
        struct tm timeinfo;
        int ntpRetry = 0;
        while (!getLocalTime(&timeinfo) && ntpRetry < 20) {
            delay(500);
            Serial.print(".");
            ntpRetry++;
        }
        
        if (getLocalTime(&timeinfo)) {
            Serial.println();
            Serial.printf("Zeit synchronisiert: %02d:%02d:%02d\n", 
                         timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        } else {
            Serial.println("\nNTP Sync fehlgeschlagen!");
        }
    }
}

void WifiClock::begin(const char* ssid1, const char* password1, const char* ssid2, const char* password2) {
    _ssid = ssid1;
    _password = password1;
    _ssid2 = ssid2;
    _password2 = password2;
    
    bool ok = connectTo(ssid1, password1, 60);
    if (!ok && ssid2 && password2) {
        Serial.println("Wechsle zum zweiten WLAN...");
        ok = connectTo(ssid2, password2, 60);
        if (ok) {
            _ssid = ssid2;
            _password = password2;
        }
    }
    
    if (ok) {
        Serial.println("Konfiguriere NTP...");
        configTzTime(timeZone, ntpServer);
        
        Serial.print("Warte auf NTP Sync");
        struct tm timeinfo;
        int ntpRetry = 0;
        while (!getLocalTime(&timeinfo) && ntpRetry < 20) {
            delay(500);
            Serial.print(".");
            ntpRetry++;
        }
        
        if (getLocalTime(&timeinfo)) {
            Serial.println();
            Serial.printf("Zeit synchronisiert: %02d:%02d:%02d\n", 
                         timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        } else {
            Serial.println("\nNTP Sync fehlgeschlagen!");
        }
    }
}

void WifiClock::update() {
    // Reconnect falls Verbindung verloren
    if (WiFi.status() != WL_CONNECTED && _ssid != nullptr) {
        static unsigned long lastReconnect = 0;
        if (millis() - lastReconnect > 30000) { // Alle 30 Sekunden
            lastReconnect = millis();
            Serial.printf("WiFi Reconnect... (Status: %d)\n", WiFi.status());
            // Status-Codes: 0=IDLE, 1=NO_SSID_AVAIL, 2=SCAN_COMPLETED, 
            // 3=CONNECTED, 4=CONNECT_FAILED, 5=CONNECTION_LOST, 6=DISCONNECTED
            WiFi.disconnect();
            delay(100);
            
            bool ok = connectTo(_ssid, _password, 20);
            if (!ok && _ssid2 && _password2) {
                Serial.println("Wechsle zum zweiten WLAN...");
                ok = connectTo(_ssid2, _password2, 20);
                if (ok) {
                    _ssid = _ssid2;
                    _password = _password2;
                }
            }
            
            if (ok) {
                configTzTime(timeZone, ntpServer);
            } else {
                Serial.printf("\nWiFi fehlgeschlagen. Status: %d\n", WiFi.status());
            }
        }
    }
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