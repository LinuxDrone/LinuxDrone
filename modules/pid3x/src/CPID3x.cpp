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

#include "CPID3x.h"
#include <math.h>
//#include "CPID.h"
//#include "system/Logger"


#include "my_memory"

#define M_PI_F       3.14159265358979323846264338328f

extern "C" {
CModule* moduleCreator()
{
	return new CPID3x();
}

const char* moduleName() {
	return "PID-3x";
}
}


CPID3x::CPID3x() :
	CModule(1024)
{
	dT				= 1;
	channels		= Roll;
	timeTickStart	= 0;
	timeTickOld 	= 0;
	for(int ch=0;ch<3;ch++) {
		in_err[ch] = 0;
		out_pid[ch] = 0;
	}
}

CPID3x::~CPID3x()
{
}

bool CPID3x::init(const mongo::BSONObj& initObject)
{
	if (!CModule::init(initObject)) {
		return false;
	}

    // Read settings in MongoDB
	settingsLoad(initObject);

	for(int ch=0;ch<3;ch++) pid[ch].zeroPID();

	return true;
}

bool CPID3x::start()
{
	CModule::start(this, &CPID3x::moduleTask);

	timeTickStart = rt_timer_read();

	return true;
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

void CPID3x::moduleTask()
{
	calcTime();

	for (uint8_t ch = 0;ch<3;ch++) {
		out_pid[ch]= pid[ch].calcPID(in_err[ch], dT);
	}

	//pid[Pitch].calcPID(in_err[channel::Pitch], dT);
	//pid[Yaw].calcPID(in_err[channel::Yaw], dT);

	outData();
}

void CPID3x::settingsLoad(const mongo::BSONObj& initObject)
{
	const char* names[][4] = {
			{"RollKp", "RollKi", "RollKd", "RollLimit"},
			{"PitchKp", "PitchKi", "PitchKd", "PitchLimit"},
			{"YawKp", "YawKi", "YawKd", "YawLimit"}
	};

	if (initObject.hasElement("params")) {
		mongo::BSONElement elemParam = initObject["params"];
		mongo::BSONObj objParam = elemParam.Obj();

		for (uint8_t ch = 0;ch<3;ch++) {
			uint8_t p = 0;
			if (objParam.hasElement(names[ch][p]))	pidParam[ch].Kp = objParam[names[ch][p++]].Number();
			if (objParam.hasElement(names[ch][p]))	pidParam[ch].Ki = (objParam[names[ch][p++]].Number());
			if (objParam.hasElement(names[ch][p]))	pidParam[ch].Kd = objParam[names[ch][p++]].Number();
			if (objParam.hasElement(names[ch][p]))	pidParam[ch].iLimit = (objParam[names[ch][p++]].Number());

			pid[ch].setPID(
					pidParam[ch].Kp,
					pidParam[ch].Ki,
					pidParam[ch].Kd,
					pidParam[ch].iLimit);
		}

/*
		if (objParam.hasElement("RollKp")) pid[channel::Roll].setKp(objParam["RollKp"].Number());
		if (objParam.hasElement("RollKi")) pid[channel::Roll].setKi(objParam["RollKp"].Number());
		if (objParam.hasElement("RollKd")) pid[channel::Roll].setKd(objParam["RollKp"].Number());
		if (objParam.hasElement("RollLimit")) pid[channel::Roll].setiLimit(objParam["RollLimit"].Number());

		if (objParam.hasElement("PitchKp")) pid[channel::Pitch].setKp(objParam["PitchKp"].Number());
		if (objParam.hasElement("PitchKi")) pid[channel::Pitch].setKi(objParam["PitchKp"].Number());
		if (objParam.hasElement("PitchKd")) pid[channel::Pitch].setKd(objParam["PitchKp"].Number());
		if (objParam.hasElement("PitchLimit")) pid[channel::Pitch].setiLimit(objParam["PitchLimit"].Number());

		if (objParam.hasElement("YawKp")) pid[channel::Yaw].setKp(objParam["YawKp"].Number());
		if (objParam.hasElement("YawKi")) pid[channel::Yaw].setKi(objParam["YawKp"].Number());
		if (objParam.hasElement("YawKd")) pid[channel::Yaw].setiLimit(objParam["YawKp"].Number());
*/
	}
    return;
}

void CPID3x::calcTime()
{
	SRTIME el;
	RTIME timeTick = rt_timer_read();
	el = rt_timer_ticks2ns(timeTick - timeTickStart);
	timeElapsed = static_cast<double>(el) / 1000000.0;

	el = rt_timer_ticks2ns(timeTick - timeTickOld);
	dT = static_cast<double>(el) / 1000000000.0;
	timeTickOld = timeTick;
}


void CPID3x::outData()
{
	mongo::BSONObjBuilder builder;
	builder << "roll" << out_pid[channel::Roll];
	builder << "pitch" << out_pid[channel::Pitch];
	builder << "yaw" << out_pid[channel::Yaw];

	sendObject(builder.obj());

}


//===================================================================
//  p r o t e c t e d   f u n c t i o n s
//===================================================================

//-------------------------------------------------------------------
//  n o t i f y
//-------------------------------------------------------------------

void CPID3x::receivedData()
{
	const char* names[] = { "errRoll", "errPitch", "errYaw" };
	for (int i = 0;i<3;i++) {
		if (hasElement(names[i])) {
			in_err[i] = static_cast<double>(valueNumber(names[i]));
		}
	}
}
