#include <Arduino.h>
#include "app_config.h"
#include <Adafruit_NeoPixel.h>

#define RGB_LED_PIN 38

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
    Serial.begin(115200);
    Serial.println("Wetterstation startet... (RGB LED)");
    
    strip.begin();
    strip.setBrightness(50);
    strip.show();
}

void loop() {
    strip.setPixelColor(0, strip.Color(255, 0, 0));
    strip.show();
    delay(1000);
    
    strip.setPixelColor(0, strip.Color(255, 255, 0));
    strip.show();
    delay(1000);
    
    strip.setPixelColor(0, strip.Color(0, 255, 0));
    strip.show();
    delay(1000);
    
    strip.setPixelColor(0, strip.Color(0, 0, 0));
    strip.show();
    delay(500);
}