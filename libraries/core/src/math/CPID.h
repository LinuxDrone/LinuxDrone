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

class CPID
{
public:
	CPID();
	CPID(double Kp, double Ki, double Kd, double iLimit);

	~CPID();

	double calcPID(double error, double dT);

	void setPID(double Kp, double Ki, double Kd, double iLimit);
	void setKp(double Kp);
	void setKi(double Ki);
	void setKd(double Kd);
	void setiLimit(double iLimit);

	double getKp();
	double getKi();
	double getKd();
	double getiLimit();

	void zeroPID();


private:
	double m_Kp, m_Ki, m_Kd;
	double m_dT;
	double m_iSum;
	double m_iLimit;
	double m_lastError;
};
