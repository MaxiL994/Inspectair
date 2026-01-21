// User_Setup.h fuer InspectAir - ST7796S Display mit ESP32-S3

#define USER_SETUP_INFO "InspectAir_ST7796S"

// Display Treiber
#define ST7796_DRIVER

// Display Groesse
#define TFT_WIDTH  320
#define TFT_HEIGHT 480

// ESP32-S3 SPI Pins
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_CS   14
#define TFT_DC    9
#define TFT_RST  46

// Kein MISO benoetigt (nur schreiben, nicht lesen)
// #define TFT_MISO -1

// Fonts laden
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_GFXFF

// SPI Frequenz
#define SPI_FREQUENCY  27000000
#define SPI_READ_FREQUENCY  20000000
