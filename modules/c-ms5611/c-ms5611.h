//--------------------------------------------------------------------
// This file was created as a part of the LinuxDrone project:
//                http://www.linuxdrone.org
//
// Distributed under the Creative Commons Attribution-ShareAlike 4.0
// International License (see accompanying License.txt file or a copy
// at http://creativecommons.org/licenses/by-sa/4.0/legalcode)
//
// The human-readable summary of (and not a substitute for) the
// license: http://creativecommons.org/licenses/by-sa/4.0/
//--------------------------------------------------------------------

#pragma once

#include <stdbool.h>
//#include "../../../services/i2c/client/i2c_client.h"

enum oversampling {
	MS5611_OSR_256  = 0,
	MS5611_OSR_512  = 2,
	MS5611_OSR_1024 = 4,
	MS5611_OSR_2048 = 6,
	MS5611_OSR_4096 = 8,
};

// Время на преобразование температуры и давления (наносекунды)
enum conversiontime {
    MS5611_CONV_TIME_OSR_256  = 1000000,
    MS5611_CONV_TIME_OSR_512  = 1500000,
    MS5611_CONV_TIME_OSR_1024 = 2500000,
    MS5611_CONV_TIME_OSR_2048 = 5000000,
    MS5611_CONV_TIME_OSR_4096 = 11000000
};

/* MS5611 Addresses */
#define MS5611_I2C_ADDR   0x77
#define MS5611_RESET      0x1E
#define MS5611_CALIB_ADDR 0xA2
#define MS5611_CALIB_LEN     6
#define MS5611_ADC_READ   0x00
#define MS5611_PRES_ADDR  0x40
#define MS5611_TEMP_ADDR  0x50

#define MS5611_OVERSAMPLING MS5611_OSR_4096
