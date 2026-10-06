#include "StatusPanelMenu.h"
#include "core/display.h"
#include "core/utils.h"
#include <WiFi.h>
#include <SD.h>

void StatusPanelMenu::drawIcon(float scale) {
    clearIconArea();
    int r = scale * 30;
    tft.drawRect(iconCenterX - r, iconCenterY - r, 2 * r, 2 * r, bruceConfig.priColor);
    tft.drawCentreString("i", iconCenterX, iconCenterY - 8, 2);
}

void StatusPanelMenu::optionsMenu() {
    while (!check(EscPress)) {
        tft.fillRect(0, 27, tftWidth, tftHeight - 27, bruceConfig.bgColor);

        int y = 35;
        tft.setTextSize(FP);

        tft.drawString("Battery: " + String(getBattery()) + "%", 10, y);
        y += LH * FP;

        tft.drawString(
            "WiFi: " + String(wifiConnected ? "Connected" : "Disconnected"), 10, y
        );
        y += LH * FP;

        tft.drawString("SD free: " + formatBytes(SD.totalBytes() - SD.usedBytes()), 10, y);
        y += LH * FP;

        delay(1000);
    }
}
