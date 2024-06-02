#include "Arduino.h"

#include "server_handler.h"
#include "common/globals.h"

#include "device_configuration.h"
#include "wifi_handler.h"

void rootReboot(AsyncWebServerRequest *request)
{
    DEBUG_PRINTLN("rootReboot");

    request->send(200, "text/plain", F("Rebooting now."));
    delay(3000);
    ESP.restart();
}

void routeInvaldateConfig(AsyncWebServerRequest *request)
{
    DEBUG_PRINTLN("routeInvaldateConfig");

    invalidateDeviceConfigurationOnEeprom();
    request->send(200, "text/plain", F("Device configuration voided. Configure at /configureDevice. Rebooting now."));
    ESP.restart();
}

void routeCheckUpdate(AsyncWebServerRequest *request)
{
    DEBUG_PRINTLN("routeCheckUpdate");

    request->send(200, "text/html", F("Checking for new firmware on github. This might take a few seconds..."));
    updater->upgradeSoftware();
}

String formatDeviceConfigurationHtmlTemplate()
{
    String ssid_str = currentDeviceConfiguration == nullptr ? "" : currentDeviceConfiguration->ssid;
    String password_str = currentDeviceConfiguration == nullptr ? "" : currentDeviceConfiguration->password;
    String hostname_str = currentDeviceConfiguration == nullptr ? "" : currentDeviceConfiguration->hostname;
    String deviceName_str = currentDeviceConfiguration == nullptr ? "" : currentDeviceConfiguration->deviceName;
    String githubAuthToken_str = currentDeviceConfiguration == nullptr ? "" : currentDeviceConfiguration->githubAuthToken;

    return String("<!DOCTYPE html> \
    <html><head><meta charset=\"utf-8\"><title>Configuration</title></head><body> \
    <form method=\"post\" action=\"/saveConfiguration\"> \
<label for=\"ssid\">WiFi SSID:</label>\
                        <input type =\"text\" id=\"ssid\" name=\"ssid\" value=\"" +
                  ssid_str + "\">\
<br><br>\
<label for=\"password\">WiFi Password:</label> \
<input type=\"password\" id=\"password\" name=\"password\" value=\"" +
                  password_str + "\">\
<br><br>\
<label for=\"hostname\">Hostname</label>\
<input type=\"text\" id=\"hostname\" name=\"hostname\" value=\"" +
                  hostname_str + "\">\
<br><br>\
<label for=\"device_name\">Device name</label>\
<input type=\"text\" id=\"device_name\" name=\"device_name\" value=\"" +
                  deviceName_str + "\"> \
<br><br>\
<label for=\"auth_token\">Github Auth Token</label>\
<input type=\"text\" id=\"auth_token\" name=\"auth_token\" value=\"" +
                  githubAuthToken_str + "\">\
<br><br>\
\
<input type=\"submit\" value=\"Save\"></form></body></html>");
}

void routeConfigure(AsyncWebServerRequest *request)
{
    DEBUG_PRINTLN("routeConfigure");
    request->send(200, "text/html", formatDeviceConfigurationHtmlTemplate());
}

void routeSaveConfiguration(AsyncWebServerRequest *request)
{
    Serial.print("HEREHEHERE");
    DEBUG_PRINTLN("routeSaveConfiguration");

    String ssid = request->arg("ssid");
    String password = request->arg("password");
    String hostName = request->arg("hostname");
    String deviceName = request->arg("device_name");
    String authToken = request->arg("auth_token");

    if (hostName == "")
        hostName = configModeHostname;

    DeviceConfiguration *newConfig = new DeviceConfiguration(ssid.c_str(), password.c_str(), hostName.c_str(), deviceName.c_str(), authToken.c_str());

    // Send a response to the client
    request->send(200, "text/plain", F("Configuration received. Will attempt connection to WiFi with provided credentials. Will save configuration if successful."));
    delay(250);

    currentDeviceConfiguration = newConfig;
    saveDeviceConfigurationToEeprom();
    setupWifi();
    LOG_PRINTLN(F("Configuration accepted. Setting quickRestart to 0 and Rebooting."));
    saveQuickRestartsToEeprom(false);

    ESP.restart(); // Note: will not reach this point if connection to Wifi is unsuccessful
}

void routeLogsStream(AsyncWebServerRequest *request)
{
    DEBUG_PRINTLN("routeLogsStream");
String html = R"(<!DOCTYPE html><html><head><script>
var ws = new WebSocket('ws://' + window.location.hostname + ':80/wsLogs');
ws.onmessage = function(event) {
  var container = document.getElementById('logContainer');

  var now = new Date();
  var timestamp = now.getHours() + ':' + now.getMinutes() + ':' + now.getSeconds();
  var newMessage = timestamp + " - " + event.data + "\n";
  container.textContent = newMessage + container.textContent;
};
</script></head><body>
<h1>Log Messages</h1>
<pre id='logContainer'></pre>
</body></html>
)";
    request->send(200, "text/html", html);
}

AsyncWebSocket wsLogs("/wsLogs");
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        data[len] = 0; // Null-terminate the data (if text)
        String message = "WebSocket message: " + String((char *)data);
        LOG_PRINTLN(message);
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
    switch (type)
    {
    case WS_EVT_CONNECT:
        LOG_PRINTLN("WebSocket " + String(server->url()) + " client #" + String(client->id()) +
                    " connected from " + client->remoteIP().toString());
        break;
    case WS_EVT_DISCONNECT:
        LOG_PRINTLN("WebSocket " + String(server->url()) + " client #" + String(client->id()) +
                    " disconnected");
        break;
    case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
    default:
        break;
    }
}

void sendToLogsWebsocket(const String &message)
{
    // Check if there are WebSocket clients connected
    if (wsLogs.count() > 0)
        wsLogs.textAll(message); // Send message to all connected log WebSocket clients
}
