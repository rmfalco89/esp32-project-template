#include <Arduino.h>

#ifdef ESP32
#include <esp_task_wdt.h>
#include <esp_sleep.h>
#elif defined(ESP8266)
//
#endif

#include "common/device_configuration.h"
#include "common/eeprom_utils.tpp"
#include "common/globals.h"
#include "common/ota_handler.h"
#include "common/server_handler.h"
#include "common/wifi_handler.h"

uint64_t lastLedFlashMillis = 0;

uint8_t quickRestartsCount;
bool configMode = false;
bool bootLoopMode = false;
uint64_t configModeLastCheckMillis = 0;
ESPGithubOtaUpdate *updater = nullptr;

void commonSetup()
{
// Enable software Watchdog
#ifdef ESP32
    esp_task_wdt_init(watchdogTimeout_s, true); // true to cause a panic (reset) when the timeout is reached
    esp_task_wdt_add(NULL);                     // Passing NULL adds the current task (loop task for Arduino)
#elif defined(ESP8266)
    ESP.wdtDisable();
    ESP.wdtEnable(WDTO_8S);
#endif

    pinMode(integratedLEDPin, OUTPUT);

    Serial.begin(115200);
    LOG_PRINTLN(F("==============\n== Welcome! ==\n=============="));

    // init EEPROM addresses
    JUST_RESTARTED_EEPROM_ADDR = 0;
    DEVICE_CONFIGURATION_EEPROM_ADDR = nextEepromSlot<QuickRestarts>(JUST_RESTARTED_EEPROM_ADDR);

    // Check whether it's a quick restart or the device config is not valid
    quickRestartsCount = readQuickRestartsFromEeprom();
    readDeviceConfigurationFromEeprom();
    if (quickRestartsCount > minQuickRestartCountToEnterConfigMode || !currentDeviceConfiguration)
    {
        configMode = true;
        if (quickRestartsCount >= bootLoopModeMinCount)
        {
            DEBUG_PRINTLN("Entering bootLoopMode as quickRestartCount = " + String(quickRestartsCount));
            bootLoopMode = true;
        }
    }
    else
    {
        bootLoopMode = false;
        configMode = false;
    }

    // Quick Restart
    saveQuickRestartsToEeprom(true);

    // Wifi setup
    if (!setupWifi())
        configMode = true;

    // Server setup
    setupServer();

    // OTA Updater
    updater = new ESPGithubOtaUpdate(SW_VERSION, BINARY_NAME, releaseRepo, currentDeviceConfiguration->githubAuthToken);
    updater->registerFirmwareUploadRoutes(webServer, &routeDescriptions);
    if (!configMode)
        updater->upgradeSoftware(); // Check and perform upgrade on startup

    LOG_PRINTLN("SW_VERSION: " + String(SW_VERSION));
    LOG_PRINTLN("Common setup complete");
}

/**
 * Performs all housekeeping operations.
 * To allow caller to avoid running code other than basic functionality to setup device,
 * Returns
 * - 2 if device is in boot loop mode
 * - 1 if device is in config mode
 * - 0 if fully configured and running
 */
uint8_t commonLoop()
{
    // Housekeeping //

    // - Watchdog
#ifdef ESP32
    esp_task_wdt_reset();
#elif defined(ESP8266)
    ESP.wdtFeed();
#endif

    // LED flash
    if (millis() - lastLedFlashMillis > ledFlashMinInterval)
    {
        Serial.print("Alive signal flash LED: ");
        Serial.flush();
        pinMode(integratedLEDPin, OUTPUT); // temporarely change pin mode
        lastLedFlashMillis = millis();
        for (int i = 0; i < 5; i++)
        {
            digitalWrite(integratedLEDPin, HIGH);
            delay(60);
            digitalWrite(integratedLEDPin, LOW);
            delay(40);
        }
#ifdef ESP32
        gpio_matrix_out(GPIO_NUM_1, U0TXD_OUT_IDX, false, false); // restore pin mode
#endif
        Serial.println();
    }

    // - check if just restarted
    if (quickRestartsCount > 0 && millis() > quickRestarMaxDurationMillis)
    {
        saveQuickRestartsToEeprom(false);
        quickRestartsCount = 0;
    }
    // - check if in config mode but a valid configuration is found.
    // This covers the case where connection to WiFI was temporarily unsuccessful
    // but the configuration is valid so the rest of the code can be executed
    if (configMode && !bootLoopMode && millis() - configModeLastCheckMillis > configModeCheckEveryMillis)
    {
        configModeLastCheckMillis = millis();
        if (readDeviceConfigurationFromEeprom())
        {
            if (setupWifi())
            {
                configMode = false;
                LOG_PRINTLN("Got valid configuration and connected to wifi.");
                // delay(500);
                // ESP.restart();
            }
        }
    }

    // - wifi and server
    loopWiFi();
    loopServer();

    // - ota software updates
    if (!configMode)
    {
        updater->checkForSoftwareUpdate();
    }

    // Ram Stats
    updateMemoryStats();

    if (bootLoopMode)
        return 2;
    if (configMode)
        return 1;
    return 0;
    // End of Housekeeping //
}