/**
 * ═══════════════════════════════════════════════════════════════════════════
 * INSPECTAIR - WEB REMOTE CONTROL
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Einfacher HTTP-Webserver auf dem ESP32 für Fernsteuerung per Handy-Browser.
 * Erlaubt Screen-Wechsel, Display-Aufwecken und zeigt Live-Sensorwerte.
 *
 * Nutzung: Im Browser http://<ESP-IP> aufrufen.
 *
 * Aktivierung: #define WEBREMOTE_ENABLED in main.cpp
 */

#ifndef WEB_REMOTE_H
#define WEB_REMOTE_H

#include <Arduino.h>

/**
 * @brief Startet den Webserver (nach WiFi-Verbindung aufrufen)
 */
void webRemote_begin();

/**
 * @brief In loop() aufrufen – verarbeitet eingehende HTTP-Requests
 */
void webRemote_loop();

#endif // WEB_REMOTE_H
