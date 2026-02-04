/**
 * @file WifiClock.h
 * @brief WiFi-Verbindung und NTP-Zeitsynchronisation
 * @author Team InspectAir
 * @date Januar 2026
 */

#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "time.h"

/**
 * @brief Klasse für WiFi-Verbindung und NTP-Zeitsynchronisation
 */
class WifiClock {
public:
    WifiClock();
    
    /**
     * @brief Initialisiert WiFi und NTP mit einem Netzwerk
     * @param ssid WLAN-Name
     * @param password WLAN-Passwort
     */
    void begin(const char* ssid, const char* password);
    
    /**
     * @brief Initialisiert WiFi und NTP mit Fallback-Netzwerk
     * @param ssid1 Primäres WLAN
     * @param password1 Primäres Passwort
     * @param ssid2 Fallback WLAN
     * @param password2 Fallback Passwort
     */
    void begin(const char* ssid1, const char* password1, const char* ssid2, const char* password2);
    
    /**
     * @brief Prüft Verbindung und reconnected bei Bedarf
     */
    void update();
    
    /**
     * @brief Schreibt formatierte Uhrzeit in Buffer (z.B. "14:05")
     * @param buf Ziel-Buffer
     * @param len Buffer-Größe
     */
    void getFormattedTime(char* buf, size_t len);
    
    /**
     * @brief Schreibt formatiertes Datum in Buffer (z.B. "25.01.2026")
     * @param buf Ziel-Buffer
     * @param len Buffer-Größe
     */
    void getFormattedDate(char* buf, size_t len);

private:
    const char* _ssid = nullptr;
    const char* _password = nullptr;
    const char* _ssid2 = nullptr;
    const char* _password2 = nullptr;

    bool connectTo(const char* ssid, const char* password, int retries);
};