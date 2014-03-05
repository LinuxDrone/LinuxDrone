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

#include "CSystemI2C.h"
#include "text/CString"
#include "system/Logger"

#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "my_memory"

CSystemI2C::CSystemI2C()
{
	m_file  = 0;
	m_slave = 0;
}

CSystemI2C::~CSystemI2C()
{
	close();
}

CSystemBus::BusType CSystemI2C::type() const
{
	return BusType_I2C;
}

bool CSystemI2C::open(const CString& busName)
{
	if (isOpened()) {
		return false;
	}
   
	m_file = ::open(busName.data(), O_RDWR);
	if (m_file < 0) {
		Logger() << "Failed to open the bus (" << busName << ")";
		m_file = 0;
		return false;
	}
	m_busName = busName;
	return true;
}

void CSystemI2C::close()
{
	if (!isOpened()) {
		return;
	}
	::close(m_file);
	m_file = 0;
	m_busName = CString();
}

bool CSystemI2C::isOpened() const
{
	return m_file == 0 ? false : true;
}

bool CSystemI2C::setSlave(uint32_t slave)
{
	if (!isOpened()) {
		return false;
	}
	int err = ioctl(m_file, I2C_SLAVE, slave);
	if (err) {
		Logger() << "Failed to acquire bus access and/or talk to slave. error = " << errno;
		return false;
	}
	m_slave = slave;
	return true;
}

uint32_t CSystemI2C::slave() const
{
	return m_slave;
}

int CSystemI2C::write(const void* data, size_t size)
{
	if (!isOpened()) {
		return -1;
	}
	int len = ::write(m_file, data, size);
	return len;
}

int CSystemI2C::read(void* data, size_t size)
{
	if (!isOpened()) {
		return -1;
	}
	int len = ::read(m_file, data, size);
	return len;
}
