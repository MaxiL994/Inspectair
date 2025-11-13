// WICHTIG: Der "extern "C"" Wrapper
#ifdef __cplusplus
extern "C" {
#endif

// Arduino-Basisfunktionen einbinden
#include <Arduino.h>
#include "app_config.h"

// Hier binden Sie später Ihre C-Header ein:
// #include "sensors/mhz19c.h"

void setup() {
  // Setup-Code hier
  Serial.begin(115200);
  Serial.println("Wetterstation startet... (Reiner C-Code)");
}

void loop() {
  // Hauptschleife hier
  delay(1000);
}

// WICHTIG: Ende des Wrappers
#ifdef __cplusplus
}
#endif