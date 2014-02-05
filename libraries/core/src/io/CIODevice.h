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

#ifndef __core__ciodevice__
#define __core__ciodevice__

#include "../core/CObject.h"
#include <stdint.h>

class CIODevice : public CObject
{
public:
	enum DeviceType
	{
		DeviceTypeUnknown,
		DeviceTypeFile,
		DeviceTypeMemoryDevice,
		DeviceTypeResourceReader
	};
	enum FileOpenMode
	{
		ReadOnly = 1,
		WriteOnly
	};
    
public:
	CIODevice();
	virtual ~CIODevice();

	// return the type of the device
	virtual DeviceType deviceType() const;

// open/close
	virtual bool open(uint32_t mode);
	virtual void close();
	virtual bool isOpen() const;
    virtual bool isSequential() const;

// io
	virtual uint64_t write(const void* data, uint64_t len);
	virtual uint64_t read(void* data, uint64_t len) const;
    virtual uint64_t bytesAvailable() const;

//
	virtual bool seek(uint64_t pos);
	virtual uint64_t pos() const;
	virtual uint64_t size() const;

protected:
};

#endif /* defined(__core__ciodevice__) */
