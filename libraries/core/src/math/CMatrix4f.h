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

#include "CAngle.h"
#include "CVector4f.h"
#include <assert.h>

class CMatrix4f
{
public:
	CMatrix4f();
	CMatrix4f(float m00, float m01, float m02, float m03,
			  float m10, float m11, float m12, float m13,
			  float m20, float m21, float m22, float m23,
			  float m30, float m31, float m32, float m33);
	explicit CMatrix4f(const float* floatArray);
	CMatrix4f(const CMatrix4f& other);
	~CMatrix4f();

	CMatrix4f& operator=(const CMatrix4f& other);

	CMatrix4f scale(float sx, float sy, float sz) const;
	CMatrix4f translate(float tx, float ty, float tz) const;
	CMatrix4f rotate(const CAngle& angle, float x, float y, float z) const;

	CMatrix4f add(const CMatrix4f& matrixRight) const;
	CMatrix4f substract(const CMatrix4f& matrixRight) const;
	CMatrix4f multiply(const CMatrix4f& matrixRight) const;
	CVector4f multiply(const CVector4f& vectorRight) const;
	CMatrix4f transpose() const;

	static CMatrix4f identity();
	static CMatrix4f makeRotation(const CAngle& angle, float x, float y, float z);
	static CMatrix4f makeRotationX(const CAngle& angle);
	static CMatrix4f makeRotationY(const CAngle& angle);
	static CMatrix4f makeRotationZ(const CAngle& angle);
	static CMatrix4f makeTranslate(float tx, float ty, float tz);
	static CMatrix4f makeScale(float sx, float sy, float sz);

	CMatrix4f operator*(const CMatrix4f& right) const;

	float* operator[] (int index) {assert(index < 4 && index >= 0); return &m[0] + index*4;}
	const float* operator[] (int index) const {assert(index < 4 && index >= 0); return &m[0] + index*4;}

public:
	float * m;
};

inline CVector4f operator * (const CMatrix4f& matrix, const CVector4f& v)
{
	return matrix.multiply(v);
}

inline CVector4f operator * (const CVector4f& v, const CMatrix4f& matrix)
{
	return matrix.multiply(v);
}
