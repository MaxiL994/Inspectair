/**
 * @file WifiClock.h
 * @brief WiFi connection and NTP time synchronization
 * @author Team InspectAir
 * @date January 2026
 */

#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "time.h"

/**
 * @brief Class for WiFi connection and NTP time synchronization
 */
class WifiClock {
public:
    WifiClock();
    
    /**
     * @brief Initializes WiFi and NTP with one network
     * @param ssid WiFi name
     * @param password WiFi password
     */
    void begin(const char* ssid, const char* password);
    
    /**
     * @brief Initializes WiFi and NTP with fallback network
     * @param ssid1 Primary WiFi
     * @param password1 Primary password
     * @param ssid2 Fallback WiFi
     * @param password2 Fallback password
     */
    void begin(const char* ssid1, const char* password1, const char* ssid2, const char* password2);
    
    /**
     * @brief Checks connection and reconnects if needed
     */
    void update();
    
    /**
     * @brief Writes formatted time to buffer (e.g. "14:05")
     * @param buf Target buffer
     * @param len Buffer size
     */
    void getFormattedTime(char* buf, size_t len);
    
    /**
     * @brief Writes formatted date to buffer (e.g. "25.01.2026")
     * @param buf Target buffer
     * @param len Buffer size
     */
    void getFormattedDate(char* buf, size_t len);

private:
    const char* _ssid = nullptr;
    const char* _password = nullptr;
    const char* _ssid2 = nullptr;
    const char* _password2 = nullptr;

    bool connectTo(const char* ssid, const char* password, int retries);
};