#include "dashboard.h"
#include "core/wifi/webInterface.h"
#include "core/utils.h"
#include "core/sd_functions.h"
#include "webFiles.h"
#include <globals.h>
#include <SD.h>

static String getLastRfCapture() {
    FS *fs;
    if (!getFsStorage(fs)) return "No capture yet";

    String path = "/BruceRF/capture_log.csv";
    if (!fs->exists(path)) return "No capture yet";

    File f = fs->open(path, FILE_READ);
    if (!f) return "No capture yet";

    String lastLine = "";
    String currentLine = "";
    while (f.available()) {
        char c = f.read();
        if (c == '\n') {
            if (currentLine.length() > 0) lastLine = currentLine;
            currentLine = "";
        } else {
            currentLine += c;
        }
    }
    f.close();

    if (lastLine == "" || lastLine.startsWith("timestamp")) return "No capture yet";
    return lastLine;
}

void registerCustomRoutes(AsyncWebServer *srv) {
    srv->on("/dashboarddata", HTTP_GET, [](AsyncWebServerRequest *request) {
        char body[400];
        String lastCapture = getLastRfCapture();
        snprintf(
            body,
            sizeof(body),
            "{\"battery\":%d,\"wifi\":\"%s\",\"lastCapture\":\"%s\"}",
            getBattery(),
            wifiConnected ? "Connected" : "Disconnected",
            lastCapture.c_str()
        );
        request->send(200, "application/json", body);
    });

    srv->on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *request) {
        serveWebUIFile(request, "dashboard.html", "text/html", true, dashboard_html, dashboard_html_size);
    });
}
