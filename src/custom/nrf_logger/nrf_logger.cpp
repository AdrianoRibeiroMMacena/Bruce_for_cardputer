#include "nrf_logger.h"
#include "core/sd_functions.h"
#include <globals.h>

void logNrfEvent(const String &action) {
    FS *fs;
    if (!getFsStorage(fs)) return;

    String filepath = "/BruceRF";
    String filename = "nrf_log.csv";

    bool fileExists = fs->exists(filepath + "/" + filename);

    File logFile = fs->open(filepath + "/" + filename, FILE_APPEND);
    if (!logFile) return;

    if (!fileExists) {
        logFile.println("timestamp,action");
    }

    logFile.println(String(timeStr) + "," + action);
    logFile.close();
}
