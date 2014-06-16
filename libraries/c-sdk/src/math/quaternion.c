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

#include <math.h>
#include "../../include/math/quaternion.h"

#ifdef __ARM_NEON__
#include <arm_neon.h>
#endif // __ARM_NEON__

QUATERNION quaternionMake(float x, float y, float z, float w) {
    QUATERNION result = {x, y, z, w};
    return result;
}

QUATERNION quaternionMakeWithArray(float values[4]) {
    QUATERNION result = {values[0], values[1], values[2], values[3]};
    return result;
}

QUATERNION quaternionMakeWithVector3(VECTOR3 v, float scalar) {
    QUATERNION result = {v.v[0], v.v[1], v.v[2], scalar};
    return result;
}

QUATERNION quaternionMakeWithAngleAndAxis(float radians, float x, float y, float z) {
    float halfAngle = radians * 0.5f;
    float scale = sinf(halfAngle);
    QUATERNION q = { scale * x, scale * y, scale * z, cosf(halfAngle) };
    return q;
}

QUATERNION quaternionMakeWithAngleAndVector3Axis(float radians, VECTOR3 axisVector) {
    return quaternionMakeWithAngleAndAxis(radians, axisVector.v[0], axisVector.v[1], axisVector.v[2]);
}

QUATERNION quaternionAdd(QUATERNION quaternionLeft, QUATERNION quaternionRight) {
#if defined(__ARM_NEON__)
    float32x4_t v = vaddq_f32(*(float32x4_t *)&quaternionLeft,
                              *(float32x4_t *)&quaternionRight);
    return *(QUATERNION*)&v;
#elif defined(GLK_SSE3_INTRINSICS)
    __m128 v = _mm_load_ps(&quaternionLeft.q[0]) + _mm_load_ps(&quaternionRight.q[0]);
    return *(QUATERNION*)&v;
#else
    QUATERNION q = { quaternionLeft.q[0] + quaternionRight.q[0],
            quaternionLeft.q[1] + quaternionRight.q[1],
            quaternionLeft.q[2] + quaternionRight.q[2],
            quaternionLeft.q[3] + quaternionRight.q[3] };
    return q;
#endif
}

QUATERNION quaternionSubtract(QUATERNION quaternionLeft, QUATERNION quaternionRight) {
#if defined(__ARM_NEON__)
    float32x4_t v = vsubq_f32(*(float32x4_t *)&quaternionLeft,
                              *(float32x4_t *)&quaternionRight);
    return *(QUATERNION *)&v;
#elif defined(GLK_SSE3_INTRINSICS)
    __m128 v = _mm_load_ps(&quaternionLeft.q[0]) - _mm_load_ps(&quaternionRight.q[0]);
    return *(QUATERNION *)&v;
#else
    QUATERNION q = { quaternionLeft.q[0] - quaternionRight.q[0],
            quaternionLeft.q[1] - quaternionRight.q[1],
            quaternionLeft.q[2] - quaternionRight.q[2],
            quaternionLeft.q[3] - quaternionRight.q[3] };
    return q;
#endif
}

QUATERNION quaternionMultiply(QUATERNION quaternionLeft, QUATERNION quaternionRight) {
#if defined(GLK_SSE3_INTRINSICS)
	const __m128 ql = _mm_load_ps(&quaternionLeft.q[0]);
	const __m128 qr = _mm_load_ps(&quaternionRight.q[0]);

	const __m128 ql3012 = _mm_shuffle_ps(ql, ql, _MM_SHUFFLE(2, 1, 0, 3));
	const __m128 ql3120 = _mm_shuffle_ps(ql, ql, _MM_SHUFFLE(0, 2, 1, 3));
	const __m128 ql3201 = _mm_shuffle_ps(ql, ql, _MM_SHUFFLE(1, 0, 2, 3));

	const __m128 qr0321 = _mm_shuffle_ps(qr, qr, _MM_SHUFFLE(1, 2, 3, 0));
	const __m128 qr1302 = _mm_shuffle_ps(qr, qr, _MM_SHUFFLE(2, 0, 3, 1));
	const __m128 qr2310 = _mm_shuffle_ps(qr, qr, _MM_SHUFFLE(0, 1, 3, 2));
	const __m128 qr3012 = _mm_shuffle_ps(qr, qr, _MM_SHUFFLE(2, 1, 0, 3));

    uint32_t signBit = 0x80000000;
    uint32_t zeroBit = 0x0;
    uint32_t __attribute__((aligned(16))) mask0001[4] = {zeroBit, zeroBit, zeroBit, signBit};
    uint32_t __attribute__((aligned(16))) mask0111[4] = {zeroBit, signBit, signBit, signBit};
    const __m128 m0001 = _mm_load_ps((float *)mask0001);
    const __m128 m0111 = _mm_load_ps((float *)mask0111);

	const __m128 aline = ql3012 * _mm_xor_ps(qr0321, m0001);
	const __m128 bline = ql3120 * _mm_xor_ps(qr1302, m0001);
	const __m128 cline = ql3201 * _mm_xor_ps(qr2310, m0001);
	const __m128 dline = ql3012 * _mm_xor_ps(qr3012, m0111);
	const __m128 r = _mm_hadd_ps(_mm_hadd_ps(aline, bline), _mm_hadd_ps(cline, dline));

    return *(QUATERNION *)&r;
#else

    QUATERNION q = { quaternionLeft.q[3] * quaternionRight.q[0] +
            quaternionLeft.q[0] * quaternionRight.q[3] +
            quaternionLeft.q[1] * quaternionRight.q[2] -
            quaternionLeft.q[2] * quaternionRight.q[1],

            quaternionLeft.q[3] * quaternionRight.q[1] +
            quaternionLeft.q[1] * quaternionRight.q[3] +
            quaternionLeft.q[2] * quaternionRight.q[0] -
            quaternionLeft.q[0] * quaternionRight.q[2],

            quaternionLeft.q[3] * quaternionRight.q[2] +
            quaternionLeft.q[2] * quaternionRight.q[3] +
            quaternionLeft.q[0] * quaternionRight.q[1] -
            quaternionLeft.q[1] * quaternionRight.q[0],

            quaternionLeft.q[3] * quaternionRight.q[3] -
            quaternionLeft.q[0] * quaternionRight.q[0] -
            quaternionLeft.q[1] * quaternionRight.q[1] -
            quaternionLeft.q[2] * quaternionRight.q[2] };
    return q;
#endif
}

float quaternionLength(QUATERNION quaternion) {
#if defined(__ARM_NEON__)
    float32x4_t v = vmulq_f32(*(float32x4_t *)&quaternion,
                              *(float32x4_t *)&quaternion);
    float32x2_t v2 = vpadd_f32(vget_low_f32(v), vget_high_f32(v));
    v2 = vpadd_f32(v2, v2);
    return sqrt(vget_lane_f32(v2, 0));
#elif defined(GLK_SSE3_INTRINSICS)
	const __m128 q = _mm_load_ps(&quaternion.q[0]);
	const __m128 product = q * q;
	const __m128 halfsum = _mm_hadd_ps(product, product);
	return _mm_cvtss_f32(_mm_sqrt_ss(_mm_hadd_ps(halfsum, halfsum)));
#else
    return (float) sqrt(quaternion.q[0] * quaternion.q[0] +
                quaternion.q[1] * quaternion.q[1] +
                quaternion.q[2] * quaternion.q[2] +
                quaternion.q[3] * quaternion.q[3]);
#endif
}

QUATERNION quaternionConjugate(QUATERNION quaternion) {
#if defined(__ARM_NEON__)
    float32x4_t *q = (float32x4_t *)&quaternion;

    uint32_t signBit = 0x80000000;
    uint32_t zeroBit = 0x0;
    uint32x4_t mask = vdupq_n_u32(signBit);
    mask = vsetq_lane_u32(zeroBit, mask, 3);
    *q = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(*q), mask));

    return *(QUATERNION *)q;
#elif defined(GLK_SSE3_INTRINSICS)
    // Multiply first three elements by -1
    const uint32_t signBit = 0x80000000;
    const uint32_t zeroBit = 0x0;
    const uint32_t __attribute__((aligned(16))) mask[4] = {signBit, signBit, signBit, zeroBit};
    __m128 v_mask = _mm_load_ps((float *)mask);
	const __m128 q = _mm_load_ps(&quaternion.q[0]);
    __m128 v = _mm_xor_ps(q, v_mask);

    return *(QUATERNION *)&v;
#else
    QUATERNION q = { -quaternion.q[0], -quaternion.q[1], -quaternion.q[2], quaternion.q[3] };
    return q;
#endif
}

QUATERNION quaternionInvert(QUATERNION quaternion) {
#if defined(__ARM_NEON__)
    float32x4_t *q = (float32x4_t *)&quaternion;
    float32x4_t v = vmulq_f32(*q, *q);
    float32x2_t v2 = vpadd_f32(vget_low_f32(v), vget_high_f32(v));
    v2 = vpadd_f32(v2, v2);
    float32_t scale = 1.0f / vget_lane_f32(v2, 0);
    v = vmulq_f32(*q, vdupq_n_f32(scale));

    uint32_t signBit = 0x80000000;
    uint32_t zeroBit = 0x0;
    uint32x4_t mask = vdupq_n_u32(signBit);
    mask = vsetq_lane_u32(zeroBit, mask, 3);
    v = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(v), mask));

    return *(QUATERNION *)&v;
#elif defined(GLK_SSE3_INTRINSICS)
	const __m128 q = _mm_load_ps(&quaternion.q[0]);
    const uint32_t signBit = 0x80000000;
    const uint32_t zeroBit = 0x0;
    const uint32_t __attribute__((aligned(16))) mask[4] = {signBit, signBit, signBit, zeroBit};
    const __m128 v_mask = _mm_load_ps((float *)mask);
	const __m128 product = q * q;
	const __m128 halfsum = _mm_hadd_ps(product, product);
	const __m128 v = _mm_xor_ps(q, v_mask) / _mm_hadd_ps(halfsum, halfsum);
    return *(QUATERNION *)&v;
#else
    float scale = 1.0f / (quaternion.q[0] * quaternion.q[0] +
            quaternion.q[1] * quaternion.q[1] +
            quaternion.q[2] * quaternion.q[2] +
            quaternion.q[3] * quaternion.q[3]);
    QUATERNION q = { -quaternion.q[0] * scale, -quaternion.q[1] * scale, -quaternion.q[2] * scale, quaternion.q[3] * scale };
    return q;
#endif
}

QUATERNION quaternionNormalize(QUATERNION quaternion) {
    float scale = 1.0f / quaternionLength(quaternion);
#if defined(__ARM_NEON__)
    float32x4_t v = vmulq_f32(*(float32x4_t *)&quaternion,
                              vdupq_n_f32((float32_t)scale));
    return *(QUATERNION *)&v;
#elif defined(GLK_SSE3_INTRINSICS)
	const __m128 q = _mm_load_ps(&quaternion.q[0]);
    __m128 v = q * _mm_set1_ps(scale);
    return *(QUATERNION *)&v;
#else
    QUATERNION q = { quaternion.q[0] * scale, quaternion.q[1] * scale, quaternion.q[2] * scale, quaternion.q[3] * scale };
    return q;
#endif
}

VECTOR3 quaternionRotateVector3(QUATERNION quaternion, VECTOR3 vector) {
    QUATERNION rotatedQuaternion = quaternionMake(vector.v[0], vector.v[1], vector.v[2], 0.0f);
    rotatedQuaternion = quaternionMultiply(quaternionMultiply(quaternion, rotatedQuaternion), quaternionInvert(quaternion));

    return vector3Make(rotatedQuaternion.q[0], rotatedQuaternion.q[1], rotatedQuaternion.q[2]);
}

VECTOR4 quaternionRotateVector4(QUATERNION quaternion, VECTOR4 vector) {
    QUATERNION rotatedQuaternion = quaternionMake(vector.v[0], vector.v[1], vector.v[2], 0.0f);
    rotatedQuaternion = quaternionMultiply(quaternionMultiply(quaternion, rotatedQuaternion), quaternionInvert(quaternion));

    return vector4Make(rotatedQuaternion.q[0], rotatedQuaternion.q[1], rotatedQuaternion.q[2], vector.v[3]);
}
