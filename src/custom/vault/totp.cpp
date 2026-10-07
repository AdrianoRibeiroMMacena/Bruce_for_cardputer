#include "totp.h"
#include <mbedtls/md.h>
#include <time.h>

static const char *B32_ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

static int base32_decode(const String &input, uint8_t *output, int outLen) {
    int buffer = 0;
    int bitsLeft = 0;
    int count = 0;
    for (unsigned int i = 0; i < input.length() && count < outLen; i++) {
        char c = toupper(input[i]);
        if (c == '=' || c == ' ') continue;
        const char *p = strchr(B32_ALPHABET, c);
        if (!p) continue;
        int val = p - B32_ALPHABET;
        buffer <<= 5;
        buffer |= val;
        bitsLeft += 5;
        if (bitsLeft >= 8) {
            output[count++] = (buffer >> (bitsLeft - 8)) & 0xFF;
            bitsLeft -= 8;
        }
    }
    return count;
}

String generateTOTP(const String &base32Secret) {
    uint8_t key[64];
    int keyLen = base32_decode(base32Secret, key, sizeof(key));
    if (keyLen <= 0) return "------";

    time_t now = time(nullptr);
    uint64_t counter = now / 30;

    uint8_t msg[8];
    for (int i = 7; i >= 0; i--) {
        msg[i] = counter & 0xFF;
        counter >>= 8;
    }

    uint8_t hmac[20];
    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA1), 1);
    mbedtls_md_hmac_starts(&ctx, key, keyLen);
    mbedtls_md_hmac_update(&ctx, msg, 8);
    mbedtls_md_hmac_finish(&ctx, hmac);
    mbedtls_md_free(&ctx);

    int offset = hmac[19] & 0x0F;
    uint32_t binCode = ((hmac[offset] & 0x7F) << 24) | ((hmac[offset + 1] & 0xFF) << 16) |
                        ((hmac[offset + 2] & 0xFF) << 8) | (hmac[offset + 3] & 0xFF);

    uint32_t otp = binCode % 1000000;
    char buf[7];
    snprintf(buf, sizeof(buf), "%06u", otp);
    return String(buf);
}
