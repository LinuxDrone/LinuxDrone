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

class CDispConsole : public CModule
{
public:
	CDispConsole();
	~CDispConsole();

	virtual bool init(const mongo::BSONObj& initObject);
	bool start();

private:

	float in_data[8], in_xyz[3];
	uint8_t scrline;
	float periodUpdateConsole = 0;
	RTIME timeTickOld = 0;

	void moduleTask();

protected:

	virtual void receivedData() override;
};
