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

#include "cpwmoutput.h"
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
	return new CPwmOutput();
}

const char* moduleName() {
	return "PwmOutput";
}
}



CPwmOutput::CPwmOutput() :
	CModule(1024)
{
}

CPwmOutput::~CPwmOutput()
{
}

bool CPwmOutput::init(const mongo::BSONObj& initObject)
{
	if (!CModule::init(initObject)) {
		return false;
	}
	CString pathToBin = "PwmOut.bin";
	int pruNumber = 1;
	if (initObject.hasElement("params")) {
		mongo::BSONElement elemParam = initObject["params"];
		mongo::BSONObj objParam = elemParam.Obj();
		if (objParam.hasElement("Pru Binary")) {
			pathToBin = objParam["Pru Binary"].String().c_str();
		}
		if (objParam.hasElement("Pru Device")) {
			//pruNumber = (int)objParam["Pru Device"].Number();
			pruNumber = (CString(objParam["Pru Device"].String().c_str()).toInt64());
		}
	}
	m_pru = CSystemPru(pathToBin,pruNumber);
	return initPwmOutput();
}

bool CPwmOutput::start()
{
	CModule::start(this, &CPwmOutput::moduleTask);
	return true;
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

bool CPwmOutput::initPwmOutput()
{
	m_pru.Init();
	for(int i=0;i<12;i++)
	{
		m_pwm[i]=1000;
	}
	m_sharedMem = static_cast<uint8_t *>(m_pru.GetSharedMem());

	*(unsigned long *)(m_sharedMem + 0x200) = m_pwm[0] * 200;
	*(unsigned long *)(m_sharedMem + 0x210) = m_pwm[1] * 200;
	*(unsigned long *)(m_sharedMem + 0x220) = m_pwm[2] * 200;
	*(unsigned long *)(m_sharedMem + 0x230) = m_pwm[3] * 200;
	*(unsigned long *)(m_sharedMem + 0x240) = m_pwm[4] * 200;
	*(unsigned long *)(m_sharedMem + 0x250) = m_pwm[5] * 200;
	*(unsigned long *)(m_sharedMem + 0x260) = m_pwm[6] * 200;
	*(unsigned long *)(m_sharedMem + 0x270) = m_pwm[7] * 200;
	*(unsigned long *)(m_sharedMem + 0x280) = m_pwm[8] * 200;
	*(unsigned long *)(m_sharedMem + 0x290) = m_pwm[9] * 200;
	*(unsigned long *)(m_sharedMem + 0x2A0) = m_pwm[10] * 200;
	*(unsigned long *)(m_sharedMem + 0x2B0) = m_pwm[11] * 200;

	*(unsigned long *)(m_sharedMem + 0x204) = 20000 * 200;
	*(unsigned long *)(m_sharedMem + 0x214) = 20000 * 200;
	*(unsigned long *)(m_sharedMem + 0x224) = 20000 * 200;
	*(unsigned long *)(m_sharedMem + 0x234) = 20000 * 200;
	*(unsigned long *)(m_sharedMem + 0x244) = 2500 * 200;
	*(unsigned long *)(m_sharedMem + 0x254) = 2500 * 200;
	*(unsigned long *)(m_sharedMem + 0x264) = 5000 * 200;
	*(unsigned long *)(m_sharedMem + 0x274) = 5000 * 200;
	*(unsigned long *)(m_sharedMem + 0x284) = 5000 * 200;
	*(unsigned long *)(m_sharedMem + 0x294) = 5000 * 200;
	*(unsigned long *)(m_sharedMem + 0x2A4) = 5000 * 200;
	*(unsigned long *)(m_sharedMem + 0x2B4) = 5000 * 200;

	m_pru.RunPru();
	return true;
}

void CPwmOutput::moduleTask()
{
	RTIME time = rt_timer_read();

	if(m_pwm[0]<2000)
	{
		m_pwm[0]+=20;
		m_pwm[1]+=20;
		m_pwm[10]+=20;
		m_pwm[11]+=20;
	}
	if(m_pwm[0]>=2000)
	{
		m_pwm[0]=1000;
		m_pwm[1]=1000;
		m_pwm[10]=1000;
		m_pwm[11]=1000;
	}

	*(unsigned long *)(m_sharedMem + 0x200) = m_pwm[0] * 200;
	*(unsigned long *)(m_sharedMem + 0x210) = m_pwm[1] * 200;

	*(unsigned long *)(m_sharedMem + 0x2A0) = m_pwm[10] * 200;
	*(unsigned long *)(m_sharedMem + 0x2B0) = m_pwm[11] * 200;


    RTIME diff = time - rt_timer_read();
    SRTIME el = rt_timer_ticks2ns(diff);
    uint64_t elapsed = abs(el) / 1000;
    //Logger() << "PwmTASK " << elapsed;
    //CSystem::sleep(10);
}
