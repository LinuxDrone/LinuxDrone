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

#include "CAngle.h"

CAngle::CAngle()
{
	m_valueRad = 0.0f;
}

CAngle::CAngle(float value, AngleUnit unit)
{
	if (unit == AngleUnit_Radians) {
		m_valueRad = value;
	} else {
		m_valueRad = value * float (DR_PI) / 180.0f;
	}
}

float CAngle::toRadians() const
{
	return m_valueRad;
}

float CAngle::toDegress() const
{
	return m_valueRad * 180.0f / float(DR_PI);
}
