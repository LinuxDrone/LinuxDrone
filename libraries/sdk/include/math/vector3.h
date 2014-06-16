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

#ifndef _C_SDK_VECTOR_3_H_
#define _C_SDK_VECTOR_3_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagVector3 {
    float v[3];

} VECTOR3, *PVECTOR3;

VECTOR3 vector3Make(float x, float y, float z);
float vector3Length(VECTOR3 vector);
VECTOR3 vector3Normalize(VECTOR3 v);

VECTOR3 vector3Add(VECTOR3 vectorLeft, VECTOR3 vectorRight);
VECTOR3 vector3Subtract(VECTOR3 vectorLeft, VECTOR3 vectorRight);
VECTOR3 vector3Multiply(VECTOR3 vectorLeft, VECTOR3 vectorRight);
VECTOR3 vector3Divide(VECTOR3 vectorLeft, VECTOR3 vectorRight);

VECTOR3 vector3AddScalar(VECTOR3 vector, float value);
VECTOR3 vector3SubtractScalar(VECTOR3 vector, float value);
VECTOR3 vector3MultiplyScalar(VECTOR3 vector, float value);
VECTOR3 vector3DivideScalar(VECTOR3 vector, float value);

VECTOR3 vector3CrossProduct(VECTOR3 vectorLeft, VECTOR3 vectorRight);

#ifdef __cplusplus
}
#endif

#endif // _C_SDK_VECTOR_3_H_
