#ifndef SERVER_HANDLER_H
#define SERVER_HANDLER_H

#include <ESPAsyncWebServer.h>

void setupServer();
void loopServer();

// Routes go here
void rootReboot(AsyncWebServerRequest *request);
void routeConfigure(AsyncWebServerRequest *request);
void routeSaveConfiguration(AsyncWebServerRequest *request);
void routeInvaldateConfig(AsyncWebServerRequest *request);
void routeCheckUpdate(AsyncWebServerRequest *request);
void routeLogsStream(AsyncWebServerRequest *request);

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len);

#endif // SERVER_HANDLER_H