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

class CVector4f
{
public:
	CVector4f() {
		v[0] = v[1] = v[2] = v[3] = 0.0f;
	}
	CVector4f(const float x, const float y, const float z, const float w) {
		v[0] = x; v[1] = y; v[2] = z; v[3] = w;
	}

	void operator +=(const CVector4f& other) { v[0] = other.v[0]; v[1] = other.v[1]; v[2] = other.v[2]; v[3] = other.v[3]; }
	void operator +=(const float val) { v[0] = val; v[1] = val; v[2] = val; v[3] = val; }

public:
	float v[4] __attribute__((aligned(16)));
};
