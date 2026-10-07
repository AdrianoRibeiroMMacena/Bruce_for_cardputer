#include "ble_classifier.h"

String classifyVendor(const String &macAddress) {
    String oui = macAddress.substring(0, 8);
    oui.toUpperCase();

    if (oui == "AC:37:43" || oui == "F0:18:98" || oui == "A4:C1:38") return "Apple";
    if (oui == "3C:5A:B4" || oui == "DC:A6:32" || oui == "B8:27:EB") return "Samsung/Raspberry";
    if (oui == "00:1A:7D" || oui == "88:C6:26") return "Fitbit/Garmin";
    if (oui == "A0:E6:F8" || oui == "48:D6:D5") return "Xiaomi";
    if (oui == "00:16:94" || oui == "00:17:88") return "Philips/Hue";

    return "Unknown";
}
