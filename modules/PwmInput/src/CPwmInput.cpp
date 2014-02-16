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

#include "CPwmInput.h"
#include "system/CSystemBusPool"
#include "system/CSystemBus"
#include "system/CSystemPru"
#include "system/CSystem"
#include "system/Logger"

#include <native/timer.h>

#include "my_memory"

extern "C" {
CModule* moduleCreator()
{
	return new CPwmInput();
}

const char* moduleName() {
	return "PwmInput";
}
}



CPwmInput::CPwmInput() :
	CModule(1024)
{
}

CPwmInput::~CPwmInput()
{
}

bool CPwmInput::init(const mongo::BSONObj& initObject)
{
	if (!CModule::init(initObject)) {
		return false;
	}
	CString pathToBin = "PwmIn.bin";
	int pruNumber = 0;
	if (initObject.hasElement("params")) {
		mongo::BSONElement elemParam = initObject["params"];
		mongo::BSONObj objParam = elemParam.Obj();
		if (objParam.hasElement("Pru Binary")) {
			pathToBin = objParam["Pru Binary"].String().c_str();
		}
		if (objParam.hasElement("Pru Device")) {
			pruNumber = (int)objParam["Pru Device"].Number();
		}
	}
	m_pru = CSystemPru(pathToBin,pruNumber);
	return initPwmInput();
}

bool CPwmInput::start()
{
	CModule::start(this, &CPwmInput::moduleTask);
	return true;
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

bool CPwmInput::initPwmInput()
{
	m_pru.Init();
	for(int i=0;i<12;i++) {
		m_pwm[i]=0;
	}
	m_sharedMem = static_cast<uint8_t *>(m_pru.GetSharedMem());
/*
	m_pwm[0] = (*(unsigned long *)(m_sharedMem + 0x104)/200);
	m_pwm[1] = (*(unsigned long *)(m_sharedMem + 0x114)/200);
	m_pwm[2] = (*(unsigned long *)(m_sharedMem + 0x124)/200);
	m_pwm[3] = (*(unsigned long *)(m_sharedMem + 0x134)/200);
	m_pwm[4] = (*(unsigned long *)(m_sharedMem + 0x144)/200);
	m_pwm[5] = (*(unsigned long *)(m_sharedMem + 0x154)/200);
	m_pwm[6] = (*(unsigned long *)(m_sharedMem + 0x164)/200);
*/

	m_pru.RunPru();
	return true;
}

uint32_t CPwmInput::readChannel(int ch)
{
	if((ch >=0) && (ch < 7)) {
		uint32_t channelData = (*(unsigned long *)(m_sharedMem + 0x104 + (0x10*ch))/200);
		if((channelData < 2100) && (channelData > 900)) {
			return channelData;
		}
	}
	return 0;
}


void CPwmInput::moduleTask()
{
	RTIME time = rt_timer_read();

	for(int i=0; i<7; i++) {
		m_pwm[i]=readChannel(i);
	}

	mongo::BSONObjBuilder builder;
	builder << "pwm0" << m_pwm[0];
	builder << "pwm1" << m_pwm[1];
	builder << "pwm2" << m_pwm[2];
	builder << "pwm3" << m_pwm[3];
	builder << "pwm4" << m_pwm[4];
	builder << "pwm5" << m_pwm[5];
	builder << "pwm6" << m_pwm[6];
	sendObject(builder.obj());

    RTIME diff = time - rt_timer_read();
    SRTIME el = rt_timer_ticks2ns(diff);
    uint64_t elapsed = abs(el) / 1000;
    //printf("%5d\r",m_pwm[0]);
    //Logger() << "PwmTASK " << elapsed;
    //CSystem::sleep(10);
}
