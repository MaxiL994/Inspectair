#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "time.h"

class WifiClock {
public:
    WifiClock();
    
    // Initialisiert WiFi und NTP
    void begin(const char* ssid, const char* password);
    void begin(const char* ssid1, const char* password1, const char* ssid2, const char* password2);
    
    // Prüft Verbindung (optional im Loop aufrufen)
    void update();
    
    // Gibt die aktuelle Zeit als formatierten String zurück (z.B. "14:05")
    String getFormattedTime();
    
    // Gibt das volle Datum zurück (z.B. "25.01.2026")
    String getFormattedDate();

private:
    const char* _ssid = nullptr;
    const char* _password = nullptr;
    const char* _ssid2 = nullptr;
    const char* _password2 = nullptr;

    bool connectTo(const char* ssid, const char* password, int retries);
};