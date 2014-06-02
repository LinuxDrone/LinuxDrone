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

#ifndef _C_SDK_QUATERNION_H_
#define _C_SDK_QUATERNION_H_

#include "vector3.h"
#include "vector4.h"

#ifdef __cplusplus
extern "C" {
#endif

struct tagQuaternion {
    float q[4];
} __attribute__((aligned(16)));
typedef struct tagQuaternion QUATERNION, *PQUATERNION;

QUATERNION quaternionMake(float x, float y, float z, float w);
QUATERNION quaternionMakeWithArray(float values[4]);
QUATERNION quaternionMakeWithVector3(VECTOR3 v, float scalar);
QUATERNION quaternionMakeWithAngleAndAxis(float radians, float x, float y, float z);
QUATERNION quaternionMakeWithAngleAndVector3Axis(float radians, VECTOR3 axisVector);
QUATERNION quaternionAdd(QUATERNION quaternionLeft, QUATERNION quaternionRight);
QUATERNION quaternionSubtract(QUATERNION quaternionLeft, QUATERNION quaternionRight);
QUATERNION quaternionMultiply(QUATERNION quaternionLeft, QUATERNION quaternionRight);
float quaternionLength(QUATERNION quaternion);
QUATERNION quaternionConjugate(QUATERNION quaternion);
QUATERNION quaternionInvert(QUATERNION quaternion);
QUATERNION quaternionNormalize(QUATERNION quaternion);
VECTOR3 quaternionRotateVector3(QUATERNION quaternion, VECTOR3 vector);
VECTOR4 quaternionRotateVector4(QUATERNION quaternion, VECTOR4 vector);

#ifdef __cplusplus
}
#endif

#endif // _C_SDK_QUATERNION_H_
