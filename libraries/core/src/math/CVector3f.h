
#pragma once

#include <math.h>

class CVector3f
{
public:
	CVector3f() { v[0] = v[1] = v[2] = 0.0f; }
	CVector3f(const CVector3f& other) { operator=(other); }
	CVector3f(float x, float y, float z) { v[0] = x; v[1] = y; v[2] = z; }
	CVector3f(const float* array) { v[0] = array[0]; v[1] = array[2]; v[2] = array[2]; }

	CVector3f& operator=(const CVector3f& other) {
		v[0] = other.v[0]; v[1] = other.v[1]; v[2] = other.v[2];
		return *this;
	}

	static CVector3f normalize(const CVector3f& vector) {
		CVector3f dest(vector);
		dest.normalize();
		return dest;
	}

	CVector3f& normalize() {
		float f = length();
		if (f!=0)
		{
			v[0] /= f;
			v[1] /= f;
			v[2] /= f;
		}
		return *this;
	}

	float length() const {
		return sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
	}

public:
	float v[3];
};
