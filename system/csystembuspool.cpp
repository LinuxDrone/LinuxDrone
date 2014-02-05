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

#include "csystembuspool.h"
#include "csystemi2c.h"
#include "my_memory"

CSystemBusPool::CSystemBusPool()
{
}

CSystemBusPool::~CSystemBusPool()
{
	clear();
}

CSystemBusPool* CSystemBusPool::instance()
{
	static CSystemBusPool pool;
	return &pool;
}

CSystemBus* CSystemBusPool::sysBus(CSystemBus::BusType type, const CString& devName)
{
	if (type == CSystemBus::BusType_Unknown) {
		return 0;
	}
	CMutexSection locker(&m_mutex);
	for (auto it = m_pool.begin();it<m_pool.end();it++) {
		if (type == CSystemBus::BusType_I2C) {
			CSystemI2C* i2c = dynamic_cast<CSystemI2C*>(*it);
			if (i2c == 0) {
				continue;
			}
			if (i2c->busName() == devName) {
				(*it)->addRef();
				return *it;
			}
		}
	}
	if (type == CSystemBus::BusType_I2C) {
		CSystemI2C* i2c = new CSystemI2C();
		if (!i2c) {
			return 0;
		}
		i2c->addRef();
		if (!i2c->open(devName)) {
			SAFE_RELEASE(i2c);
			return 0;
		}
		m_pool.push_back(i2c);
		return i2c;
	}
	return 0;
}

void CSystemBusPool::releaseBus(CSystemBus* bus)
{
	if (!bus) {
		return;
	}
	CMutexSection locker(&m_mutex);
	uint32_t refs = bus->refs();
	if (refs == 1) {
		for (auto it = m_pool.begin();it<m_pool.end();it++) {
			if (*it == bus) {
				m_pool.erase(it);
				SAFE_RELEASE(bus);
				return;
			}
		}
	}
	bus->release();
}

void CSystemBusPool::clear()
{
	CMutexSection locker(&m_mutex);
	for (auto it = m_pool.begin();it<m_pool.end();it++) {
		SAFE_RELEASE((*it));
	}
	m_pool.clear();
}
