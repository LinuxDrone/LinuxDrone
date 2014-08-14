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

#include "../../include/math/matrix3.h"
#include <math.h>

#if defined(__ARM_NEON__)
#include <arm_neon.h>
#endif

const MATRIX3 matrix3Identity = {1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f};

MATRIX3 matrix3Make(float m00, float m01, float m02,
        float m10, float m11, float m12,
        float m20, float m21, float m22) {
    MATRIX3 m = {m00, m01, m02,
            m10, m11, m12,
            m20, m21, m22};
    return m;
}

MATRIX3 matrix3MakeAndTranspose(float m00, float m01, float m02,
        float m10, float m11, float m12,
        float m20, float m21, float m22) {
    MATRIX3 m = {m00, m10, m20,
            m01, m11, m21,
            m02, m12, m22};
    return m;
}

MATRIX3 matrix3MakeWithArray(float values[9]) {
    MATRIX3 result = {values[0], values[1], values[2],
            values[3], values[4], values[5],
            values[6], values[7], values[8]};
    return result;
}

MATRIX3 matrix3MakeWithArrayAndTranspose(float values[9]) {
    MATRIX3 result = {values[0], values[3], values[6],
            values[1], values[4], values[7],
            values[2], values[5], values[8]};
    return result;
}

MATRIX3 matrix3MakeWihColumns(VECTOR3 column0, VECTOR3 column1, VECTOR3 column2) {
    MATRIX3 result = {column0.v[0], column0.v[1], column0.v[2],
            column1.v[0], column1.v[1], column1.v[2],
            column2.v[0], column2.v[1], column2.v[2],};
    return result;
}

MATRIX3 matrix3MakeWihRows(VECTOR3 row0, VECTOR3 row1, VECTOR3 row2) {
    MATRIX3 result = {row0.v[0], row1.v[0], row2.v[0],
            row0.v[1], row1.v[1], row2.v[1],
            row0.v[2], row1.v[2], row2.v[2]};
    return result;
}

MATRIX3 matrix3MakeWithQuaternion(QUATERNION q) {
    q = quaternionNormalize(q);

    float x = q.q[0];
    float y = q.q[1];
    float z = q.q[2];
    float w = q.q[3];

    float _2x = x + x;
    float _2y = y + y;
    float _2z = z + z;
    float _2w = w + w;

    MATRIX3 m = {1.0f - _2y * y - _2z * z,
            _2x * y + _2w * z,
            _2x * z - _2w * y,

            _2x * y - _2w * z,
            1.0f - _2x * x - _2z * z,
            _2y * z + _2w * x,

            _2x * z + _2w * y,
            _2y * z - _2w * x,
            1.0f - _2x * x - _2y * y};

    return m;
}

MATRIX3 matrix3MakeScale(float sx, float sy, float sz) {
    MATRIX3 m = matrix3Identity;
    m.m[0] = sx;
    m.m[4] = sy;
    m.m[8] = sz;
    return m;
}

MATRIX3 matrix3MakeRotation(float radians, float x, float y, float z) {
    VECTOR3 v = vector3Normalize(vector3Make(x, y, z));
    float cos = cosf(radians);
    float cosp = 1.0f - cos;
    float sin = sinf(radians);

    MATRIX3 m = {cos + cosp * v.v[0] * v.v[0],
            cosp * v.v[0] * v.v[1] + v.v[2] * sin,
            cosp * v.v[0] * v.v[2] - v.v[1] * sin,

            cosp * v.v[0] * v.v[1] - v.v[2] * sin,
            cos + cosp * v.v[1] * v.v[1],
            cosp * v.v[1] * v.v[2] + v.v[0] * sin,

            cosp * v.v[0] * v.v[2] + v.v[1] * sin,
            cosp * v.v[1] * v.v[2] - v.v[0] * sin,
            cos + cosp * v.v[2] * v.v[2]};

    return m;
}

MATRIX3 matrix3MakeXRotation(float radians) {
    float cos = cosf(radians);
    float sin = sinf(radians);

    MATRIX3 m = {1.0f, 0.0f, 0.0f,
            0.0f, cos, sin,
            0.0f, -sin, cos};

    return m;
}

MATRIX3 matrix3MakeYRotation(float radians) {
    float cos = cosf(radians);
    float sin = sinf(radians);

    MATRIX3 m = {cos, 0.0f, -sin,
            0.0f, 1.0f, 0.0f,
            sin, 0.0f, cos};

    return m;
}

MATRIX3 matrix3MakeZRotation(float radians) {
    float cos = cosf(radians);
    float sin = sinf(radians);

    MATRIX3 m = {cos, sin, 0.0f,
            -sin, cos, 0.0f,
            0.0f, 0.0f, 1.0f};

    return m;
}

MATRIX3 matrix3SetColumn(MATRIX3 matrix, int column, VECTOR3 vector) {
    matrix.m[column * 3 + 0] = vector.v[0];
    matrix.m[column * 3 + 1] = vector.v[1];
    matrix.m[column * 3 + 2] = vector.v[2];
    return matrix;
}

VECTOR3 matrix3GetColumn(MATRIX3 matrix, int column) {
    VECTOR3 result = {matrix.m[column * 3 + 0], matrix.m[column * 3 + 1], matrix.m[column * 3 + 2]};
    return result;
}

MATRIX3 matrix3SetRow(MATRIX3 matrix, int row, VECTOR3 vector) {
    matrix.m[row] = vector.v[0];
    matrix.m[row + 3] = vector.v[1];
    matrix.m[row + 6] = vector.v[2];

    return matrix;
}

VECTOR3 matrix3GetRow(MATRIX3 matrix, int row) {
    VECTOR3 result = {matrix.m[row], matrix.m[3 + row], matrix.m[6 + row]};
    return result;
}

MATRIX3 matrix3Transpose(MATRIX3 matrix) {
    MATRIX3 m = {matrix.m[0], matrix.m[3], matrix.m[6],
            matrix.m[1], matrix.m[4], matrix.m[7],
            matrix.m[2], matrix.m[5], matrix.m[8]};
    return m;
}

MATRIX3 matrix3Multiply(MATRIX3 matrixLeft, MATRIX3 matrixRight) {
#if defined(__ARM_NEON__)
    MATRIX3 m;
    float32x4x3_t iMatrixLeft;
    float32x4x3_t iMatrixRight;
    float32x4x3_t mm;

    iMatrixLeft.val[0] = vld1q_f32(&matrixLeft.m[0]); // 0 1 2 3
    iMatrixLeft.val[1] = vld1q_f32(&matrixLeft.m[3]); // 3 4 5 6
    iMatrixLeft.val[2] = vld1q_f32(&matrixLeft.m[5]); // 5 6 7 8

    iMatrixRight.val[0] = vld1q_f32(&matrixRight.m[0]); // 0 1 2 3
    iMatrixRight.val[1] = vld1q_f32(&matrixRight.m[3]); // 3 4 5 6
    iMatrixRight.val[2] = vld1q_f32(&matrixRight.m[5]); // 5 6 7 8

    iMatrixLeft.val[2] = vextq_f32(iMatrixLeft.val[2], iMatrixLeft.val[2], 1); // 6 7 8 x

    mm.val[0] = vmulq_n_f32(iMatrixLeft.val[0], vgetq_lane_f32(iMatrixRight.val[0], 0));
    mm.val[1] = vmulq_n_f32(iMatrixLeft.val[0], vgetq_lane_f32(iMatrixRight.val[0], 3));
    mm.val[2] = vmulq_n_f32(iMatrixLeft.val[0], vgetq_lane_f32(iMatrixRight.val[1], 3));

    mm.val[0] = vmlaq_n_f32(mm.val[0], iMatrixLeft.val[1], vgetq_lane_f32(iMatrixRight.val[0], 1));
    mm.val[1] = vmlaq_n_f32(mm.val[1], iMatrixLeft.val[1], vgetq_lane_f32(iMatrixRight.val[1], 1));
    mm.val[2] = vmlaq_n_f32(mm.val[2], iMatrixLeft.val[1], vgetq_lane_f32(iMatrixRight.val[2], 2));

    mm.val[0] = vmlaq_n_f32(mm.val[0], iMatrixLeft.val[2], vgetq_lane_f32(iMatrixRight.val[0], 2));
    mm.val[1] = vmlaq_n_f32(mm.val[1], iMatrixLeft.val[2], vgetq_lane_f32(iMatrixRight.val[1], 2));
    mm.val[2] = vmlaq_n_f32(mm.val[2], iMatrixLeft.val[2], vgetq_lane_f32(iMatrixRight.val[2], 3));

    memcpy(&m.m[0], (char *)&(mm.val[0]), 16);
    memcpy(&m.m[3], (char *)&(mm.val[1]), 16);
    float32x2_t vlow = vget_low_f32(mm.val[2]);
    memcpy(&m.m[6], (char *)&vlow, 8);
    m.m[8] = vgetq_lane_f32(mm.val[2], 2);

    return m;
#elif defined(GLK_SSE3_INTRINSICS)
	struct {
		MATRIX3 m;
		char pad[16*4 - sizeof(MATRIX3)];
	} ret;

    const __m128 iMatrixLeft0 = _mm_loadu_ps(&matrixLeft.m[0]); // 0 1 2 3 // unaligned load
    const __m128 iMatrixLeft1 = _mm_loadu_ps(&matrixLeft.m[3]); // 3 4 5 6 // unaligned load
    const __m128 iMatrixLeft2Tmp = _mm_loadu_ps(&matrixLeft.m[5]); // 5 6 7 8 // unaligned load
    const __m128 iMatrixLeft2 = _mm_shuffle_ps(iMatrixLeft2Tmp, iMatrixLeft2Tmp, _MM_SHUFFLE(0, 3, 2, 1)); // 6 7 8 x

    const __m128 iMatrixRight0 = _mm_loadu_ps(&matrixRight.m[0]);
    const __m128 iMatrixRight1 = _mm_loadu_ps(&matrixRight.m[3]);
    const __m128 iMatrixRight2 = _mm_loadu_ps(&matrixRight.m[5]);

    const __m128 mm0 = iMatrixLeft0 * _mm_shuffle_ps(iMatrixRight0, iMatrixRight0, _MM_SHUFFLE(0, 0, 0, 0))  // mm0 = L0*R0 L1*R0 L2*R0 L3*R0
                     + iMatrixLeft1 * _mm_shuffle_ps(iMatrixRight0, iMatrixRight0, _MM_SHUFFLE(1, 1, 1, 1))  // mm0 = L0*R0+L3*R1 L1*R0+L4*R1 L2*R0+L5*R1 L3*R0+L6*R1
                     + iMatrixLeft2 * _mm_shuffle_ps(iMatrixRight0, iMatrixRight0, _MM_SHUFFLE(2, 2, 2, 2));

    const __m128 mm1 = iMatrixLeft0 * _mm_shuffle_ps(iMatrixRight0, iMatrixRight0, _MM_SHUFFLE(3, 3, 3, 3))  // mm1 = L0*R3 L1*R3 L2*R3 L3*R3
                     + iMatrixLeft1 * _mm_shuffle_ps(iMatrixRight1, iMatrixRight1, _MM_SHUFFLE(1, 1, 1, 1))  // mm1 = L0*R3+L3*R4 L1*R3+L4*R4 L2*R3+L5*R4 L3*R3+
                     + iMatrixLeft2 * _mm_shuffle_ps(iMatrixRight1, iMatrixRight1, _MM_SHUFFLE(2, 2, 2, 2));

    const __m128 mm2 = iMatrixLeft0 * _mm_shuffle_ps(iMatrixRight1, iMatrixRight1, _MM_SHUFFLE(3, 3, 3, 3)) // mm2 = L0*R6 L1*R6 L2*R6 L3*R6
                     + iMatrixLeft1 * _mm_shuffle_ps(iMatrixRight2, iMatrixRight2, _MM_SHUFFLE(2, 2, 2, 2))
                     + iMatrixLeft2 * _mm_shuffle_ps(iMatrixRight2, iMatrixRight2, _MM_SHUFFLE(3, 3, 3, 3));

    _mm_storeu_ps(&ret.m.m[0], mm0); //unaligned store to indices: 0 1 2 3
    _mm_storeu_ps(&ret.m.m[3], mm1); //unaligned store to indices: 3 4 5 6
    _mm_storeu_ps(&ret.m.m[6], mm2); //unaligned store to indices: 6 7 8

    return ret.m;
#else
    MATRIX3 m;

    m.m[0] = matrixLeft.m[0] * matrixRight.m[0] + matrixLeft.m[3] * matrixRight.m[1] + matrixLeft.m[6] * matrixRight.m[2];
    m.m[3] = matrixLeft.m[0] * matrixRight.m[3] + matrixLeft.m[3] * matrixRight.m[4] + matrixLeft.m[6] * matrixRight.m[5];
    m.m[6] = matrixLeft.m[0] * matrixRight.m[6] + matrixLeft.m[3] * matrixRight.m[7] + matrixLeft.m[6] * matrixRight.m[8];

    m.m[1] = matrixLeft.m[1] * matrixRight.m[0] + matrixLeft.m[4] * matrixRight.m[1] + matrixLeft.m[7] * matrixRight.m[2];
    m.m[4] = matrixLeft.m[1] * matrixRight.m[3] + matrixLeft.m[4] * matrixRight.m[4] + matrixLeft.m[7] * matrixRight.m[5];
    m.m[7] = matrixLeft.m[1] * matrixRight.m[6] + matrixLeft.m[4] * matrixRight.m[7] + matrixLeft.m[7] * matrixRight.m[8];

    m.m[2] = matrixLeft.m[2] * matrixRight.m[0] + matrixLeft.m[5] * matrixRight.m[1] + matrixLeft.m[8] * matrixRight.m[2];
    m.m[5] = matrixLeft.m[2] * matrixRight.m[3] + matrixLeft.m[5] * matrixRight.m[4] + matrixLeft.m[8] * matrixRight.m[5];
    m.m[8] = matrixLeft.m[2] * matrixRight.m[6] + matrixLeft.m[5] * matrixRight.m[7] + matrixLeft.m[8] * matrixRight.m[8];

    return m;
#endif
}

MATRIX3 matrix3Add(MATRIX3 matrixLeft, MATRIX3 matrixRight) {
#if defined(GLK_SSE3_INTRINSICS)
    MATRIX3 m;

    _mm_storeu_ps(&m.m[0], _mm_loadu_ps(&matrixLeft.m[0]) + _mm_loadu_ps(&matrixRight.m[0]));
    _mm_storeu_ps(&m.m[4], _mm_loadu_ps(&matrixLeft.m[4]) + _mm_loadu_ps(&matrixRight.m[4]));
    m.m[8] = matrixLeft.m[8] + matrixRight.m[8];

    return m;
#else
    MATRIX3 m;

    m.m[0] = matrixLeft.m[0] + matrixRight.m[0];
    m.m[1] = matrixLeft.m[1] + matrixRight.m[1];
    m.m[2] = matrixLeft.m[2] + matrixRight.m[2];

    m.m[3] = matrixLeft.m[3] + matrixRight.m[3];
    m.m[4] = matrixLeft.m[4] + matrixRight.m[4];
    m.m[5] = matrixLeft.m[5] + matrixRight.m[5];

    m.m[6] = matrixLeft.m[6] + matrixRight.m[6];
    m.m[7] = matrixLeft.m[7] + matrixRight.m[7];
    m.m[8] = matrixLeft.m[8] + matrixRight.m[8];

    return m;
#endif
}

MATRIX3 matrix3Subtract(MATRIX3 matrixLeft, MATRIX3 matrixRight) {
#if defined(GLK_SSE3_INTRINSICS)
    MATRIX3 m;

    _mm_storeu_ps(&m.m[0], _mm_loadu_ps(&matrixLeft.m[0]) - _mm_loadu_ps(&matrixRight.m[0]));
    _mm_storeu_ps(&m.m[4], _mm_loadu_ps(&matrixLeft.m[4]) - _mm_loadu_ps(&matrixRight.m[4]));
    m.m[8] = matrixLeft.m[8] - matrixRight.m[8];

    return m;
#else
    MATRIX3 m;

    m.m[0] = matrixLeft.m[0] - matrixRight.m[0];
    m.m[1] = matrixLeft.m[1] - matrixRight.m[1];
    m.m[2] = matrixLeft.m[2] - matrixRight.m[2];

    m.m[3] = matrixLeft.m[3] - matrixRight.m[3];
    m.m[4] = matrixLeft.m[4] - matrixRight.m[4];
    m.m[5] = matrixLeft.m[5] - matrixRight.m[5];

    m.m[6] = matrixLeft.m[6] - matrixRight.m[6];
    m.m[7] = matrixLeft.m[7] - matrixRight.m[7];
    m.m[8] = matrixLeft.m[8] - matrixRight.m[8];

    return m;
#endif
}

MATRIX3 matrix3Scale(MATRIX3 matrix, float sx, float sy, float sz) {
    MATRIX3 m = {matrix.m[0] * sx, matrix.m[1] * sx, matrix.m[2] * sx,
            matrix.m[3] * sy, matrix.m[4] * sy, matrix.m[5] * sy,
            matrix.m[6] * sz, matrix.m[7] * sz, matrix.m[8] * sz};
    return m;
}

MATRIX3 matrix3ScaleWithVector3(MATRIX3 matrix, VECTOR3 scaleVector) {
    MATRIX3 m = {matrix.m[0] * scaleVector.v[0], matrix.m[1] * scaleVector.v[0], matrix.m[2] * scaleVector.v[0],
            matrix.m[3] * scaleVector.v[1], matrix.m[4] * scaleVector.v[1], matrix.m[5] * scaleVector.v[1],
            matrix.m[6] * scaleVector.v[2], matrix.m[7] * scaleVector.v[2], matrix.m[8] * scaleVector.v[2]};
    return m;
}

MATRIX3 matrix3ScaleWithVector4(MATRIX3 matrix, VECTOR4 scaleVector) {
    MATRIX3 m = {matrix.m[0] * scaleVector.v[0], matrix.m[1] * scaleVector.v[0], matrix.m[2] * scaleVector.v[0],
            matrix.m[3] * scaleVector.v[1], matrix.m[4] * scaleVector.v[1], matrix.m[5] * scaleVector.v[1],
            matrix.m[6] * scaleVector.v[2], matrix.m[7] * scaleVector.v[2], matrix.m[8] * scaleVector.v[2]};
    return m;
}

MATRIX3 matrix3Rotate(MATRIX3 matrix, float radians, float x, float y, float z) {
    MATRIX3 rm = matrix3MakeRotation(radians, x, y, z);
    return matrix3Multiply(matrix, rm);
}

MATRIX3 matrix3RotateWithVector3(MATRIX3 matrix, float radians, VECTOR3 axisVector) {
    MATRIX3 rm = matrix3MakeRotation(radians, axisVector.v[0], axisVector.v[1], axisVector.v[2]);
    return matrix3Multiply(matrix, rm);
}

MATRIX3 matrix3RotateWithVector4(MATRIX3 matrix, float radians, VECTOR4 axisVector) {
    MATRIX3 rm = matrix3MakeRotation(radians, axisVector.v[0], axisVector.v[1], axisVector.v[2]);
    return matrix3Multiply(matrix, rm);
}

MATRIX3 matrix3RotateX(MATRIX3 matrix, float radians) {
    MATRIX3 rm = matrix3MakeXRotation(radians);
    return matrix3Multiply(matrix, rm);
}

MATRIX3 matrix3RotateY(MATRIX3 matrix, float radians) {
    MATRIX3 rm = matrix3MakeYRotation(radians);
    return matrix3Multiply(matrix, rm);
}

MATRIX3 matrix3RotateZ(MATRIX3 matrix, float radians) {
    MATRIX3 rm = matrix3MakeZRotation(radians);
    return matrix3Multiply(matrix, rm);
}

VECTOR3 matrix3MultiplyVector3(MATRIX3 matrixLeft, VECTOR3 vectorRight) {
    VECTOR3 v = {matrixLeft.m[0] * vectorRight.v[0] + matrixLeft.m[3] * vectorRight.v[1] + matrixLeft.m[6] * vectorRight.v[2],
            matrixLeft.m[1] * vectorRight.v[0] + matrixLeft.m[4] * vectorRight.v[1] + matrixLeft.m[7] * vectorRight.v[2],
            matrixLeft.m[2] * vectorRight.v[0] + matrixLeft.m[5] * vectorRight.v[1] + matrixLeft.m[8] * vectorRight.v[2]};
    return v;
}
