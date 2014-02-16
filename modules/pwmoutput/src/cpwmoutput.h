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
#include "system/CBus"
#include "system/CSystemPru"

class CPwmOutput : public CModule
{
public:
	CPwmOutput();
	~CPwmOutput();

	virtual bool init(const mongo::BSONObj& initObject);
	bool start();

private:
	CSystemPru m_pru;
	uint32_t m_pwm[12];
	uint32_t m_period[12];
	uint8_t *m_sharedMem;

	bool initPwmOutput();
	void moduleTask();
	void setChannelPeriod(int channel, uint32_t period);
	void setChannelPulseWidth(int channel, uint32_t pw);
// notify
	virtual void recievedData(const mongo::BSONObj& data);
};
