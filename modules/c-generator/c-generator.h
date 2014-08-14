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

// Структура хранит параметры модуля
typedef struct generatorParams {
    bool  enState;
    bool  enMeander;
    bool  enSine;
    float period;
    float dutyCycle;
    float stateData;
    float rangeLow;
    float rangeCenter;
    float rangeHigh;
} generatorParams_t;
