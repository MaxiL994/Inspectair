#include "WifiClock.h"
#include "debug_log.h"

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
    
    LOG_I("WIFI", "Verbinde mit: %s", ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < retries) {
        delay(WIFI_RETRY_DELAY_MS);
        retry++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        LOG_I("WIFI", "Verbunden! IP: %s", WiFi.localIP().toString().c_str());
        return true;
    }
    
    LOG_W("WIFI", "Verbindung fehlgeschlagen (Status: %d)", WiFi.status());
    return false;
}

void WifiClock::begin(const char* ssid, const char* password) {
    _ssid = ssid;
    _password = password;
    _ssid2 = nullptr;
    _password2 = nullptr;
    
    if (connectTo(ssid, password, WIFI_CONNECT_RETRIES)) {
        configTzTime(TIME_ZONE, NTP_SERVER);
        
        struct tm timeinfo;
        int ntpRetry = 0;
        while (!getLocalTime(&timeinfo) && ntpRetry < NTP_SYNC_RETRIES) {
            delay(WIFI_RETRY_DELAY_MS);
            ntpRetry++;
        }
        
        if (getLocalTime(&timeinfo)) {
            LOG_I("WIFI", "NTP: %02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        } else {
            LOG_W("WIFI", "NTP Sync fehlgeschlagen!");
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
        ok = connectTo(ssid2, password2, WIFI_CONNECT_RETRIES);
        if (ok) {
            _ssid = ssid2;
            _password = password2;
        }
    }
    
    if (ok) {
        configTzTime(TIME_ZONE, NTP_SERVER);
        
        struct tm timeinfo;
        int ntpRetry = 0;
        while (!getLocalTime(&timeinfo) && ntpRetry < NTP_SYNC_RETRIES) {
            delay(WIFI_RETRY_DELAY_MS);
            ntpRetry++;
        }
        
        if (getLocalTime(&timeinfo)) {
            LOG_I("WIFI", "NTP: %02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        } else {
            LOG_W("WIFI", "NTP Sync fehlgeschlagen!");
        }
    }
}

void WifiClock::update() {
    if (WiFi.status() != WL_CONNECTED && _ssid != nullptr) {
        static unsigned long lastReconnect = 0;
        if (millis() - lastReconnect > RECONNECT_INTERVAL_MS) {
            lastReconnect = millis();
            LOG_W("WIFI", "Reconnect... (Status: %d)", WiFi.status());
            WiFi.disconnect();
            delay(100);
            
            bool ok = connectTo(_ssid, _password, WIFI_RECONNECT_RETRIES);
            if (!ok && _ssid2 && _password2) {
                ok = connectTo(_ssid2, _password2, WIFI_RECONNECT_RETRIES);
                if (ok) {
                    _ssid = _ssid2;
                    _password = _password2;
                }
            }
            
            if (ok) {
                configTzTime(TIME_ZONE, NTP_SERVER);
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