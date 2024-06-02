#include "common/globals.h"
#include "common/version.h"

// LED
#ifdef ESP32
const uint8_t integratedLEDPin = 1;
#elif defined(ESP8266)
const uint8_t integratedLEDPin = 2;
#endif
const uint ledFlashMinInterval = 4000;

// Firmware
const char *BINARY_NAME = "esp32devkitc.bin";

// GitHub
const char *releaseRepo = "rmfalco89/sump_pump-control";

// Watchdog -> must be less than quick restart
const int watchdogTimeout_s = 15; // 15s


// Quick Restart && Config Mode
const uint8_t bootLoopModeMinCount = 5;
const uint16_t quickRestarMaxDurationMillis = 20 * 1000;   // 30s
const uint32_t configModeCheckEveryMillis = 2 * 60 * 1000; // 2m
uint8_t minQuickRestartCountToEnterConfigMode = 2;

// Wifi
const char *configModeSsid = "ArduinoNet";
const char *configModeHostname = "arduino";
const uint32_t wifiConnectionStatusCheckMillis = 3 * 60 * 1000; // 3m
const uint16_t wifiConnectionMaxMillis = 12 * 1000;             // 12s
const IPAddress dns(8, 8, 8, 8);                                // Google's DNS

// Ram Stats
uint64_t ramStatsUpdateIntervalMillis = 30000;