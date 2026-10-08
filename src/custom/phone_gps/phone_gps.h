#ifndef __PHONE_GPS_H__
#define __PHONE_GPS_H__

#include <Arduino.h>
#include <MenuItemInterface.h>

class PhoneGpsMenu : public MenuItemInterface {
public:
    PhoneGpsMenu() : MenuItemInterface("Phone GPS") {}

    void optionsMenu(void);
    void drawIcon(float scale);
    bool hasTheme() { return false; }
    const String& themePath() override {
        static String empty = "";
        return empty;
    }
};

#endif
