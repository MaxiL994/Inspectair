#include "WifiClock.h"

// ============================================
// CONFIGURATION
// ============================================
static const char* NTP_SERVER = "pool.ntp.org";
static const char* TIME_ZONE = "CET-1CEST,M3.5.0,M10.5.0/3";  // Berlin

static const int WIFI_CONNECT_RETRIES = 60;      // Max connection attempts
static const int WIFI_RECONNECT_RETRIES = 20;    // Reconnect attempts
static const int NTP_SYNC_RETRIES = 20;          // NTP sync attempts
static const unsigned long RECONNECT_INTERVAL_MS = 30000;  // 30 seconds
static const unsigned long WIFI_RETRY_DELAY_MS = 500;      // Delay between attempts

WifiClock::WifiClock() {
}

bool WifiClock::connectTo(const char* ssid, const char* password, int retries) {
    if (!ssid || !password) return false;
    
    Serial.print("Connecting to WiFi: ");
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
        Serial.print("WiFi connected! IP: ");
        Serial.println(WiFi.localIP());
        return true;
    }
    
    Serial.println("\nWiFi connection failed!");
    Serial.printf("WiFi Status: %d\n", WiFi.status());
    return false;
}

void WifiClock::begin(const char* ssid, const char* password) {
    _ssid = ssid;
    _password = password;
    _ssid2 = nullptr;
    _password2 = nullptr;
    
    if (connectTo(ssid, password, WIFI_CONNECT_RETRIES)) {
        Serial.println("Configuring NTP...");
        configTzTime(TIME_ZONE, NTP_SERVER);
        
        Serial.print("Waiting for NTP sync");
        struct tm timeinfo;
        int ntpRetry = 0;
        while (!getLocalTime(&timeinfo) && ntpRetry < NTP_SYNC_RETRIES) {
            delay(WIFI_RETRY_DELAY_MS);
            Serial.print(".");
            ntpRetry++;
        }
        
        if (getLocalTime(&timeinfo)) {
            Serial.println();
            Serial.printf("Time synchronized: %02d:%02d:%02d\n", 
                         timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        } else {
            Serial.println("\nNTP sync failed!");
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
        Serial.println("Switching to second WiFi...");
        ok = connectTo(ssid2, password2, WIFI_CONNECT_RETRIES);
        if (ok) {
            _ssid = ssid2;
            _password = password2;
        }
    }
    
    if (ok) {
        Serial.println("Configuring NTP...");
        configTzTime(TIME_ZONE, NTP_SERVER);
        
        Serial.print("Waiting for NTP sync");
        struct tm timeinfo;
        int ntpRetry = 0;
        while (!getLocalTime(&timeinfo) && ntpRetry < NTP_SYNC_RETRIES) {
            delay(WIFI_RETRY_DELAY_MS);
            Serial.print(".");
            ntpRetry++;
        }
        
        if (getLocalTime(&timeinfo)) {
            Serial.println();
            Serial.printf("Time synchronized: %02d:%02d:%02d\n", 
                         timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        } else {
            Serial.println("\nNTP sync failed!");
        }
    }
}

void WifiClock::update() {
    // Reconnect if connection lost
    if (WiFi.status() != WL_CONNECTED && _ssid != nullptr) {
        static unsigned long lastReconnect = 0;
        if (millis() - lastReconnect > RECONNECT_INTERVAL_MS) {
            lastReconnect = millis();
            Serial.printf("WiFi Reconnect... (Status: %d)\n", WiFi.status());
            WiFi.disconnect();
            delay(100);
            
            bool ok = connectTo(_ssid, _password, WIFI_RECONNECT_RETRIES);
            if (!ok && _ssid2 && _password2) {
                Serial.println("Switching to second WiFi...");
                ok = connectTo(_ssid2, _password2, WIFI_RECONNECT_RETRIES);
                if (ok) {
                    _ssid = _ssid2;
                    _password = _password2;
                }
            }
            
            if (ok) {
                configTzTime(TIME_ZONE, NTP_SERVER);
            } else {
                Serial.printf("\nWiFi failed. Status: %d\n", WiFi.status());
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