#ifndef __CUSTOM_VAULT_H__
#define __CUSTOM_VAULT_H__

#include <Arduino.h>
#include <MenuItemInterface.h>

class VaultMenu : public MenuItemInterface {
public:
    VaultMenu() : MenuItemInterface("Vault") {}

    void optionsMenu(void);
    void drawIcon(float scale);
    bool hasTheme() { return false; }
    const String& themePath() override {
        static String empty = "";
        return empty;
    }
};

#endif
