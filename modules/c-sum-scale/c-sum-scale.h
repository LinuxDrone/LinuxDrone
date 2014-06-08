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
typedef struct sumScaleChParams {
    // Канал Roll
    // Выходной сигнал в градусах
    // в диапазоне углов -180...180 deg
    bool  degEn;
    float out_RangeLow;
    float out_RangeCenter;
    float out_RangeHigh;
    bool  in0_Invert;
    float in0_RangeLow;
    float in0_RangeCenter;
    float in0_RangeHigh;
    bool  in1_Invert;
    float in1_RangeLow;
    float in1_RangeCenter;
    float in1_RangeHigh;

    float in0_scaleLow;
    float in0_scaleHigh;

    float in1_scaleLow;
    float in1_scaleHigh;

} sumScaleChParams_t;

// Входные данные модуля
struct in_data
{
    float in0;
    float in1;
};

enum channel {Roll, Pitch, Yaw};
