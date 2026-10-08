#ifndef __IR_SMART_H__
#define __IR_SMART_H__

#include <Arduino.h>
#include <MenuItemInterface.h>

class IrSmartMenu : public MenuItemInterface {
public:
    IrSmartMenu() : MenuItemInterface("IR Smart") {}

    void optionsMenu(void);
    void drawIcon(float scale);
    bool hasTheme() { return false; }
    const String& themePath() override {
        static String empty = "";
        return empty;
    }
};

#endif
