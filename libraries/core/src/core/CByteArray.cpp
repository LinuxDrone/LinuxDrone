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

#include "CByteArray.h"
#include "../thread/CMutex.h"
#include "../text/CString.h"
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include "../memmanager/my_memory.h"

class CByteArrayPrivate
{
    friend class CByteArray;
public:
	CByteArrayPrivate(uint32_t size, const char* data = 0, bool referenceOnly = false) {
        m_refCount = 1;
		m_referenceOnly = referenceOnly;
        if (referenceOnly) {
            assert(data != 0);
            m_data = const_cast<char*>(data);
            m_size = size;
        } else {
            m_data = (char*)my_malloc(size);
            if (m_data) {
                m_size = size;
            }
            if (data) {
                memcpy(m_data, data, size);
            }
        }
    }
	CByteArrayPrivate(const CByteArrayPrivate& other) {
        m_refCount = 1;
        m_data     = 0;
        m_size     = 0;
        m_refCount = false;
        operator=(other);
    }
	~CByteArrayPrivate() {
        if (!m_referenceOnly) {
            if (m_data) {
            	my_free(m_data);
                m_data = 0;
            }
            m_size = 0;
        }
    }
    
	void ref() {
        CMutexSection locker( &m_mutex );
        ++m_refCount;
    }
	void deref() {
        CMutexSection locker( &m_mutex );
        if( !m_refCount )
            return;
        if( m_refCount > 1 )
            --m_refCount;
        else
        {
            locker.unlock();
            delete this;
        }
    }
	uint32_t refs() const {
        CMutexSection locker(&m_mutex);
        return m_refCount;
    }
    
	const CByteArrayPrivate& operator=(const CByteArrayPrivate& other) {
        if(m_data == other.m_data)
            return *this;
        if (m_data)
        {
        	my_free(m_data);
            m_data = 0;
        }
        m_size = 0;
        m_referenceOnly = false;
        if(other.m_size)
        {
            if (other.m_referenceOnly) {
                m_referenceOnly = true;
                m_data = other.m_data;
                m_size = other.m_size;
            } else {
                m_referenceOnly = false;
                m_data = (char*)my_malloc(other.m_size);
                if (m_data)
                {
                    m_size = other.m_size;
                    memcpy(m_data, other.m_data, m_size);
                }
            }
        }
        return *this;
    }
    
	bool setSize(uint32_t newSize) {
        if (!newSize) {
            m_size = 0;
            if (!m_referenceOnly) {
                if (m_data) {
                	my_free(m_data);
                    m_data = 0;
                }
            }
            m_data = 0;
            m_referenceOnly = false;
        } else if (m_referenceOnly) {
            if (newSize <= m_size) {
                m_size = newSize;
            } else {
                return false;
            }
        } else {
            char* data = (char*)my_realloc(m_data, newSize);
            if (!data) {
                return false;
            }
            m_data = data;
            m_size = newSize;
        }
        return true;
    }
	uint32_t size() const {
        return m_size;
    }
	char*& data() {
        return m_data;
    }
    
private:
    uint32_t   m_refCount;
	uint32_t   m_size;
	char     * m_data;
    bool       m_referenceOnly;
    mutable CMutex   m_mutex;
};


CByteArray::CByteArray()
{
    d = 0;
}

CByteArray::CByteArray(const char* data, uint32_t size, bool referenceOnly /*= false*/)
{
    d = new CByteArrayPrivate(size, data, referenceOnly);
}

CByteArray::CByteArray(const unsigned char* data, uint32_t size, bool, bool referenceOnly /*= false*/)
{
    d = new CByteArrayPrivate(size, (const char*)data, referenceOnly);
}

CByteArray::CByteArray(const CByteArray& data)
{
	d = data.d;
	if (d) {
		d->ref();
    }
}

CByteArray::CByteArray(const CString& string)            // construct byte array from text dump
{
	d = 0;
	//Каждый байт кодируется двумя 16-ричными разрядами (символами)
	if (string.length() % 2)
	{
		assert(false);
		return;
	}
    
	for (int i = 0;i<string.length();i+=2)
	{
		char highDigit = string.at(i), lowDigit = string.at(i + 1);
		unsigned char byte = highDigit * 16 + lowDigit;
		append((char*)&byte, 1);
	}
}

CByteArray::~CByteArray()
{
    if (d) {
        d->deref();
        d = 0;
    }
}

const CByteArray& CByteArray::operator=(const CByteArray& other)
{
	if (d) {
		d->deref();
		d = 0;
	}
	if (other.d) {
		other.d->ref();
		d = other.d;
	}
	return *this;
}

CByteArray& CByteArray::operator+=(const CByteArray& other)
{
    return append(other.data(), other.size());
}

char& CByteArray::operator[](int pos)
{
	if( !d || pos >= int(d->m_size) ) {
        assert(false);
    }
	return d->m_data[pos];
}

char CByteArray::operator[](int pos) const
{
	if( !d || pos >= int(d->m_size) ) {
        assert(false);
    }
	return d->m_data[pos];
}

bool CByteArray::operator<(const CByteArray& other) const
{
    if (size() > other.size()) {
        return false;
    } else if (size() < other.size()) {
        return true;
    } else if (size() == other.size()) {
        if (size() == 0) {
            return false;
        }
        int size = this->size();
        const char* data = this->data();
        const char* dataOther = other.data();
        for (int i = 0;i<size;i++) {
            if (data[i] >= dataOther[i]) {
                return false;
            }
        }
        return true;
    } else {
        assert(false);
    }
    return false;
}

CByteArray& CByteArray::append(const char* data, uint32_t size)
{
    if (d && d->m_referenceOnly) {
        assert(false);
        return *this;
    }
	if (!data)
		return *this;
	if(d)
	{
		if(d->refs() == 1)
		{
			uint32_t oldSize = d->size();
			if (!d->setSize(oldSize+size)) {
				return *this;
            }
            if (d->m_data && data) {
                memcpy(d->data() + oldSize, data, size);
            }
			return *this;
		}
		else
		{
			CByteArrayPrivate* old = d;
			d = new CByteArrayPrivate(old->size(), old->data());
			old->deref();
			return append( data, size );
		}
	}
	else
	{
		d = new CByteArrayPrivate(size, data);
		return *this;
	}
    return *this;
}

CByteArray& CByteArray::append(const char* data, const RANGE& range)
{
    if (d && d->m_referenceOnly) {
        assert(false);
        return *this;
    }
	if (!data || -1 == range.location || 0 == range.length) {
		return *this;
    }
	makeNewDataIfNeeded();
	int oldSize = d->size();
	if (!d->setSize(oldSize+range.length))
	{
		assert(0 == "not enough memory");
		return *this;
	}
	memcpy(d->m_data+oldSize, data+range.location, range.length);
	return *this;
}

CByteArray& CByteArray::append(const CByteArray& other)
{
	return append(other.data(), other.size());
}

CByteArray& CByteArray::insert(uint32_t start, const char* data, uint32_t size)
{
	if (!size || !data)
		return *this;
    if (d && d->m_referenceOnly) {
        assert(false);
        return *this;
    }
	if (start > d->m_size)
		return *this;
	uint32_t oldSize = this->size();
	setSize(this->size()+size);
	if (!d || !d->m_data)
		return *this;
	char* dst = d->m_data+d->m_size-1;
	const char* src = d->m_data+oldSize-1;
	for(uint32_t i = 0;i<(oldSize-start);i++ )
	{
		*dst = *src;
		dst--;
		src--;
	}
	memcpy(d->m_data+start, data, size);
	return *this;
}

CByteArray& CByteArray::remove(int start, int len)
{
    if (d && d->m_referenceOnly) {
        assert(false);
    }
	// проверим входящие параметры
	if( start >= int(d->m_size) )
		return *this;
	if( len > int(d->m_size-start) )
		len = d->m_size-start;
	// теперь скопируем данные
	char* dst = d->m_data+start;
	const char* src = d->m_data+start+len;
	int size = d->m_size - (start+len);
	for( int i = 0;i<size;i++ )
	{
		*dst = *src;
		dst++;
		src++;
	}
	// а теперь изменим размер памяти
	setSize( d->m_size -len );
	return *this;
}

CByteArray CByteArray::makeCopy() const
{
    if (d == 0) {
        return CByteArray();
    }
    return CByteArray(data(), size());
}

CByteArray CByteArray::fromRawData(const char* data, uint32_t size, bool referenceOnly /*= false*/)
{
	return CByteArray(data, size, referenceOnly);
}

bool CByteArray::isEmpty() const
{
	if (!data() || !size()) {
		return true;
    }
	return false;
}

// specified byte array is equal our array?
bool CByteArray::isEqual( const CByteArray& other ) const
{
	if (d == other.d) {
		return true;
    }
	if (size() != other.size()) {
		return false;
    }
	uint32_t size = this->size();
	const char* ptr1 = data();
	const char* ptr2 = other.data();
	for (uint32_t i = 0;i<size;i++)
	{
		if( *ptr1 != *ptr2 )
			return false;
		ptr1++; ptr2++;
	}
	return true;
}

// data in the buffer as a text?
// 0 must meet  only at the and, or do not meet
bool CByteArray::isString() const
{
	if (!d) {
		return false;
    }
	const char* ptr = d->data();
	for (uint32_t i = 0; i < d->m_size; i++, ptr++)
	{
		if (*ptr == 0)
		{
			if (i != d->m_size-1) {
				return false;
			} else {
				return true;
            }
		}
	}
	return true;
}

bool CByteArray::isReferenceOnly() const
{
    if (!d) {
        return false;
    }
    return d->m_referenceOnly;
}

bool CByteArray::setSize(uint32_t newSize)
{
    if (d && d->m_referenceOnly) {
        assert(false);
        return false;
    }
	if (!d) {
		d = new CByteArrayPrivate(newSize, 0);
    }
	else
	{
		if (1 != d->refs())
		{
			CByteArrayPrivate* old_d = d;
			d = new CByteArrayPrivate(newSize);
			if (newSize && old_d->size())
			{
				uint32_t size = newSize;
				if (size > old_d->size()) {
					size = old_d->size();
                }
				if (d->data()) {
					memcpy( (char*)d->data(), old_d->data(), size );
                }
			}
			old_d->deref();
		}
		else
			d->setSize(newSize);
	}
	return true;
}

uint32_t CByteArray::size() const
{
	if( d )
		return d->size();
	else
		return 0;
}

const char* CByteArray::data() const
{
	if( d )
		return d->data();
	else
		return 0;
}

// string with a dump of the object
CString CByteArray::generateDump(bool addSpacesBetweenBytes /*= false*/) const
{
	static const char hexVal[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	CString dump;
    
	for (uint32_t i = 0;i<size();i++)
	{
		char byteTextHex [3] = {0};
		unsigned char byte = *reinterpret_cast<const unsigned char*>(data() + i);
		unsigned char hexDigitHigh = byte / 16, hexDigitLow = byte & 0x0F;
        
		byteTextHex[0] = hexVal[hexDigitHigh];
		byteTextHex[1] = hexVal[hexDigitLow];
        
		dump += byteTextHex;
		if (addSpacesBetweenBytes)
			dump += ' ';
	}
	return dump;
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

void CByteArray::makeNewDataIfNeeded()
{
	if (!d) {
		d = new CByteArrayPrivate((uint32_t)0);
    }
	else if( 1 != d->refs() )
	{
		CByteArrayPrivate* new_d = new CByteArrayPrivate( *d );
		assert(new_d);
		d->deref();
		d = new_d;
	}
}
