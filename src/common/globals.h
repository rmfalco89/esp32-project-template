#ifndef GLOBALS_H
#define GLOBALS_H

#include <ESPAsyncWebServer.h>
#include <map>

#include "common/device_configuration.h"
#include "common/memory_stats.h"
#include "common/ota_handler.h"

extern const uint8_t integratedLEDPin;
extern const uint ledFlashMinInterval;

extern ESPGithubOtaUpdate *updater;
extern AsyncWebServer *webServer;

extern std::map<String, String> routeDescriptions;

extern int DEVICE_CONFIGURATION_EEPROM_ADDR;
extern DeviceConfiguration *currentDeviceConfiguration;

// Firmware
extern const char *SW_VERSION;
extern const char *BINARY_NAME;

// Watchdog
extern const int watchdogTimeout_s;
void sendToLogsWebsocket(const String &message);

// Config mode and Just Restarted
extern bool configMode;
extern bool bootLoopMode;
extern int JUST_RESTARTED_EEPROM_ADDR;
extern uint8_t minQuickRestartCountToEnterConfigMode;

extern uint8_t quickRestartsCount;
extern const uint16_t quickRestarMaxDurationMillis;
extern const uint8_t bootLoopModeMinCount;
extern const uint32_t configModeCheckEveryMillis;

// Wifi
extern const char *configModeSsid;
extern const char *configModeHostname;
extern const uint32_t wifiConnectionStatusCheckMillis;
extern const uint16_t wifiConnectionMaxMillis;
extern const IPAddress dns;

// Logs WebSocket and Ram management
extern AsyncWebSocket wsLogs;
extern MemoryStats ramStats;
extern uint64_t ramStatsUpdateIntervalMillis;

// GitHub
extern const char *releaseRepo;
extern const char *GITHUB_TOKEN;

// LOG to Serial and to WebSocket
#define LOG_PRINT(str)                    \
    {                                     \
        Serial.print(str);                \
        sendToLogsWebsocket(String(str)); \
    }
#define LOG_PRINTLN(str)                         \
    {                                            \
        Serial.println(str);                     \
        sendToLogsWebsocket(String(str) + "\n"); \
    }

// Uncomment the following line to enable debug output.
// #define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(str) LOG_PRINT(str)
#define DEBUG_PRINTLN(str) LOG_PRINTLN(str)
#else
#define DEBUG_PRINT(str)   \
    {                      \
        Serial.print(str); \
    }
#define DEBUG_PRINTLN(str)   \
    {                        \
        Serial.println(str); \
    }
#endif

#endif // GLOBALS_H