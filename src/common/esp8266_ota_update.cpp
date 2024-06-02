#ifdef ESP8266

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Updater.h>

ESP8266WebServer otaServer(8888);

void setupEsp8266OtaUpdate() {
    otaServer.on("/", HTTP_GET, []() {
        otaServer.send(200, "text/html", "<form method='POST' action='/update' enctype='multipart/form-data'>"
                                         "<input type='file' name='firmware'>"
                                         "<input type='submit' value='Update Firmware'>"
                                         "</form>");
    });

    otaServer.on("/update", HTTP_POST, []() {
        otaServer.send(200, "text/plain", (Update.hasError()) ? "Update Failed" : "Update Success! Rebooting...");
        delay(2000);
        ESP.restart();
    }, []() {
        HTTPUpload& upload = otaServer.upload();
        if (upload.status == UPLOAD_FILE_START) {
            Serial.printf("Update Start: %s\n", upload.filename.c_str());
            if (!Update.begin(ESP.getFreeSketchSpace())) {
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            if (Update.end(true)) {
                Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
            } else {
                Update.printError(Serial);
            }
        }
        yield(); // Keep the watchdog timer happy
    });

    otaServer.begin();
}

void handleEsp8266OtaUpdate() {
    otaServer.handleClient();
}

#endif