#include "wifi_handler.h"

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <ESPmDNS.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#endif

#include "common/globals.h"
#include "device_configuration.h"

const char *ssid, *password, *hostname;

uint64_t lastCheckedMillis = 0;

bool connectWiFi(const char *ssid, const char *password, const char *hostname)
{
    bool connected = false;
    uint8_t numRetries = 5;
    IPAddress ipAddress = IPAddress((uint32_t)0);

#ifdef ESP8266
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
#endif

    if (configMode)
    {
        WiFi.mode(WIFI_AP);
        connected = WiFi.softAP(ssid, password);
        ipAddress = WiFi.softAPIP();
    }
    else
    {
        while (!connected && numRetries-- > 0)
        {
            WiFi.mode(WIFI_STA);
            // Uncomment to set dns
            // WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, dns);
            WiFi.begin(ssid, password);
            DEBUG_PRINT(F("Connecting to WiFi..."));
            uint64_t connectionAttemptBeginMillis = millis();

            while (millis() - connectionAttemptBeginMillis < wifiConnectionMaxMillis && !connected)
            {
                delay(500);
                DEBUG_PRINT(F("."));
                if (WiFi.status() == WL_CONNECTED)
                {
                    connected = true;
                    ipAddress = WiFi.localIP();
                }
            }
            if (!connected)
                DEBUG_PRINTLN(F("\nUnable to connect. Trying again."));
        }
        if (!connected)
        {
            DEBUG_PRINTLN(F("Connection to WiFi unsuccessful. Entering config mode"));
            configMode = true;
            return setupWifi();
            // this is actually very dangerous. Don't uncomment unless you know exactly what you
            // are doing. And even then, remember that you did it once you hit unwanted behavior.
            // saveQuickRestartsToEeprom(true); // use the just restarted logic to enter in config mode
            // ESP.restart(); 
                              
        }
        DEBUG_PRINTLN("\nConnected to WiFi with IP address " + ipAddress.toString());
    }

    // Initialize mDNS
    MDNS.end();
    delay(1000);
    if (!MDNS.begin(hostname))
    {
        LOG_PRINTLN(F("Error setting up MDNS responder!"));
    }
    DEBUG_PRINTLN("mDNS responder started with hostname " + String(hostname));

    return true;
}

bool setupWifi()
{
    LOG_PRINT(F("Setting up WiFi in "));
    LOG_PRINT(configMode ? F("AP") : F("STA"));
    LOG_PRINTLN(F(" mode."));
    if (configMode)
    {
        ssid = configModeSsid;
        password = "";
        hostname = configModeHostname;
    }
    else
    {
        ssid = currentDeviceConfiguration->ssid;
        password = currentDeviceConfiguration->password;
        hostname = currentDeviceConfiguration->hostname;
    }

    WiFi.disconnect();
    delay(200);
    WiFi.softAPdisconnect(true);
    delay(200);
    WiFi.mode(WIFI_OFF); // Turn off to reset the Wi-Fi mode
    delay(3000);         // Short delay to allow Wi-Fi hardware to reset

    return connectWiFi(ssid, password, hostname);
}

void loopWiFi()
{
#ifdef ESP8266
    MDNS.update();
#endif

    if (configMode)
        return;

    // Make sure WiFi is connected, reconnect if necessary
    if (millis() - lastCheckedMillis >= wifiConnectionStatusCheckMillis)
    {
        lastCheckedMillis = millis();
        WiFiClient client;
        const char *host = "www.google.com";
        const int port = 80; // HTTP port

        if (WiFi.status() != WL_CONNECTED || !client.connect(host, port))
        {
            LOG_PRINTLN("Wifi disconnected. Attemting new wifi setupx");
            setupWifi();
        }
    }
}
