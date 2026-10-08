#include "phone_gps.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/sd_functions.h"
#include "core/utils.h"
#include <TinyGPSPlus.h>
#include <WiFi.h>
#include <globals.h>

void PhoneGpsMenu::drawIcon(float scale) {
    clearIconArea();
    int r = scale * 22;
    tft.drawCircle(iconCenterX, iconCenterY, r, bruceConfig.priColor);
    tft.drawCircle(iconCenterX, iconCenterY, r / 3, bruceConfig.priColor);
}

void PhoneGpsMenu::optionsMenu() {
    returnToMenu = false;

    if (!WiFi.isConnected()) {
        displayError("Connect to WiFi first");
        return;
    }

    String ip = keyboard("", 15, "Phone IP (GPS app):");
    if (ip == "") return;
    String portStr = keyboard("11123", 6, "Port:");
    int port = portStr.toInt();
    if (port <= 0) port = 11123;

    WiFiClient client;
    tft.fillScreen(bruceConfig.bgColor);
    tft.drawCentreString("Connecting...", tftWidth / 2, 40, 1);

    if (!client.connect(ip.c_str(), port)) {
        displayError("Connection failed");
        delay(1500);
        return;
    }

    TinyGPSPlus gps;
    FS *fs;
    bool hasStorage = getFsStorage(fs);
    bool logStarted = false;

    while (!check(EscPress) && client.connected()) {
        while (client.available()) { gps.encode(client.read()); }

        tft.fillScreen(bruceConfig.bgColor);
        tft.drawCentreString("Phone GPS", tftWidth / 2, 15, 1);

        if (gps.location.isValid()) {
            char buf[40];
            snprintf(buf, sizeof(buf), "Lat: %.6f", gps.location.lat());
            tft.drawCentreString(buf, tftWidth / 2, 40, 1);
            snprintf(buf, sizeof(buf), "Lng: %.6f", gps.location.lng());
            tft.drawCentreString(buf, tftWidth / 2, 55, 1);

            if (gps.altitude.isValid()) {
                snprintf(buf, sizeof(buf), "Alt: %.1fm", gps.altitude.meters());
                tft.drawCentreString(buf, tftWidth / 2, 70, 1);
            }
            if (gps.satellites.isValid()) {
                snprintf(buf, sizeof(buf), "Sats: %d", (int)gps.satellites.value());
                tft.drawCentreString(buf, tftWidth / 2, 85, 1);
            }

            if (hasStorage) {
                File f = fs->open("/BruceRF/phone_gps_log.csv", FILE_APPEND);
                if (f) {
                    if (!logStarted) {
                        f.println("timestamp,lat,lng,alt,sats");
                        logStarted = true;
                    }
                    char line[100];
                    snprintf(
                        line, sizeof(line), "%s,%.6f,%.6f,%.1f,%d", timeStr, gps.location.lat(),
                        gps.location.lng(), gps.altitude.isValid() ? gps.altitude.meters() : 0.0,
                        gps.satellites.isValid() ? (int)gps.satellites.value() : 0
                    );
                    f.println(line);
                    f.close();
                }
            }
        } else {
            tft.drawCentreString("Waiting for fix...", tftWidth / 2, 50, 1);
        }

        delay(500);
    }

    client.stop();
}
