#include "audio_trigger.h"
#include "core/display.h"
#include "core/sd_functions.h"
#include "core/utils.h"
#include "modules/others/mic.h"
#include <globals.h>

#define SAMPLE_RATE 16000
#define NUM_SAMPLES 512
#define GAIN 2.0
#define THRESHOLD 3000
#define RECORD_MS 5000

void AudioTriggerMenu::drawIcon(float scale) {
    clearIconArea();
    int r = scale * 20;
    tft.drawCircle(iconCenterX, iconCenterY, r, bruceConfig.priColor);
    tft.drawFastVLine(iconCenterX, iconCenterY - r / 2, r, bruceConfig.priColor);
}

void AudioTriggerMenu::optionsMenu() {
    returnToMenu = false;

    FS *fs;
    if (!getFsStorage(fs)) {
        displayError("No storage available");
        return;
    }

    while (!check(EscPress)) {
        tft.fillScreen(bruceConfig.bgColor);
        tft.drawCentreString("Listening...", tftWidth / 2, 40, 1);
        tft.drawCentreString("ESC to stop", tftWidth / 2, 60, 1);

        int16_t *samples = nullptr;
        uint32_t outRate = 0;

        if (mic_capture_samples(NUM_SAMPLES, SAMPLE_RATE, GAIN, &samples, &outRate) && samples) {
            int16_t maxAmp = 0;
            for (uint32_t i = 0; i < NUM_SAMPLES; i++) {
                int16_t v = abs(samples[i]);
                if (v > maxAmp) maxAmp = v;
            }
            free(samples);

            if (maxAmp > THRESHOLD) {
                tft.fillScreen(bruceConfig.bgColor);
                tft.drawCentreString("Triggered!", tftWidth / 2, 30, 1);
                tft.drawCentreString("Recording...", tftWidth / 2, 50, 1);

                String ts = String(timeStr);
                ts.replace(":", "-");
                String path = "/BruceAudio/rec_" + ts + ".wav";

                uint32_t outBytes = 0;
                mic_record_wav_to_path(fs, path, RECORD_MS, &outBytes, GAIN, [](void) -> bool {
                    return !check(EscPress);
                });

                tft.fillScreen(bruceConfig.bgColor);
                tft.drawCentreString("Saved:", tftWidth / 2, 30, 1);
                tft.drawCentreString(path, tftWidth / 2, 50, 1);
                delay(1500);
            }
        }

        if (returnToMenu) break;
    }
}
