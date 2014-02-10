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
	uint16_t m_C[6];
	uint8_t m_oversampling;

	bool initPwmOutput();
	bool setOneReg(uint8_t reg);
	bool setReg(uint8_t reg, uint8_t data);
	bool getReg(uint8_t reg, uint8_t* data, size_t size = 1);
	void moduleTask();
};
