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

#include "CSystemBus.h"

CSystemBus::CSystemBus()
{
}

CSystemBus::~CSystemBus()
{
}

CSystemBus::BusType CSystemBus::type() const
{
	return BusType_Unknown;
}

void CSystemBus::lock()
{
	m_mutex.lock();
}

void CSystemBus::unlock()
{
	m_mutex.unlock();
}

CMutex* CSystemBus::mutex()
{
	return &m_mutex;
}

CString CSystemBus::busName() const
{
	return m_busName;
}
