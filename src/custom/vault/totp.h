#ifndef __CUSTOM_TOTP_H__
#define __CUSTOM_TOTP_H__

#include <Arduino.h>

String generateTOTP(const String &base32Secret);

#endif
