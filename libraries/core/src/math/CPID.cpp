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

#include "CPID.h"

CPID::CPID()
	: 	m_Kp(1),
	  	m_Ki(0),
	  	m_Kd(0),
	  	m_dT(0),
	  	m_iLimit(0),
	  	m_iSum(0),
	  	m_lastError(0)
{

}

CPID::CPID(double Kp, double Ki, double Kd, double iLimit)
	: 	m_Kp(Kp),
	  	m_Ki(Ki),
	  	m_Kd(Kd),
	  	m_iLimit(iLimit),
		m_dT(0),
		m_iSum(0),
		m_lastError(0)
{

}

CPID::~CPID()
{

}

double CPID::calcPID(double error, double dT)
{
	// Сalculation of the integral unit
	m_iSum += m_Ki * error * dT;

	// Limit on the magnitude of the integral
	if (m_iSum > m_iLimit) {
		m_iSum = m_iLimit;
	} else if (m_iSum < -m_iLimit) {
		m_iSum = -m_iLimit;
	}

	// Calculation of the differential unit
	double diff = m_Kd * (error - m_lastError) / dT;
	m_lastError = error;

	return (m_Kp * error) + m_iSum + diff;
}

// Parameter setting
void CPID::setPID(double Kp, double Ki, double Kd, double iLimit)
{
	m_Kp = Kp;
	m_Ki = Ki;
	m_Kd = Kd;
	m_iLimit = iLimit;
}

void CPID::setKp(double Kp)
{
	m_Kp = Kp;
}

void CPID::setKi(double Ki)
{
	m_Ki = Ki;
}

void CPID::setKd(double Kd)
{
	m_Kd = Kd;
}

void CPID::setiLimit(double iLimit)
{
	m_iLimit = iLimit;
}

// Get parameter
double CPID::getKp()
{
	return m_Kp;
}

double CPID::getKi()
{
	return m_Ki;
}

double CPID::getKd()
{
	return m_Kd;
}

double CPID::getiLimit()
{
	return m_iLimit;
}


void CPID::zeroPID()
{
	m_iSum = 0;
	m_lastError = 0;
}

