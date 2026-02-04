#include "WifiClock.h"

// ============================================
// KONFIGURATION
// ============================================
static const char* NTP_SERVER = "pool.ntp.org";
static const char* TIME_ZONE = "CET-1CEST,M3.5.0,M10.5.0/3";  // Berlin

static const int WIFI_CONNECT_RETRIES = 60;      // Max. Verbindungsversuche
static const int WIFI_RECONNECT_RETRIES = 20;    // Reconnect-Versuche
static const int NTP_SYNC_RETRIES = 20;          // NTP-Sync-Versuche
static const unsigned long RECONNECT_INTERVAL_MS = 30000;  // 30 Sekunden
static const unsigned long WIFI_RETRY_DELAY_MS = 500;      // Wartezeit zwischen Versuchen

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
        delay(WIFI_RETRY_DELAY_MS);
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
    
    if (connectTo(ssid, password, WIFI_CONNECT_RETRIES)) {
        Serial.println("Konfiguriere NTP...");
        configTzTime(TIME_ZONE, NTP_SERVER);
        
        Serial.print("Warte auf NTP Sync");
        struct tm timeinfo;
        int ntpRetry = 0;
        while (!getLocalTime(&timeinfo) && ntpRetry < NTP_SYNC_RETRIES) {
            delay(WIFI_RETRY_DELAY_MS);
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
    
    bool ok = connectTo(ssid1, password1, WIFI_CONNECT_RETRIES);
    if (!ok && ssid2 && password2) {
        Serial.println("Wechsle zum zweiten WLAN...");
        ok = connectTo(ssid2, password2, WIFI_CONNECT_RETRIES);
        if (ok) {
            _ssid = ssid2;
            _password = password2;
        }
    }
    
    if (ok) {
        Serial.println("Konfiguriere NTP...");
        configTzTime(TIME_ZONE, NTP_SERVER);
        
        Serial.print("Warte auf NTP Sync");
        struct tm timeinfo;
        int ntpRetry = 0;
        while (!getLocalTime(&timeinfo) && ntpRetry < NTP_SYNC_RETRIES) {
            delay(WIFI_RETRY_DELAY_MS);
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
        if (millis() - lastReconnect > RECONNECT_INTERVAL_MS) {
            lastReconnect = millis();
            Serial.printf("WiFi Reconnect... (Status: %d)\n", WiFi.status());
            WiFi.disconnect();
            delay(100);
            
            bool ok = connectTo(_ssid, _password, WIFI_RECONNECT_RETRIES);
            if (!ok && _ssid2 && _password2) {
                Serial.println("Wechsle zum zweiten WLAN...");
                ok = connectTo(_ssid2, _password2, WIFI_RECONNECT_RETRIES);
                if (ok) {
                    _ssid = _ssid2;
                    _password = _password2;
                }
            }
            
            if (ok) {
                configTzTime(TIME_ZONE, NTP_SERVER);
            } else {
                Serial.printf("\nWiFi fehlgeschlagen. Status: %d\n", WiFi.status());
            }
        }
    }
}

void WifiClock::getFormattedTime(char* buf, size_t len) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        snprintf(buf, len, "--:--");
        return;
    }
    strftime(buf, len, "%H:%M", &timeinfo);
}

void WifiClock::getFormattedDate(char* buf, size_t len) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        snprintf(buf, len, "--.--.----");
        return;
    }
    strftime(buf, len, "%d.%m.%Y", &timeinfo);
}