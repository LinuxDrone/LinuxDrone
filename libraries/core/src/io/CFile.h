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

#ifndef __core__cfile__
#define __core__cfile__

#include "CIODevice.h"
#include "../text/CString.h"

class CFile : public CIODevice
{
public:
	enum Permission {
		ReadOwner = 0x4000, WriteOwner = 0x2000, ExeOwner = 0x1000,
		ReadUser  = 0x0400, WriteUser  = 0x0200, ExeUser  = 0x0100,
		ReadGroup = 0x0040, WriteGroup = 0x0020, ExeGroup = 0x0010,
		ReadOther = 0x0004, WriteOther = 0x0002, ExeOther = 0x0001
	};
    
public:
	CFile(const CString& fileName = CString());
	virtual ~CFile();
    
	// return the type of the device
	DeviceType deviceType() const;
    
// parameters of file
	void setFileName(const CString& fileName);
	CString fileName();
    
// open/close
	virtual bool open(uint32_t mode);
	virtual void close();
	virtual bool isOpen() const;
	bool remove();
	bool rename(const CString& newName);
    
// io
	virtual uint64_t write(const void* data, uint64_t len);
	uint64_t write(const CByteArray& data);
	virtual uint64_t read(void* data, uint64_t len) const;
	CByteArray readAll();
	void flush();
	
//
	virtual bool seek(uint64_t pos);
	virtual uint64_t pos() const;
	virtual uint64_t size() const;
    
private:
	CString    m_fileName;
	FILE     * m_file;
	uint32_t   m_openFlags;
    
#ifdef _WIN32
	void* m_hFile;
	void* m_hMap;
#endif
};

#endif /* defined(__core__cfile__) */
