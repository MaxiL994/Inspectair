/**
 * ═══════════════════════════════════════════════════════════════════════════
 * INSPECTAIR - DEBUG LOG SYSTEM
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Einheitliches Logging mit konfigurierbarem Level.
 * Level: 0=OFF, 1=ERROR, 2=WARN, 3=INFO, 4=DEBUG
 *
 * Verwendung:
 *   LOG_I("SENSOR", "Temperatur: %.1f°C", temp);
 *   LOG_E("WIFI",   "Verbindung verloren (Status: %d)", status);
 */

#ifndef DEBUG_LOG_H
#define DEBUG_LOG_H

#include <Arduino.h>

// ============================================
// LOG LEVEL KONFIGURATION
// ============================================
// Kann in platformio.ini überschrieben werden:
//   build_flags = -DLOG_LEVEL=4
#ifndef LOG_LEVEL
#define LOG_LEVEL 3  // Default: INFO (ERROR + WARN + INFO)
#endif

// ============================================
// LOG MAKROS
// ============================================
// Format: [E][TAG] Nachricht
// Jede Zeile wird automatisch mit \n abgeschlossen

#if LOG_LEVEL >= 1
  #define LOG_E(tag, fmt, ...) Serial.printf("[E][%-6s] " fmt "\n", tag, ##__VA_ARGS__)
#else
  #define LOG_E(tag, fmt, ...)
#endif

#if LOG_LEVEL >= 2
  #define LOG_W(tag, fmt, ...) Serial.printf("[W][%-6s] " fmt "\n", tag, ##__VA_ARGS__)
#else
  #define LOG_W(tag, fmt, ...)
#endif

#if LOG_LEVEL >= 3
  #define LOG_I(tag, fmt, ...) Serial.printf("[I][%-6s] " fmt "\n", tag, ##__VA_ARGS__)
#else
  #define LOG_I(tag, fmt, ...)
#endif

#if LOG_LEVEL >= 4
  #define LOG_D(tag, fmt, ...) Serial.printf("[D][%-6s] " fmt "\n", tag, ##__VA_ARGS__)
#else
  #define LOG_D(tag, fmt, ...)
#endif

// ============================================
// STARTUP BANNER
// ============================================
#define LOG_BANNER() do { \
    Serial.println(); \
    Serial.println("╔══════════════════════════════════════════╗"); \
    Serial.println("║      INSPECTAIR v3.1 - Luftqualität     ║"); \
    Serial.println("╚══════════════════════════════════════════╝"); \
    Serial.println(); \
} while(0)

#endif // DEBUG_LOG_H
