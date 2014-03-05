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

#include "core/CObject"
#include "thread/CMutex"
#include "text/CString"
#include <stddef.h>
#include <stdint.h>

class CSystemBus : public CObject
{
public:
    enum BusType 
    {
        BusType_Unknown,
        BusType_Serial,
        BusType_I2C
    };

public:
    CSystemBus();
    virtual ~CSystemBus();

    virtual BusType type() const;

    void lock();
    void unlock();
    CMutex* mutex();

    virtual bool open(const CString& busName) = 0;
    virtual void close() = 0;
    virtual bool isOpened() const = 0;
    virtual CString busName() const;

    virtual bool setSlave(uint32_t slave) = 0;
    virtual uint32_t slave() const = 0;

    virtual int write(const void* data, size_t size) = 0;
    virtual int read(void* data, size_t size) = 0;

protected:
    CString m_busName;
    CMutex  m_mutex;
};
