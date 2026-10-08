#ifndef __AUDIO_TRIGGER_H__
#define __AUDIO_TRIGGER_H__

#include <Arduino.h>
#include <MenuItemInterface.h>

class AudioTriggerMenu : public MenuItemInterface {
public:
    AudioTriggerMenu() : MenuItemInterface("Audio Trig") {}

    void optionsMenu(void);
    void drawIcon(float scale);
    bool hasTheme() { return false; }
    const String& themePath() override {
        static String empty = "";
        return empty;
    }
};

#endif
