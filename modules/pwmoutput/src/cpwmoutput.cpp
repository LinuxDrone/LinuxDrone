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
			pruNumber = (int)objParam["Pru Device"].Number();
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
	m_sharedMem = static_cast<uint8_t *>(m_pru.GetSharedMem());

	for(int i=0;i<12;i++) {
		m_pwm[i]=1000;
		setChannelPulseWidth(i,m_pwm[i]);
	}

	for(int i=0;i<4;i++) {
		// 50Hz
		m_period[i]=20000;
		setChannelPeriod(i,m_period[i]);
	}
	for(int i=4;i<8;i++) {
		// 400Hz
		m_period[i]=2500;
		setChannelPeriod(i,m_period[i]);
	}
	for(int i=8;i<12;i++) {
		// 200Hz
		m_period[i]=5000;
		setChannelPeriod(i,m_period[i]);
	}
	m_pru.RunPru();
	return true;
}

//===================================================================
//  setChannelPeriod: period [µs]
//===================================================================
void CPwmOutput::setChannelPeriod(int channel, uint32_t period)
{
	if((channel >=0) && (channel < 12)) {
		*(unsigned long *)(m_sharedMem + 0x204 + (channel*0x10)) = period * 200;
	}
}

//===================================================================
//  setChannelPulseWidth: pw [µs]
//===================================================================
void CPwmOutput::setChannelPulseWidth(int channel, uint32_t pw)
{
	if((channel >=0) && (channel < 12)) {
		if((pw > 200) && (pw < 2200)) {
			*(unsigned long *)(m_sharedMem + 0x200 + (channel*0x10)) = pw * 200;
		}
	}
}

void CPwmOutput::moduleTask()
{
	RTIME time = rt_timer_read();

	if(m_pwm[0]<2000) {
		m_pwm[0]+=20;
		//m_pwm[1]+=20;
		m_pwm[10]+=20;
		m_pwm[11]+=20;
	}
	if(m_pwm[0]>=2000) {
		m_pwm[0]=1000;
		//m_pwm[1]=1000;
		m_pwm[10]=1000;
		m_pwm[11]=1000;
	}

	for(int i=0;i<12;i++) {
		setChannelPulseWidth(i,m_pwm[i]);
	}

    RTIME diff = time - rt_timer_read();
    SRTIME el = rt_timer_ticks2ns(diff);
    uint64_t elapsed = abs(el) / 1000;
    //Logger() << "PwmTASK " << elapsed;
    //CSystem::sleep(10);
}

void CPwmOutput::receivedData()
{
	for (int i = 0;i<12;i++) {
		CString pwm = CString("pwm%1").arg(i);
		if (hasElement(pwm)) {
			m_pwm[i] = float (valueNumber(pwm));
		}
	}
}

