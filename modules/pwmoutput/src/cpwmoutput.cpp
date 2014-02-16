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
	CModule("PwmOutput", 1024)
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
	CString busName = "PwmOut.bin";
	int busType = 1;
	if (initObject.hasElement("params")) {
		mongo::BSONElement elemParam = initObject["params"];
		mongo::BSONObj objParam = elemParam.Obj();
		if (objParam.hasElement("bus_name")) {
			busName = objParam["bus_name"].String().c_str();
		}
		if (objParam.hasElement("bus_type")) {
			busType = (int)objParam["bus_type"].Number();
		}
	}
	m_pru = CSystemPru(busName,busType);
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
	uint8_t *sharedMem = static_cast<uint8_t *>(m_pru.GetSharedMem());

	*(unsigned long *)(sharedMem + 0x200) = 1000 * 200;
	*(unsigned long *)(sharedMem + 0x210) = 1000 * 200;
	*(unsigned long *)(sharedMem + 0x220) = 1000 * 200;
	*(unsigned long *)(sharedMem + 0x230) = 1000 * 200;
	*(unsigned long *)(sharedMem + 0x240) = 1000 * 200;
	*(unsigned long *)(sharedMem + 0x250) = 1000 * 200;
	*(unsigned long *)(sharedMem + 0x260) = 1000 * 200;
	*(unsigned long *)(sharedMem + 0x270) = 1000 * 200;
	*(unsigned long *)(sharedMem + 0x280) = 1000 * 200;
	*(unsigned long *)(sharedMem + 0x290) = 1000 * 200;
	*(unsigned long *)(sharedMem + 0x2A0) = 1000 * 200;
	*(unsigned long *)(sharedMem + 0x2B0) = 1000 * 200;

	*(unsigned long *)(sharedMem + 0x204) = 20000 * 200;
	*(unsigned long *)(sharedMem + 0x214) = 20000 * 200;
	*(unsigned long *)(sharedMem + 0x224) = 20000 * 200;
	*(unsigned long *)(sharedMem + 0x234) = 20000 * 200;
	*(unsigned long *)(sharedMem + 0x244) = 2500 * 200;
	*(unsigned long *)(sharedMem + 0x254) = 2500 * 200;
	*(unsigned long *)(sharedMem + 0x264) = 5000 * 200;
	*(unsigned long *)(sharedMem + 0x274) = 5000 * 200;
	*(unsigned long *)(sharedMem + 0x284) = 5000 * 200;
	*(unsigned long *)(sharedMem + 0x294) = 5000 * 200;
	*(unsigned long *)(sharedMem + 0x2A4) = 5000 * 200;
	*(unsigned long *)(sharedMem + 0x2B4) = 5000 * 200;
	m_pru.RunPru();
	return true;
}

bool CPwmOutput::setOneReg(uint8_t reg)
{
	return true;
}

bool CPwmOutput::setReg(uint8_t reg, uint8_t data)
{
	return true;
}

bool CPwmOutput::getReg(uint8_t reg, uint8_t* data, size_t size /*= 1*/)
{
	return true;
}

void CPwmOutput::moduleTask()
{
	RTIME time = rt_timer_read();


    RTIME diff = time - rt_timer_read();
    SRTIME el = rt_timer_ticks2ns(diff);
    uint64_t elapsed = abs(el) / 1000;
    //Logger() << elapsed;
    CSystem::sleep(100);
}
