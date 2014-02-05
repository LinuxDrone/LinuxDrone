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

#include "cbus.h"
#include "csystembuspool.h"

CBus::CBus(CSystemBus::BusType type, const CString& device, int slave /*= 0*/)
{
	m_bus   = CSystemBusPool::instance()->sysBus(type, device);
	m_slave = slave;
}

CBus::CBus(const CBus& bus)
{
	m_bus   = 0;
	m_slave = -1;
	operator=(bus);
}

CBus::~CBus()
{
	if (m_bus) {
		CSystemBusPool::instance()->releaseBus(m_bus);
		m_bus = 0;
	}
}

CBus& CBus::operator=(const CBus& bus)
{
	if (m_bus) {
		CSystemBusPool::instance()->releaseBus(m_bus);
	}
	m_bus = bus.m_bus;
	m_slave = bus.m_slave;
	if (m_bus) {
		m_bus->addRef();
	}
	return *this;
}

void CBus::lock()
{
	if (!m_bus) {
		return;
	}
	m_bus->lock();
}

void CBus::unlock()
{
	if (!m_bus) {
		return;
	}
	m_bus->unlock();
}

CMutex* CBus::mutex()
{
	if (!m_bus) {
		return 0;
	}
	return m_bus->mutex();
}

CSystemBus* CBus::handle()
{
	return m_bus;
}

bool CBus::isOpened() const
{
	if (!m_bus) {
		return false;
	}
	return m_bus->isOpened();
}

void CBus::setSlave(uint32_t slave)
{
	if (!m_bus) {
		return;
	}
	if (m_bus->slave() == slave) {
		return;
	}
	m_slave = slave;
	m_bus->setSlave(slave);
}

uint32_t CBus::slave()
{
	return m_slave;
}

int CBus::write(const void* data, size_t size)
{
	if (!m_bus) {
		return -1;
	}
	setSlave(m_slave);
	return m_bus->write(data, size);
}

int CBus::read(void* data, size_t size)
{
	if (!m_bus) {
		return -1;
	}
	setSlave(m_slave);
	return m_bus->read(data, size);
}
