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

#ifndef __core__cbytearray__
#define __core__cbytearray__

#include <stdint.h>

typedef struct tagRange
{
    tagRange()
    {
        location = -1;
        length   = 0;
    }
    tagRange(int loc, int len)
    {
        location = loc;
        length   = len;
    }
	
    int location;
    int length;
    
} RANGE, *PRANGE;

class CString;
class CByteArrayPrivate;

class CByteArray
{
public:
    CByteArray();
	CByteArray(const char* data, uint32_t size, bool referenceOnly = false);
	CByteArray(const unsigned char* data, uint32_t size, bool, bool referenceOnly = false);
	CByteArray(const CByteArray& data);
    CByteArray(const CString& string);            // construct byte array from text dump
    ~CByteArray();
    
	const CByteArray& operator=(const CByteArray& other);
	CByteArray& operator+=(const CByteArray& other);
	char& operator[](int pos);
	char operator[](int pos) const;
    bool operator<(const CByteArray& other) const;
    
	CByteArray& append(const char* data, uint32_t size);
	CByteArray& append(const char* data, const RANGE& range);
	CByteArray& append(const CByteArray& other);
	CByteArray& insert(uint32_t start, const char* data, uint32_t size);
	CByteArray& remove(int start, int len);
    CByteArray makeCopy() const;
    
	CByteArray fromRawData(const char* data, uint32_t size, bool referenceOnly = false);
    
	bool isEmpty() const;
	// specified byte array is equal our array?
	bool isEqual( const CByteArray& other ) const;
	// data in the buffer as a text?
	// 0 must meet  only at the and, or do not meet
	bool isString() const;
    bool isReferenceOnly() const;
	bool setSize(uint32_t newSize);
	uint32_t size() const;
    
	const char* data() const;
    
	// string with a dump of the object
	CString generateDump(bool addSpacesBetweenBytes = false) const;

private:
    CByteArrayPrivate * d;
    
    void makeNewDataIfNeeded();
};

#endif /* defined(__core__cbytearray__) */
