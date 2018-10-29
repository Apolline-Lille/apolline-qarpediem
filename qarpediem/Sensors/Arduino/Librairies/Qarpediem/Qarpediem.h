#ifndef __QARPEDIEM_PIN__
#define __QARPEDIEM_PIN__

#include <Arduino.h>
#include "SensorServer.h"

namespace Qarpediem {
	int const MUX_PIN1 = 2;
	int const MUX_PIN2 = 3;
	int const MUX_PIN3 = 4;
	int const SS_DUST = 5;
	int const SS_CAN = 6;
	int const DIG_TH = 7;
	int const DIG_MVT = 23;
	int const DIGI_1_LED = 25;
	int const DIGI_2_LED = 27;
	int const ANALOG_SON_ENVOLOPE = A14;
	int const ANALOG_PRESSION = A12;

	HardwareSerial* const D_SUB_SERIAL = &Serial2;
};

#endif // __QARPEDIEM_PIN__
