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

#include "CSystemBus.h"

class CBus
{
public:
	CBus(CSystemBus::BusType type = CSystemBus::BusType_Unknown, const CString& device = CString(), int slave = 0);
	CBus(const CBus& bus);
	~CBus();

	CBus& operator=(const CBus& bus);

	void lock();
	void unlock();
	CMutex* mutex();

	CSystemBus* handle();
	bool isOpened() const;

	void setSlave(uint32_t slave);
	uint32_t slave();
	int write(const void* data, size_t size);
	int read(void* data, size_t size);

private:
	CSystemBus * m_bus;
	uint32_t     m_slave;
};
