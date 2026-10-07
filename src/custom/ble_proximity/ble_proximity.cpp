#include "ble_proximity.h"
#include "core/display.h"
#include "core/utils.h"
#include "modules/ble/ble_common.h"
#include "modules/others/audio.h"
#include <globals.h>

#define PROXIMITY_TIMEOUT_MS 15000
#define SCAN_WINDOW_SEC 2

void BleProximityMenu::drawIcon(float scale) {
    clearIconArea();
    int r = scale * 25;
    tft.drawCircle(iconCenterX, iconCenterY, r, bruceConfig.priColor);
    tft.drawCircle(iconCenterX, iconCenterY, r / 2, bruceConfig.priColor);
}

void BleProximityMenu::optionsMenu() {
    tft.fillScreen(bruceConfig.bgColor);
    tft.drawCentreString("Scanning for devices..", tftWidth / 2, 40, 1);

    if (!ble_scan_setup() || pBLEScan == nullptr) {
        displayError("Failed to init BLE scan");
        return;
    }

    pBLEScan->clearResults();
    BLEScanResults foundDevices = pBLEScan->getResults(SCAN_WINDOW_SEC * 1000, false);
    int deviceCount = foundDevices.getCount();

    if (deviceCount == 0) {
        displayError("No devices found");
        delay(1000);
        return;
    }

    options.clear();
    std::vector<String> macList;
    for (int i = 0; i < deviceCount && i < 20; i++) {
        const NimBLEAdvertisedDevice *dev = foundDevices.getDevice(i);
        if (!dev) continue;
        String name = dev->getName().c_str();
        String addr = dev->getAddress().toString().c_str();
        String label = (name.isEmpty() ? addr : name);
        macList.push_back(addr);
        options.push_back({label, [=]() {}});
    }

    addOptionToMainMenu();
    int selected = loopOptions(options);
    if (selected < 0 || selected >= (int)macList.size()) return;

    String targetMac = macList[selected];
    unsigned long lastSeen = millis();

    while (!check(EscPress)) {
        tft.fillScreen(bruceConfig.bgColor);
        tft.drawCentreString("Watching:", tftWidth / 2, 30, 1);
        tft.drawCentreString(targetMac, tftWidth / 2, 50, 1);

        pBLEScan->clearResults();
        BLEScanResults devices = pBLEScan->getResults(SCAN_WINDOW_SEC * 1000, false);
        bool found = false;
        for (int i = 0; i < devices.getCount(); i++) {
            const NimBLEAdvertisedDevice *dev = devices.getDevice(i);
            if (!dev) continue;
            String addr = dev->getAddress().toString().c_str();
            if (addr == targetMac) {
                found = true;
                break;
            }
        }

        if (found) {
            lastSeen = millis();
            tft.drawCentreString("Status: NEARBY", tftWidth / 2, 80, 1);
        } else {
            unsigned long elapsed = millis() - lastSeen;
            tft.drawCentreString("Status: OUT OF RANGE", tftWidth / 2, 80, 1);
            if (elapsed > PROXIMITY_TIMEOUT_MS) {
                playTone(2000, 300);
                delay(100);
                playTone(2000, 300);
            }
        }
    }
}
