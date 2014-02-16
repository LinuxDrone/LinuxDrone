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

void CPwmOutput::recievedData(const mongo::BSONObj& data)
{
	mongo::BSONObjIterator it(data);
	while (it.more()) {
		mongo::BSONElement elem = it.next();
		if (CString("pwm0") == elem.fieldName()) {
			m_pwm[0] = float (elem.Number());
		} else if (CString("pwm1") == elem.fieldName()) {
			m_pwm[1] = float (elem.Number());
		} else if (CString("pwm2") == elem.fieldName()) {
			m_pwm[2] = float (elem.Number());
		} else if (CString("pwm3") == elem.fieldName()) {
			m_pwm[3] = float (elem.Number());
		} else if (CString("pwm4") == elem.fieldName()) {
			m_pwm[4] = float (elem.Number());
		} else if (CString("pwm5") == elem.fieldName()) {
			m_pwm[5] = float (elem.Number());
		} else if (CString("pwm6") == elem.fieldName()) {
			m_pwm[6] = float (elem.Number());
		} else if (CString("pwm7") == elem.fieldName()) {
			m_pwm[7] = float (elem.Number());
		} else if (CString("pwm8") == elem.fieldName()) {
			m_pwm[8] = float (elem.Number());
		} else if (CString("pwm9") == elem.fieldName()) {
			m_pwm[9] = float (elem.Number());
		} else if (CString("pwm10") == elem.fieldName()) {
			m_pwm[10] = float (elem.Number());
		} else if (CString("pwm11") == elem.fieldName()) {
			m_pwm[11] = float (elem.Number());
		}
	}
}

