#ifndef __STATUS_PANEL_MENU_H__
#define __STATUS_PANEL_MENU_H__

#include <MenuItemInterface.h>

class StatusPanelMenu : public MenuItemInterface {
public:
    StatusPanelMenu() : MenuItemInterface("Status") {}

    void optionsMenu(void);
    void drawIcon(float scale);
    bool hasTheme() { return false; }
    const String& themePath() override {
        static String empty = "";
        return empty;
    }
};

#endif
