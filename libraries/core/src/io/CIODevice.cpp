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

#include "CIODevice.h"

CIODevice::CIODevice()
{
}

CIODevice::~CIODevice()
{
}

// return the type of the device
CIODevice::DeviceType CIODevice::deviceType() const
{
	return DeviceTypeUnknown;
}

/////////////////////////////////////////////////////////////////////
//                             open/close                          //
/////////////////////////////////////////////////////////////////////

bool CIODevice::open(uint32_t /*mode*/)
{
	return false;
}

void CIODevice::close()
{
}

bool CIODevice::isOpen() const
{
	return false;
}

bool CIODevice::isSequential() const
{
	return false;
}

/////////////////////////////////////////////////////////////////////
//                                io                               //
/////////////////////////////////////////////////////////////////////

uint64_t CIODevice::write(const void* data, uint64_t len)
{
	return 0;
}

uint64_t CIODevice::read(void* data, uint64_t len) const
{
	return 0;
}

uint64_t CIODevice::bytesAvailable() const
{
	return 0;
}

/////////////////////////////////////////////////////////////////////
//                                                                 //
/////////////////////////////////////////////////////////////////////

bool CIODevice::seek(uint64_t pos)
{
	return 0;
}

uint64_t CIODevice::pos() const
{
	return 0;
}

uint64_t CIODevice::size() const
{
	return 0;
}
