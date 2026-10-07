#ifndef __BLE_PROXIMITY_H__
#define __BLE_PROXIMITY_H__

#include <Arduino.h>
#include <MenuItemInterface.h>

class BleProximityMenu : public MenuItemInterface {
public:
    BleProximityMenu() : MenuItemInterface("BLE Alarm") {}

    void optionsMenu(void);
    void drawIcon(float scale);
    bool hasTheme() { return false; }
    const String& themePath() override {
        static String empty = "";
        return empty;
    }
};

#endif
