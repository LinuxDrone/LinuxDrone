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

#include "../../include/math/vector3.h"
#include <math.h>

VECTOR3 vector3Make(float x, float y, float z) {
    VECTOR3 result = {x, y, z};
    return result;
}

float vector3Length(VECTOR3 vector) {
    return (float) sqrt(vector.v[0] * vector.v[0] + vector.v[1] * vector.v[1] + vector.v[2] * vector.v[2]);
}

VECTOR3 vector3Normalize(VECTOR3 vector) {
    float scale = 1.0f / vector3Length(vector);
    VECTOR3 v = { vector.v[0] * scale, vector.v[1] * scale, vector.v[2] * scale };
    return v;
}
