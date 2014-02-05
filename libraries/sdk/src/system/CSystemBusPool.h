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

#include <vector>
#include "CSystemBus.h"
#include "thread/CMutex"

class CString;
class CSystemBus;

class CSystemBusPool
{
public:
	CSystemBusPool();
	~CSystemBusPool();

	static CSystemBusPool* instance();

	void lock();
	void unlock();
	CMutex* mutex();

	CSystemBus* sysBus(CSystemBus::BusType type, const CString& devName);
	void releaseBus(CSystemBus* bus);

	void clear();

private:
	std::vector<CSystemBus*> m_pool;
	CMutex m_mutex;
};
