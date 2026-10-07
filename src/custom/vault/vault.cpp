#include "vault.h"
#include "totp.h"
#include "core/display.h"
#include "core/utils.h"
#include "core/mykeyboard.h"
#include "core/sd_functions.h"
#include <ArduinoJson.h>
#include <globals.h>
#include <mbedtls/aes.h>
#include <mbedtls/sha256.h>

#define VAULT_PATH "/vault.bin"

static void deriveKey(const String &pin, uint8_t *key32) {
    mbedtls_sha256((const unsigned char *)pin.c_str(), pin.length(), key32, 0);
}

static String pkcs7_pad(const String &data) {
    int padLen = 16 - (data.length() % 16);
    String out = data;
    for (int i = 0; i < padLen; i++) out += (char)padLen;
    return out;
}

static String pkcs7_unpad(const String &data) {
    if (data.length() == 0) return data;
    int padLen = (uint8_t)data[data.length() - 1];
    if (padLen < 1 || padLen > 16 || padLen > (int)data.length()) return "";
    return data.substring(0, data.length() - padLen);
}

static bool saveVault(const String &pin, const String &plainJson) {
    FS *fs;
    if (!getFsStorage(fs)) return false;

    uint8_t key[32];
    deriveKey(pin, key);

    uint8_t iv[16];
    for (int i = 0; i < 16; i++) iv[i] = esp_random() & 0xFF;

    String padded = pkcs7_pad(plainJson);
    int len = padded.length();
    uint8_t *cipher = (uint8_t *)malloc(len);
    if (!cipher) return false;

    uint8_t ivCopy[16];
    memcpy(ivCopy, iv, 16);

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, key, 256);
    mbedtls_aes_crypt_cbc(
        &aes, MBEDTLS_AES_ENCRYPT, len, ivCopy, (const unsigned char *)padded.c_str(), cipher
    );
    mbedtls_aes_free(&aes);

    File f = fs->open(VAULT_PATH, FILE_WRITE);
    if (!f) {
        free(cipher);
        return false;
    }
    f.write(iv, 16);
    f.write(cipher, len);
    f.close();
    free(cipher);
    return true;
}

static bool loadVault(const String &pin, String &outJson) {
    FS *fs;
    if (!getFsStorage(fs)) return false;
    if (!fs->exists(VAULT_PATH)) return false;

    File f = fs->open(VAULT_PATH, FILE_READ);
    if (!f) return false;

    uint8_t iv[16];
    f.read(iv, 16);
    int len = f.size() - 16;
    if (len <= 0 || len % 16 != 0) {
        f.close();
        return false;
    }

    uint8_t *cipher = (uint8_t *)malloc(len);
    f.read(cipher, len);
    f.close();

    uint8_t key[32];
    deriveKey(pin, key);

    uint8_t *plain = (uint8_t *)malloc(len);
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_dec(&aes, key, 256);
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, len, iv, cipher, plain);
    mbedtls_aes_free(&aes);

    String padded = "";
    for (int i = 0; i < len; i++) padded += (char)plain[i];
    free(cipher);
    free(plain);

    outJson = pkcs7_unpad(padded);
    return outJson.length() > 0;
}

void VaultMenu::drawIcon(float scale) {
    clearIconArea();
    int w = scale * 26, h = scale * 20;
    tft.drawRoundRect(iconCenterX - w / 2, iconCenterY - h / 2 + 6, w, h, 4, bruceConfig.priColor);
    tft.drawCircle(iconCenterX, iconCenterY - h / 2, w / 4, bruceConfig.priColor);
}

void VaultMenu::optionsMenu() {
    String pin = keyboard("", 20, "Enter vault PIN:", true);
    if (pin == "") return;
    returnToMenu = false;

    String json;
    bool loaded = loadVault(pin, json);
    JsonDocument doc;
    if (loaded) {
        DeserializationError err = deserializeJson(doc, json);
        if (err) doc.to<JsonArray>();
    } else {
        doc.to<JsonArray>();
    }
    JsonArray arr = doc.as<JsonArray>();

    while (!returnToMenu) {
        options.clear();
        for (JsonObject entry : arr) {
            String label = entry["label"].as<String>();
            options.push_back({label, [=]() {}});
        }
        options.push_back({"+ Add new", [=]() {}});

        addOptionToMainMenu();
        int selected = loopOptions(options);
        if (returnToMenu) break;

        if (selected == (int)arr.size()) {
            // Add new entry
            String label = keyboard("", 30, "Label (e.g. Gmail):");
            if (label == "") continue;
            String type = keyboard("totp", 10, "Type (totp/password):");
            String value = keyboard("", 64, "Secret/Password:", type == "password");

            JsonObject newEntry = arr.add<JsonObject>();
            newEntry["label"] = label;
            newEntry["type"] = type;
            newEntry["value"] = value;

            String outJson;
            serializeJson(doc, outJson);
            saveVault(pin, outJson);

        } else if (selected >= 0 && selected < (int)arr.size()) {
            JsonObject entry = arr[selected];
            String type = entry["type"].as<String>();
            String value = entry["value"].as<String>();

            if (type == "totp") {
                while (!check(EscPress)) {
                    tft.fillScreen(bruceConfig.bgColor);
                    tft.drawCentreString(entry["label"].as<String>(), tftWidth / 2, 30, 1);
                    tft.setTextSize(3);
                    tft.drawCentreString(generateTOTP(value), tftWidth / 2, 70, 1);
                    tft.setTextSize(FP);
                    delay(1000);
                }
            } else {
                tft.fillScreen(bruceConfig.bgColor);
                tft.drawCentreString(entry["label"].as<String>(), tftWidth / 2, 30, 1);
                tft.drawCentreString(value, tftWidth / 2, 60, 1);
                tft.drawCentreString("Press any key", tftWidth / 2, 90, 1);
                while (!check(EscPress) && !check(SelPress)) { delay(50); }
            }
        }
    }
}
