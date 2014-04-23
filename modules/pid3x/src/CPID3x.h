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

#include "module/CModule"
#include "math/CPID"

#include <native/timer.h>

class CPID3x : public CModule
{
public:
	CPID3x();
	~CPID3x();

	virtual bool init(const mongo::BSONObj& initObject);
	bool start();

private:

	enum channel {
		Roll, Pitch, Yaw
	} channels;

	struct pidParams {
		double Kp = 1;
		double Ki = 0;
		double Kd = 0;
		double iLimit = 0;
	};

	pidParams	pidParam[3];
	CPID		pid[3];

	double in_err[3], out_pid[3];

	RTIME timeTickStart, timeTickOld;
	double dT;
	double timeElapsed = 0; // in miliseconds


	void settingsLoad(const mongo::BSONObj& initObject);
	void calcTime();
	void outData();
	void moduleTask();

protected:
	virtual void receivedData() override;
};
