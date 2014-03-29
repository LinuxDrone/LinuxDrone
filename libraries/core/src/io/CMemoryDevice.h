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

#include "CIODevice.h"

class CByteArray;

class CMemoryDevice : public CIODevice
{
public:
    CMemoryDevice(CByteArray* data);
    virtual ~CMemoryDevice();

    const CByteArray& data() const;

    virtual DeviceType deviceType() const override;

// open/close
    virtual bool open(uint32_t mode) override;
    virtual void close() override;
    virtual bool isOpen() const override;

// io
    virtual uint64_t write(const void *data, uint64_t len) override;
    virtual uint64_t read(void *data, uint64_t len) const override;

//
    virtual bool seek(uint64_t pos) override;
    virtual uint64_t pos() const override;
    virtual uint64_t size() const override;

private:
    CByteArray       * m_data;
    mutable uint64_t   m_pos;
    bool               m_bOpen;
};
