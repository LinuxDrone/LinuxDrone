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

class CSystemI2C : public CSystemBus
{
public:
	CSystemI2C();
	virtual ~CSystemI2C();

	virtual BusType type() const;

	virtual bool open(const CString& busName);
	virtual void close();
	virtual bool isOpened() const;

	virtual bool setSlave(uint32_t slave);
	virtual uint32_t slave() const;

	virtual int write(const void* data, size_t size);
	virtual int read(void* data, size_t size);

private:
	int      m_file;
	uint32_t m_slave;
};
