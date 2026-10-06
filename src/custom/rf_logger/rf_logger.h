#ifndef __RF_LOGGER_H__
#define __RF_LOGGER_H__

#include <Arduino.h>
#include "modules/rf/structs.h"

void logCapturedSignal(float frequency, RfCodes codes, bool raw);

#endif
