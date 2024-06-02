#include "server_handler.h"

#ifdef ESP32
#include <HTTPClient.h>
#elif defined(ESP8266)
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#endif

#include "common/globals.h"

AsyncWebServer *webServer;
std::map<String, String> routeDescriptions;

void setupServer()
{
    webServer = new AsyncWebServer(80);

    // Default routes
    webServer->on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request)
                  { rootReboot(request); });
    routeDescriptions["/reboot"] = "";

    webServer->on("/configureDevice", HTTP_GET, [](AsyncWebServerRequest *request)
                  { routeConfigure(request); });
    routeDescriptions["/configureDevice"] = "Device configuration (wifi, hostname, github token)";

    webServer->on("/saveConfiguration", HTTP_POST, [](AsyncWebServerRequest *request)
                  { routeSaveConfiguration(request); });

    webServer->on("/invalidateConfig", HTTP_GET, [](AsyncWebServerRequest *request)
                  { routeInvaldateConfig(request); });
    routeDescriptions["/invalidateConfig"] = "";

    webServer->on("/checkForUpdates", HTTP_GET, [](AsyncWebServerRequest *request)
                  { routeCheckUpdate(request); });
    routeDescriptions["/checkForUpdates"] = "Checks for newer firmware on github";

    webServer->addHandler(&wsLogs);
    webServer->on("/logsStream", HTTP_GET, [](AsyncWebServerRequest *request)
                  { routeLogsStream(request); });
    routeDescriptions["/logsStream"] = "Get a logs streaming for remote debugging";

    // Add more routes here
    // if (!configMode)
    // {
    //     webServer->on("/routePath", HTTP_MODE, func);
    // }

    // Start the server
    webServer->begin();
}

void loopServer()
{
    // Not needed for AsyncWebServer
    // webServer->handleClient(); // Handle client requests
}
