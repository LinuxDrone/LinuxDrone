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

#include "quaternion.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagMatrix3 {
    float m[9];

} MATRIX3;

extern const MATRIX3 matrix3Identity;

MATRIX3 matrix3Make(float m00, float m01, float m02,
        float m10, float m11, float m12,
        float m20, float m21, float m22);

MATRIX3 matrix3MakeAndTranspose(float m00, float m01, float m02,
        float m10, float m11, float m12,
        float m20, float m21, float m22);

MATRIX3 matrix3MakeWithArray(float values[9]);

MATRIX3 matrix3MakeWithArrayAndTranspose(float values[9]);

MATRIX3 matrix3MakeWihColumns(VECTOR3 column0, VECTOR3 column1, VECTOR3 column2);

MATRIX3 matrix3MakeWihRows(VECTOR3 row0, VECTOR3 row1, VECTOR3 row2);

MATRIX3 matrix3MakeWithQuaternion(QUATERNION q);

MATRIX3 matrix3MakeScale(float sx, float sy, float sz);

MATRIX3 matrix3MakeRotation(float radians, float x, float y, float z);

MATRIX3 matrix3MakeXRotation(float radians);

MATRIX3 matrix3MakeYRotation(float radians);

MATRIX3 matrix3MakeZRotation(float radians);

MATRIX3 matrix3SetColumn(MATRIX3 matrix, int column, VECTOR3 vector);

VECTOR3 matrix3GetColumn(MATRIX3 matrix, int column);

MATRIX3 matrix3SetRow(MATRIX3 matrix, int row, VECTOR3 vector);

VECTOR3 matrix3GetRow(MATRIX3 matrix, int row);

MATRIX3 matrix3Transpose(MATRIX3 matrix);

MATRIX3 matrix3Multiply(MATRIX3 matrixLeft, MATRIX3 matrixRight);

MATRIX3 matrix3Add(MATRIX3 matrixLeft, MATRIX3 matrixRight);

MATRIX3 matrix3Subtract(MATRIX3 matrixLeft, MATRIX3 matrixRight);

MATRIX3 matrix3Scale(MATRIX3 matrix, float sx, float sy, float sz);

MATRIX3 matrix3ScaleWithVector3(MATRIX3 matrix, VECTOR3 scaleVector);

MATRIX3 matrix3ScaleWithVector4(MATRIX3 matrix, VECTOR4 scaleVector);

MATRIX3 matrix3Rotate(MATRIX3 matrix, float radians, float x, float y, float z);

MATRIX3 matrix3RotateWithVector3(MATRIX3 matrix, float radians, VECTOR3 axisVector);

MATRIX3 matrix3RotateWithVector4(MATRIX3 matrix, float radians, VECTOR4 axisVector);

MATRIX3 matrix3RotateX(MATRIX3 matrix, float radians);

MATRIX3 matrix3RotateY(MATRIX3 matrix, float radians);

MATRIX3 matrix3RotateZ(MATRIX3 matrix, float radians);

VECTOR3 matrix3MultiplyVector3(MATRIX3 matrixLeft, VECTOR3 vectorRight);

#ifdef __cplusplus
}
#endif
