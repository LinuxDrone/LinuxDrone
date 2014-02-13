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

#include "CMatrix4f.h"
#include "CVector4f.h"
#include "CVector3f.h"
#include "../system/CSystem.h"
#include "../system/Logger.h"
#include <string.h>
#include <math.h>

//#define __ARM_NEON__
#define HL_DISABLE_SSE_SUPPORT

#if defined __ARM_NEON__
#include <arm_neon.h>
#endif

#ifndef HL_DISABLE_SSE_SUPPORT
#include <immintrin.h>
#include <stdint.h>
#endif

static const size_t alignment = 16;

CMatrix4f::CMatrix4f()
{
	m = (float*)CSystem::alignedAlloc(16 * sizeof(float), alignment /* SSE and Neon required alignment*/);
	memset(m, 0, 16 * sizeof(float));
}

CMatrix4f::CMatrix4f(float m00, float m01, float m02, float m03,
		  float m10, float m11, float m12, float m13,
		  float m20, float m21, float m22, float m23,
		  float m30, float m31, float m32, float m33)
{
	m = (float*) CSystem::alignedAlloc(16 * sizeof(float), alignment /* SSE required alignment*/);
	memset(m, 0, 16 * sizeof(float));

	(*this)[0][0] = m00; (*this)[0][1] = m01; (*this)[0][2] = m02; (*this)[0][3] = m03;
	(*this)[1][0] = m10; (*this)[1][1] = m11; (*this)[1][2] = m12; (*this)[1][3] = m13;
	(*this)[2][0] = m20; (*this)[2][1] = m21; (*this)[2][2] = m22; (*this)[2][3] = m23;
	(*this)[3][0] = m30; (*this)[3][1] = m31; (*this)[3][2] = m32; (*this)[3][3] = m33;
}

CMatrix4f::CMatrix4f(const float* floatArray)
{
	m = (float*) CSystem::alignedAlloc(16 * sizeof(float), alignment /* SSE required alignment*/);
	memcpy(m, floatArray, 16 * sizeof(float));
}

CMatrix4f::CMatrix4f(const CMatrix4f& other)
{
	m = (float*) CSystem::alignedAlloc(16 * sizeof(float), alignment /* SSE required alignment*/);
	memcpy(m, other.m, 16 * sizeof(float));
}

CMatrix4f::~CMatrix4f()
{
	CSystem::alignedFree(m);
	m = 0;
}

CMatrix4f& CMatrix4f::operator=(const CMatrix4f& other)
{
	memcpy(m, other.m, 16 * sizeof(float));
	return *this;
}

CMatrix4f CMatrix4f::scale(float sx, float sy, float sz) const
{
#if defined(__ARM_NEON__)
	float32x4x4_t iMatrix = *(float32x4x4_t *)m;
	float32x4x4_t result;

	result.val[0] = vmulq_n_f32(iMatrix.val[0], (float32_t)sx);
	result.val[1] = vmulq_n_f32(iMatrix.val[1], (float32_t)sy);
	result.val[2] = vmulq_n_f32(iMatrix.val[2], (float32_t)sz);
	result.val[3] = iMatrix.val[3];
	return CMatrix4f((const float*)&result);
#elif !defined(HL_DISABLE_SSE_SUPPORT)
	CMatrix4f matrix;

	_mm_store_ps(&matrix.m[0], _mm_mul_ps(_mm_load_ps(&m[0]), _mm_load1_ps(&sx)));
	_mm_store_ps(&matrix.m[4], _mm_mul_ps(_mm_load_ps(&m[4]), _mm_load1_ps(&sy)));
	_mm_store_ps(&matrix.m[8], _mm_mul_ps(_mm_load_ps(&m[8]), _mm_load1_ps(&sz)));
	_mm_store_ps(&matrix.m[12], _mm_load_ps(&m[12]));

	return matrix;
#else
	CMatrix4f matrix(m[0] * sx, m[1] * sx, m[2] * sx, m[3] * sx,
					 m[4] * sy, m[5] * sy, m[6] * sy, m[7] * sy,
					 m[8] * sz, m[9] * sz, m[10] * sz, m[11] * sz,
					 m[12], m[13], m[14], m[15] );
	return matrix;
#endif
}

CMatrix4f CMatrix4f::translate(float tx, float ty, float tz) const
{
	CMatrix4f mat( m[0], m[1], m[2], m[3],
				  m[4], m[5], m[6], m[7],
				  m[8], m[9], m[10], m[11],
				  m[0] * tx + m[4] * ty + m[8] * tz + m[12],
				  m[1] * tx + m[5] * ty + m[9] * tz + m[13],
				  m[2] * tx + m[6] * ty + m[10] * tz + m[14],
				  m[15] );
	return mat;
}

CMatrix4f CMatrix4f::rotate(const CAngle& angle, float x, float y, float z) const
{
	CMatrix4f rm = CMatrix4f::makeRotation(angle, x, y, z);
	CMatrix4f nm = multiply(rm);
	return nm;
}

CMatrix4f CMatrix4f::add(const CMatrix4f& matrixRight) const
{
#if defined(__ARM_NEON__)
	float32x4x4_t iMatrixLeft = *(float32x4x4_t *)m;
	float32x4x4_t iMatrixRight = *(float32x4x4_t *)matrixRight.m;
	float32x4x4_t result;

	result.val[0] = vaddq_f32(iMatrixLeft.val[0], iMatrixRight.val[0]);
	result.val[1] = vaddq_f32(iMatrixLeft.val[1], iMatrixRight.val[1]);
	result.val[2] = vaddq_f32(iMatrixLeft.val[2], iMatrixRight.val[2]);
	result.val[3] = vaddq_f32(iMatrixLeft.val[3], iMatrixRight.val[3]);

	return CMatrix4f((const float*)&result);
#elif !defined(HL_DISABLE_SSE_SUPPORT)
	CMatrix4f result;

	_mm_store_ps(&result.m[0], _mm_add_ps(_mm_load_ps(m + 0), _mm_load_ps(&matrixRight.m[0])));
	_mm_store_ps(&result.m[4], _mm_add_ps(_mm_load_ps(m + 4), _mm_load_ps(&matrixRight.m[4])));
	_mm_store_ps(&result.m[8], _mm_add_ps(_mm_load_ps(m + 8), _mm_load_ps(&matrixRight.m[8])));
	_mm_store_ps(&result.m[12], _mm_add_ps(_mm_load_ps(m + 12), _mm_load_ps(&matrixRight.m[12])));

	return result;
#else
	CMatrix4f mat;

	mat.m[0] = m[0] + matrixRight.m[0];
	mat.m[1] = m[1] + matrixRight.m[1];
	mat.m[2] = m[2] + matrixRight.m[2];
	mat.m[3] = m[3] + matrixRight.m[3];

	mat.m[4] = m[4] + matrixRight.m[4];
	mat.m[5] = m[5] + matrixRight.m[5];
	mat.m[6] = m[6] + matrixRight.m[6];
	mat.m[7] = m[7] + matrixRight.m[7];

	mat.m[8] = m[8] + matrixRight.m[8];
	mat.m[9] = m[9] + matrixRight.m[9];
	mat.m[10] = m[10] + matrixRight.m[10];
	mat.m[11] = m[11] + matrixRight.m[11];

	mat.m[12] = m[12] + matrixRight.m[12];
	mat.m[13] = m[13] + matrixRight.m[13];
	mat.m[14] = m[14] + matrixRight.m[14];
	mat.m[15] = m[15] + matrixRight.m[15];
	return mat;
#endif
}

CMatrix4f CMatrix4f::substract(const CMatrix4f& matrixRight) const
{
#if defined(__ARM_NEON__)
	float32x4x4_t iMatrixLeft = *(float32x4x4_t *)m;
	float32x4x4_t iMatrixRight = *(float32x4x4_t *)matrixRight.m;
	float32x4x4_t result;

	result.val[0] = vsubq_f32(iMatrixLeft.val[0], iMatrixRight.val[0]);
	result.val[1] = vsubq_f32(iMatrixLeft.val[1], iMatrixRight.val[1]);
	result.val[2] = vsubq_f32(iMatrixLeft.val[2], iMatrixRight.val[2]);
	result.val[3] = vsubq_f32(iMatrixLeft.val[3], iMatrixRight.val[3]);

	return CMatrix4f((const float*)&result);
#elif !defined(HL_DISABLE_SSE_SUPPORT)
	CMatrix4f result;

	_mm_store_ps(result[0], _mm_sub_ps(_mm_load_ps(m + 0),  _mm_load_ps(matrixRight.m + 0)));
	_mm_store_ps(result[4], _mm_sub_ps(_mm_load_ps(m + 4),  _mm_load_ps(matrixRight.m + 4)));
	_mm_store_ps(result[8], _mm_sub_ps(_mm_load_ps(m + 8),  _mm_load_ps(matrixRight.m + 8)));
	_mm_store_ps(result[12],_mm_sub_ps(_mm_load_ps(m + 12),  _mm_load_ps(matrixRight.m + 12)));

	return result;
#else
	CMatrix4f mat;

	mat.m[0] = m[0] - matrixRight.m[0];
	mat.m[1] = m[1] - matrixRight.m[1];
	mat.m[2] = m[2] - matrixRight.m[2];
	mat.m[3] = m[3] - matrixRight.m[3];

	mat.m[4] = m[4] - matrixRight.m[4];
	mat.m[5] = m[5] - matrixRight.m[5];
	mat.m[6] = m[6] - matrixRight.m[6];
	mat.m[7] = m[7] - matrixRight.m[7];

	mat.m[8] = m[8] - matrixRight.m[8];
	mat.m[9] = m[9] - matrixRight.m[9];
	mat.m[10] = m[10] - matrixRight.m[10];
	mat.m[11] = m[11] - matrixRight.m[11];

	mat.m[12] = m[12] - matrixRight.m[12];
	mat.m[13] = m[13] - matrixRight.m[13];
	mat.m[14] = m[14] - matrixRight.m[14];
	mat.m[15] = m[15] - matrixRight.m[15];
	return mat;
#endif
}

CMatrix4f CMatrix4f::multiply(const CMatrix4f& matrixRight) const
{
#if defined(__ARM_NEON__)
	float32x4x4_t iMatrixLeft = *(float32x4x4_t *)m;
	float32x4x4_t iMatrixRight = *(float32x4x4_t *)matrixRight.m;
	float32x4x4_t result;

	result.val[0] = vmulq_n_f32(iMatrixLeft.val[0], vgetq_lane_f32(iMatrixRight.val[0], 0));
	result.val[1] = vmulq_n_f32(iMatrixLeft.val[0], vgetq_lane_f32(iMatrixRight.val[1], 0));
	result.val[2] = vmulq_n_f32(iMatrixLeft.val[0], vgetq_lane_f32(iMatrixRight.val[2], 0));
	result.val[3] = vmulq_n_f32(iMatrixLeft.val[0], vgetq_lane_f32(iMatrixRight.val[3], 0));

	result.val[0] = vmlaq_n_f32(result.val[0], iMatrixLeft.val[1], vgetq_lane_f32(iMatrixRight.val[0], 1));
	result.val[1] = vmlaq_n_f32(result.val[1], iMatrixLeft.val[1], vgetq_lane_f32(iMatrixRight.val[1], 1));
	result.val[2] = vmlaq_n_f32(result.val[2], iMatrixLeft.val[1], vgetq_lane_f32(iMatrixRight.val[2], 1));
	result.val[3] = vmlaq_n_f32(result.val[3], iMatrixLeft.val[1], vgetq_lane_f32(iMatrixRight.val[3], 1));

	result.val[0] = vmlaq_n_f32(result.val[0], iMatrixLeft.val[2], vgetq_lane_f32(iMatrixRight.val[0], 2));
	result.val[1] = vmlaq_n_f32(result.val[1], iMatrixLeft.val[2], vgetq_lane_f32(iMatrixRight.val[1], 2));
	result.val[2] = vmlaq_n_f32(result.val[2], iMatrixLeft.val[2], vgetq_lane_f32(iMatrixRight.val[2], 2));
	result.val[3] = vmlaq_n_f32(result.val[3], iMatrixLeft.val[2], vgetq_lane_f32(iMatrixRight.val[3], 2));

	result.val[0] = vmlaq_n_f32(result.val[0], iMatrixLeft.val[3], vgetq_lane_f32(iMatrixRight.val[0], 3));
	result.val[1] = vmlaq_n_f32(result.val[1], iMatrixLeft.val[3], vgetq_lane_f32(iMatrixRight.val[1], 3));
	result.val[2] = vmlaq_n_f32(result.val[2], iMatrixLeft.val[3], vgetq_lane_f32(iMatrixRight.val[2], 3));
	result.val[3] = vmlaq_n_f32(result.val[3], iMatrixLeft.val[3], vgetq_lane_f32(iMatrixRight.val[3], 3));

	return CMatrix4f((const float*)&result);
#elif !defined(HL_DISABLE_SSE_SUPPORT)
	const __m128 l0 = _mm_load_ps(m + 0);
	const __m128 l1 = _mm_load_ps(m + 4);
	const __m128 l2 = _mm_load_ps(m + 8);
	const __m128 l3 = _mm_load_ps(m + 12);

	const __m128 r0 = _mm_load_ps(matrixRight.m + 0);
	const __m128 r1 = _mm_load_ps(matrixRight.m + 4);
	const __m128 r2 = _mm_load_ps(matrixRight.m + 8);
	const __m128 r3 = _mm_load_ps(matrixRight.m + 12);

	const __m128 m0 = _mm_add_ps(
		_mm_add_ps(
		_mm_mul_ps(l0, _mm_shuffle_ps(r0, r0, _MM_SHUFFLE(0, 0, 0, 0))),
		_mm_mul_ps(l1, _mm_shuffle_ps(r0, r0, _MM_SHUFFLE(1, 1, 1, 1)))),
		_mm_add_ps(
		_mm_mul_ps(l2, _mm_shuffle_ps(r0, r0, _MM_SHUFFLE(2, 2, 2, 2))),
		_mm_mul_ps(l3, _mm_shuffle_ps(r0, r0, _MM_SHUFFLE(3, 3, 3, 3)))));

	const __m128 m1 = _mm_add_ps(
		_mm_add_ps(
		_mm_mul_ps(l0, _mm_shuffle_ps(r1, r1, _MM_SHUFFLE(0, 0, 0, 0))),
		_mm_mul_ps(l1, _mm_shuffle_ps(r1, r1, _MM_SHUFFLE(1, 1, 1, 1)))),
		_mm_add_ps(
		_mm_mul_ps(l2, _mm_shuffle_ps(r1, r1, _MM_SHUFFLE(2, 2, 2, 2))),
		_mm_mul_ps(l3, _mm_shuffle_ps(r1, r1, _MM_SHUFFLE(3, 3, 3, 3)))));

	const __m128 m2 = _mm_add_ps(
		_mm_add_ps(
		_mm_mul_ps(l0, _mm_shuffle_ps(r2, r2, _MM_SHUFFLE(0, 0, 0, 0))),
		_mm_mul_ps(l1, _mm_shuffle_ps(r2, r2, _MM_SHUFFLE(1, 1, 1, 1)))),
		_mm_add_ps(
		_mm_mul_ps(l2, _mm_shuffle_ps(r2, r2, _MM_SHUFFLE(2, 2, 2, 2))),
		_mm_mul_ps(l3, _mm_shuffle_ps(r2, r2, _MM_SHUFFLE(3, 3, 3, 3)))));

	const __m128 m3 = _mm_add_ps(
		_mm_add_ps(
		_mm_mul_ps(l0, _mm_shuffle_ps(r3, r3, _MM_SHUFFLE(0, 0, 0, 0))),
		_mm_mul_ps(l1, _mm_shuffle_ps(r3, r3, _MM_SHUFFLE(1, 1, 1, 1)))),
		_mm_add_ps(
		_mm_mul_ps(l2, _mm_shuffle_ps(r3, r3, _MM_SHUFFLE(2, 2, 2, 2))),
		_mm_mul_ps(l3, _mm_shuffle_ps(r3, r3, _MM_SHUFFLE(3, 3, 3, 3)))));

	CMatrix4f m;
	_mm_store_ps(&m.m[0], m0);
	_mm_store_ps(&m.m[4], m1);
	_mm_store_ps(&m.m[8], m2);
	_mm_store_ps(&m.m[12], m3);
	return m;
#else
	CMatrix4f mat;

	mat[0][0] = (*this)[0][0] * matrixRight[0][0] + (*this)[1][0] * matrixRight[0][1] + (*this)[2][0] * matrixRight[0][2] + (*this)[3][0] * matrixRight[0][3];
	mat[1][0] = (*this)[0][0] * matrixRight[1][0] + (*this)[1][0] * matrixRight[1][1] + (*this)[2][0] * matrixRight[1][2] + (*this)[3][0] * matrixRight[1][3];
	mat[2][0] = (*this)[0][0] * matrixRight[2][0] + (*this)[1][0] * matrixRight[2][1] + (*this)[2][0] * matrixRight[2][2] + (*this)[3][0] * matrixRight[2][3];
	mat[3][0] = (*this)[0][0] * matrixRight[3][0] + (*this)[1][0] * matrixRight[3][1] + (*this)[2][0] * matrixRight[3][2] + (*this)[3][0] * matrixRight[3][3];

	mat[0][1] = (*this)[0][1] * matrixRight[0][0] + (*this)[1][1] * matrixRight[0][1] + (*this)[2][1] * matrixRight[0][2] + (*this)[3][1] * matrixRight[0][3];
	mat[1][1] = (*this)[0][1] * matrixRight[1][0] + (*this)[1][1] * matrixRight[1][1] + (*this)[2][1] * matrixRight[1][2] + (*this)[3][1] * matrixRight[1][3];
	mat[2][1] = (*this)[0][1] * matrixRight[2][0] + (*this)[1][1] * matrixRight[2][1] + (*this)[2][1] * matrixRight[2][2] + (*this)[3][1] * matrixRight[2][3];
	mat[3][1] = (*this)[0][1] * matrixRight[3][0] + (*this)[1][1] * matrixRight[3][1] + (*this)[2][1] * matrixRight[3][2] + (*this)[3][1] * matrixRight[3][3];

	mat[0][2] = (*this)[0][2] * matrixRight[0][0] + (*this)[1][2] * matrixRight[0][1] + (*this)[2][2] * matrixRight[0][2] + (*this)[3][2] * matrixRight[0][3];
	mat[1][2] = (*this)[0][2] * matrixRight[1][0] + (*this)[1][2] * matrixRight[1][1] + (*this)[2][2] * matrixRight[1][2] + (*this)[3][2] * matrixRight[1][3];
	mat[2][2] = (*this)[0][2] * matrixRight[2][0] + (*this)[1][2] * matrixRight[2][1] + (*this)[2][2] * matrixRight[2][2] + (*this)[3][2] * matrixRight[2][3];
	mat[3][2] = (*this)[0][2] * matrixRight[3][0] + (*this)[1][2] * matrixRight[3][1] + (*this)[2][2] * matrixRight[3][2] + (*this)[3][2] * matrixRight[3][3];

	mat[0][3] = (*this)[0][3] * matrixRight[0][0] + (*this)[1][3] * matrixRight[0][1] + (*this)[2][3] * matrixRight[0][2] + (*this)[3][3] * matrixRight[0][3];
	mat[1][3] = (*this)[0][3] * matrixRight[1][0] + (*this)[1][3] * matrixRight[1][1] + (*this)[2][3] * matrixRight[1][2] + (*this)[3][3] * matrixRight[1][3];
	mat[2][3] = (*this)[0][3] * matrixRight[2][0] + (*this)[1][3] * matrixRight[2][1] + (*this)[2][3] * matrixRight[2][2] + (*this)[3][3] * matrixRight[2][3];
	mat[3][3] = (*this)[0][3] * matrixRight[3][0] + (*this)[1][3] * matrixRight[3][1] + (*this)[2][3] * matrixRight[3][2] + (*this)[3][3] * matrixRight[3][3];

	return mat;
#endif
}

CVector4f CMatrix4f::multiply(const CVector4f& vectorRight) const
{
#if defined(__ARM_NEON__)
	float32x4x4_t iMatrix = *(float32x4x4_t *)m;
	float32x4_t v;

	iMatrix.val[0] = vmulq_n_f32(iMatrix.val[0], (float32_t)vectorRight.v[0]);
	iMatrix.val[1] = vmulq_n_f32(iMatrix.val[1], (float32_t)vectorRight.v[1]);
	iMatrix.val[2] = vmulq_n_f32(iMatrix.val[2], (float32_t)vectorRight.v[2]);
	iMatrix.val[3] = vmulq_n_f32(iMatrix.val[3], (float32_t)vectorRight.v[3]);

	iMatrix.val[0] = vaddq_f32(iMatrix.val[0], iMatrix.val[1]);
	iMatrix.val[2] = vaddq_f32(iMatrix.val[2], iMatrix.val[3]);

	v = vaddq_f32(iMatrix.val[0], iMatrix.val[2]);
	return CVector4f((const float*)&v);
#elif !defined(HL_DISABLE_SSE_SUPPORT)
	__m128 v;
	if ((size_t)(void*)(&vectorRight.v[0]) % 16 != 0)
		v = _mm_loadu_ps(&vectorRight.v[0]); // not aligned
	else
		v = _mm_load_ps(&vectorRight.v[0]); // aligned

	const __m128 r = _mm_add_ps(
	_mm_add_ps(_mm_mul_ps(_mm_load_ps(m + 0), _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0))),
			   _mm_mul_ps(_mm_load_ps(m + 4), _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1)))),
	_mm_add_ps(_mm_mul_ps(_mm_load_ps(m + 8), _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2))),
			   _mm_mul_ps(_mm_load_ps(m + 12), _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3)))));

	CVector4f ret;
	_mm_store_ps(&(ret.v[0]), r);
	return ret;
#else
	float x = (*this)[0][0] * vectorRight.v[0] + (*this)[1][0] * vectorRight.v[1] + (*this)[2][0] * vectorRight.v[2] + (*this)[3][0] * vectorRight.v[3];
	float y = (*this)[0][1] * vectorRight.v[0] + (*this)[1][1] * vectorRight.v[1] + (*this)[2][1] * vectorRight.v[2] + (*this)[3][1] * vectorRight.v[3];
	float z = (*this)[0][2] * vectorRight.v[0] + (*this)[1][2] * vectorRight.v[1] + (*this)[2][2] * vectorRight.v[2] + (*this)[3][2] * vectorRight.v[3];
	float w = (*this)[0][3] * vectorRight.v[0] + (*this)[1][3] * vectorRight.v[1] + (*this)[2][3] * vectorRight.v[2] + (*this)[3][3] * vectorRight.v[3];
	CVector4f v = CVector4f(x, y, z, w);
	return v;
#endif
}

CMatrix4f CMatrix4f::transpose() const
{
	CMatrix4f matrix((*this)[0][0], (*this)[1][0], (*this)[2][0], (*this)[3][0],
					 (*this)[0][1], (*this)[1][1], (*this)[2][1], (*this)[3][1],
					 (*this)[0][2], (*this)[1][2], (*this)[2][2], (*this)[3][2],
					 (*this)[0][3], (*this)[1][3], (*this)[2][3], (*this)[3][3]);

	return matrix;
}

CMatrix4f CMatrix4f::identity()
{
	CMatrix4f m(1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
	return m;
}

CMatrix4f CMatrix4f::makeRotation(const CAngle& angle, float x, float y, float z)
{
	CVector3f v = CVector3f::normalize(CVector3f(x, y, z));
	float cos = cosf(angle.toRadians());
	float cosp = 1.0f - cos;
	float sin = sinf(angle.toRadians());

	float m[] = { cos + cosp * v.v[0] * v.v[0],
		cosp * v.v[0] * v.v[1] + v.v[2] * sin,
		cosp * v.v[0] * v.v[2] - v.v[1] * sin,
		0.0f,
		cosp * v.v[0] * v.v[1] - v.v[2] * sin,
		cos + cosp * v.v[1] * v.v[1],
		cosp * v.v[1] * v.v[2] + v.v[0] * sin,
		0.0f,
		cosp * v.v[0] * v.v[2] + v.v[1] * sin,
		cosp * v.v[1] * v.v[2] - v.v[0] * sin,
		cos + cosp * v.v[2] * v.v[2],
		0.0f,
		0.0f,
		0.0f,
		0.0f,
		1.0f };
	CMatrix4f mat(m);
	return mat;
}

CMatrix4f CMatrix4f::makeRotationX(const CAngle& angle)
{
	float cos = cosf(angle.toRadians());
	float sin = sinf(angle.toRadians());

	float m[] = { 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, cos, sin, 0.0f,
		0.0f, -sin, cos, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f };

	CMatrix4f mat(m);
	return mat;
}

CMatrix4f CMatrix4f::makeRotationY(const CAngle& angle)
{
	float cos = cosf(angle.toRadians());
	float sin = sinf(angle.toRadians());

	float m[] = { cos, 0.0f, -sin, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		sin, 0.0f, cos, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f };

	CMatrix4f mat(m);
	return mat;
}

CMatrix4f CMatrix4f::makeRotationZ(const CAngle& angle)
{
	float cos = cosf(angle.toRadians());
	float sin = sinf(angle.toRadians());

	float m[] = { cos, sin, 0.0f, 0.0f,
		-sin, cos, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f };

	CMatrix4f mat(m);
	return mat;
}

CMatrix4f CMatrix4f::makeTranslate(float tx, float ty, float tz)
{
	CMatrix4f m = CMatrix4f::identity();
	m.m[12] = tx;
	m.m[13] = ty;
	m.m[14] = tz;
	return m;
}

CMatrix4f CMatrix4f::makeScale(float sx, float sy, float sz)
{
	CMatrix4f m = CMatrix4f::identity();
	m.m[0] = sx;
	m.m[5] = sy;
	m.m[10] = sz;
	return m;
}

CMatrix4f CMatrix4f::operator*(const CMatrix4f& right) const
{
	return multiply(right);
}
