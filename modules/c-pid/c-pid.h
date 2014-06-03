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

#include "pid.h"

#define ROLL_KP         0.5f
#define ROLL_KI         0.0f
#define ROLL_KD         0.0f
#define ROLL_ILIMIT     0.0f

#define PITH_KP         0.5f
#define PITH_KI         0.0f
#define PITH_KD         0.0f
#define PITH_ILIMIT     0.0f

#define YAW_KP          0.5f
#define YAW_KI          0.0f
#define YAW_KD          0.0f
#define YAW_ILIMIT      0.0f


enum channel {Roll, Pitch, Yaw};
