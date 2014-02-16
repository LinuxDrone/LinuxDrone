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

#define DR_PI 3.14159274101257f

class CAngle
{
public:
	enum AngleUnit {
		AngleUnit_Degress,
		AngleUnit_Radians
	};

public:
	CAngle();
	CAngle(float value, AngleUnit unit);

	float toRadians() const;
	float toDegress() const;

private:
	float m_valueRad;
};
