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

// Пока все параметры задавать через дефайны.
#define DEG_ROLL_EN             true
#define OUT_RANGE_LOW_ROLL      -90.0f
#define OUT_RANGE_CENTER_ROLL   0.0f
#define OUT_RANGE_HIGH_ROLL     90.0f
#define IN0_ROLL_INVERT         true
#define IN0_RANGE_LOW_ROLL     -90.0f
#define IN0_RANGE_CENTER_ROLL   0.0f
#define IN0_RANGE_HIGH_ROLL     90.0f
#define IN1_ROLL_INVERT         false
#define IN1_RANGE_LOW_ROLL      -90.0f
#define IN1_RANGE_CENTER_ROLL   0.0f
#define IN1_RANGE_HIGH_ROLL     90.0f


#define DEG_PITCH_EN             true
#define OUT_RANGE_LOW_PITCH      -90.0f
#define OUT_RANGE_CENTER_PITCH   0.0f
#define OUT_RANGE_HIGH_PITCH     90.0f
#define IN0_PITCH_INVERT         true
#define IN0_RANGE_LOW_PITCH     -90.0f
#define IN0_RANGE_CENTER_PITCH   0.0f
#define IN0_RANGE_HIGH_PITCH     90.0f
#define IN1_PITCH_INVERT         false
#define IN1_RANGE_LOW_PITCH      1000.0f
#define IN1_RANGE_CENTER_PITCH   1500.0f
#define IN1_RANGE_HIGH_PITCH     2000.0f


#define DEG_YAW_EN             true
#define OUT_RANGE_LOW_YAW      -90.0f
#define OUT_RANGE_CENTER_YAW   0.0f
#define OUT_RANGE_HIGH_YAW     90.0f
#define IN0_YAW_INVERT         true
#define IN0_RANGE_LOW_YAW     -90.0f
#define IN0_RANGE_CENTER_YAW   0.0f
#define IN0_RANGE_HIGH_YAW     90.0f
#define IN1_YAW_INVERT         false
#define IN1_RANGE_LOW_YAW      1000.0f
#define IN1_RANGE_CENTER_YAW   1500.0f
#define IN1_RANGE_HIGH_YAW     2000.0f

enum channel {Roll, Pitch, Yaw};
