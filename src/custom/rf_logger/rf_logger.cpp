#include "rf_logger.h"
#include "core/sd_functions.h"
#include <globals.h>

void logCapturedSignal(float frequency, RfCodes codes, bool raw) {
    FS *fs;
    if (!getFsStorage(fs)) return;

    String filepath = "/BruceRF";
    String filename = "capture_log.csv";

    bool fileExists = fs->exists(filepath + "/" + filename);

    File logFile = fs->open(filepath + "/" + filename, FILE_APPEND);
    if (!logFile) return;

    if (!fileExists) {
        logFile.println("timestamp,frequency,protocol,data,raw");
    }

    String line = String(timeStr) + "," + String(frequency, 2) + "," +
                  (codes.protocol == "" ? "Unknown" : codes.protocol) + "," + codes.data + "," +
                  (raw ? "true" : "false");

    logFile.println(line);
    logFile.close();
}
