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

class CDotNetWrapper : public CModule
{
public:
	CDotNetWrapper();
	~CDotNetWrapper();

	virtual bool init(const mongo::BSONObj& initObject);
	bool start();

private:
	uint32_t m_pwm[12];
	uint8_t *m_sharedMem;

	bool initPwmInput();
	void moduleTask();
};
