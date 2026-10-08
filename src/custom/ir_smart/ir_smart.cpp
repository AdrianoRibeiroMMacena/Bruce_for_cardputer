#include "ir_smart.h"
#include "core/display.h"
#include "core/sd_functions.h"
#include "core/utils.h"
#include "modules/ir/custom_ir.h"
#include "modules/ir/ir_utils.h"
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRutils.h>
#include <globals.h>

struct GuessCmd {
    const char *label;
    const char *hexCommand;
};

static GuessCmd commonNecGuesses[] = {
    {"Power",  "0x12"},
    {"Vol +",  "0x02"},
    {"Vol -",  "0x03"},
    {"Ch +",   "0x00"},
    {"Ch -",   "0x01"},
    {"Mute",   "0x0D"},
};
#define NUM_GUESSES (sizeof(commonNecGuesses) / sizeof(GuessCmd))

void IrSmartMenu::drawIcon(float scale) {
    clearIconArea();
    int r = scale * 22;
    tft.drawCircle(iconCenterX, iconCenterY, r, bruceConfig.priColor);
    tft.drawCircle(iconCenterX, iconCenterY, r - 8, bruceConfig.priColor);
}

void IrSmartMenu::optionsMenu() {
    returnToMenu = false;
    setup_ir_pin(bruceConfigPins.irRx, INPUT);

    IRrecv irrecv(bruceConfigPins.irRx, SAFE_STACK_BUFFER_SIZE / 2, 50);
    irrecv.enableIRIn();
    decode_results results;

    tft.fillScreen(bruceConfig.bgColor);
    tft.drawCentreString("Point a remote and", tftWidth / 2, 30, 1);
    tft.drawCentreString("press any button", tftWidth / 2, 50, 1);
    tft.drawCentreString("(ESC to cancel)", tftWidth / 2, 70, 1);

    bool captured = false;
    String address = "";

    while (!check(EscPress)) {
        if (irrecv.decode(&results)) {
            if (results.decode_type == decode_type_t::NEC) {
                address = uint64ToString(results.address);
                captured = true;
                irrecv.resume();
                break;
            }
            irrecv.resume();
        }
        delay(20);
    }

    if (!captured) return;

    tft.fillScreen(bruceConfig.bgColor);
    tft.drawCentreString("Captured! Addr:", tftWidth / 2, 20, 1);
    tft.drawCentreString(address, tftWidth / 2, 40, 1);
    delay(1000);

    while (!returnToMenu) {
        options.clear();
        for (unsigned int i = 0; i < NUM_GUESSES; i++) {
            String label = String(commonNecGuesses[i].label);
            options.push_back({label, [=]() {}});
        }

        addOptionToMainMenu();
        int selected = loopOptions(options);
        if (returnToMenu) break;
        if (selected < 0 || selected >= (int)NUM_GUESSES) continue;

        String cmdHex = String(commonNecGuesses[selected].hexCommand);
        sendNECCommand(address, cmdHex, true);

        tft.fillScreen(bruceConfig.bgColor);
        tft.drawCentreString("Sent: " + String(commonNecGuesses[selected].label), tftWidth / 2, 30, 1);
        tft.drawCentreString("SEL = worked, save", tftWidth / 2, 55, 1);
        tft.drawCentreString("Any key = next", tftWidth / 2, 70, 1);

        unsigned long start = millis();
        while (millis() - start < 2000) {
            if (check(SelPress)) {
                FS *fs;
                if (getFsStorage(fs)) {
                    File f = fs->open("/BruceRF/ir_smart_guesses.csv", FILE_APPEND);
                    if (f) {
                        f.println(
                            String(timeStr) + "," + address + "," + cmdHex + "," +
                            String(commonNecGuesses[selected].label)
                        );
                        f.close();
                    }
                }
                break;
            }
            delay(20);
        }
    }
}
