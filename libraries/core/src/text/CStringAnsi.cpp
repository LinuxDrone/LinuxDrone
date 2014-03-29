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


#include <assert.h>
#include <vector>
#include "CStringAnsi.h"
#include "../thread/CMutex.h"
#include "../memmanager/my_memory.h"

std::vector<CStringAnsiPrivate*> m_cache;
CMutex                           m_mutexCache;

class CStringAnsiPrivate
{
public:
    CStringAnsiPrivate() {
        m_refs          = 0;
        m_data          = 0;
        m_length        = 0;
        m_referenceOnly = false;
    }
    CStringAnsiPrivate(const CStringAnsiPrivate& other) {
        assert(false);
        operator=(other);
    }
    ~CStringAnsiPrivate() {
        if (m_data) {
        	if (!m_referenceOnly) {
        		my_free(m_data);
        	}
            m_data = 0;
        }
    }

//===================================================================
//  s t a t i c   f u n c t i o n s
//===================================================================

    static CStringAnsiPrivate* createFromCache() {
    	return new CStringAnsiPrivate();
//        CMutexSection locker(&m_mutexCache);
//        if (!m_cache.size()) {
//            locker.unlock();
//            return new CStringAnsiPrivate();
//        }
//        CStringAnsiPrivate* d = m_cache[m_cache.size()-1];
//        m_cache.pop_back();
//        return d;
    }
    static void deleteToCache(CStringAnsiPrivate* d) {
        if (!d) {
            return;
        }
        delete d;
//        CMutexSection locker(&m_mutexCache);
//        if (m_cache.size() >= MAX_CACHE_SIZE) {
//            locker.unlock();
//            delete d;
//            return;
//        }
//        if (d->m_referenceOnly) {
//            d->clear();
//        }
//        m_cache.push_back(d);
    }
//    static void clearCache() {
//        CMutexSection locker(&m_mutexCache);
//        for (CStringAnsiPrivate* d:m_cache) {
//            delete d;
//        }
//        m_cache.clear();
//    }

//===================================================================
//  p u b l i c   f u n c t i o n s
//===================================================================

    uint32_t addRef() {
        return ++m_refs;
    }
    uint32_t release() {
        if (!m_refs) {
            return 0;
        }
        m_refs--;
        if (!m_refs) {
            CStringAnsiPrivate::deleteToCache(this);
            return 0;
        }
        return m_refs;
    }
    uint32_t refs() const {
        return m_refs;
    }

    CStringAnsiPrivate& operator=(const CStringAnsiPrivate& other) {
        if (m_data) {
            if (!m_referenceOnly) {
                free(m_data);
                m_data = 0;
                m_length = 0;
            }
        }
        if (other.m_referenceOnly) {
        	m_data = other.m_data;
        	m_length = other.m_length;
        	m_referenceOnly = true;
        }
        else if (other.m_data && other.m_length) {
            setSize(other.m_length);
            assert(m_length == other.m_length);
            if (m_length != other.m_length) {
                return *this;
            }
            memcpy(m_data, other.m_data, other.m_length);
            m_length   = other.m_length;
            m_data[m_length] = 0x0;
        } else {
            m_data          = 0;
            m_length        = 0;
            m_referenceOnly = false;
        }
        return *this;
    }

    CStringAnsiPrivate& operator=(const char* str) {
        if (!str) {
            clear();
        } else {
            if (m_referenceOnly) {
                clear();
            }
            size_t len = strlen(str);
            if( !len )
                clear();
            else
            {
                char* data = (char*)my_realloc(m_data, len+1);
                if (!data) {
                    return *this;
                }
                m_data = data;
                memcpy( m_data, str, len );
                m_data[len] = 0x0;
                m_length    = len;
            }
        }
        return *this;
    }

    CStringAnsiPrivate& operator+=(const CStringAnsiPrivate& other)
    {
        if (!other.m_data || !other.m_length) {
            return *this;
        }
        char* data = nullptr;
        if (!m_referenceOnly && m_length) {
            data = (char*)my_realloc(m_data, m_length+other.m_length+1);
        } else {
            data = (char*)my_malloc(m_length+other.m_length+1);
            if (m_length) {
                memcpy(data, m_data, m_length);
            }
        }
        if (!data) {
            assert( 0 == "not enough memory" );
            return *this;
        }
        m_data = data;
        memcpy(m_data+m_length, other.m_data, other.m_length);
        m_length += other.m_length;
        m_data[m_length] = 0x0;
        m_referenceOnly = false;
        return *this;
    }

    CStringAnsiPrivate& operator+=(const char* str)
    {
        if (!str) {
            return *this;
        }
        size_t len = strlen(str);
        if (!len) {
            return *this;
        }
        char* data = nullptr;
        if (!m_referenceOnly && m_length) {
            data = (char*)my_realloc(m_data, m_length+len+1);
        } else {
            data = (char*)my_malloc(m_length+len+1);
            if (m_length) {
                memcpy(data, m_data, m_length);
            }
        }
        if (!data) {
            assert( 0 == "not enough memory" );
            return *this;
        }
        m_data = data;
        memcpy(m_data+m_length, str, len);
        m_length += (int)len;
        m_data[m_length] = 0x0;
        m_referenceOnly = false;
        return *this;
    }

    void clear() {
        if (m_data && !m_referenceOnly) {
            free(m_data);
        }
        m_data          = nullptr;
        m_length        = 0;
        m_referenceOnly = false;
    }

    void setSize(size_t len) {
    	assert(!m_referenceOnly);
        char* data = (char*)my_realloc(m_data, len+1);
        if (!data)
        {
            assert( 0 == "not enough memory" );
            return;
        }
        m_data = data;
        memset( m_data, 0, len+1 );
        m_length   = len;
    }

    uint32_t   m_refs;
    char     * m_data;
    size_t     m_length;
    bool       m_referenceOnly;

private:
//    static std::vector<CStringAnsiPrivate*> m_cache;
//    static CMutex                           m_mutexCache;
    static const uint32_t MAX_CACHE_SIZE = 50;
};

CStringAnsi::CStringAnsi()
{
    d = nullptr;
}

CStringAnsi::CStringAnsi(const CStringAnsi& other)
{
    d = nullptr;
    operator=(other);
}

CStringAnsi::CStringAnsi(const char *str, size_t len /*= -1*/, bool referenceOnly /*= false*/)
{
    d = nullptr;
    if (str == nullptr || len == 0) {
        return;
    }
    if (referenceOnly) {
        makeNewDataIfNeed();
        d->m_data = const_cast<char*>(str);
        if (len != -1) {
            assert(false);
        }
        d->m_length = strlen(str);
        d->m_referenceOnly = true;
    } else {
        if (len == -1) {
            operator=(str);
            return;
        }
        makeNewDataIfNeed();
        if (!d) {
            return;
        }
        d->setSize(len);
        if (!d->m_data) {
            return;
        }
        memcpy(d->m_data, str, len);
        d->m_data[len] = 0x0;
    }
}

CStringAnsi::~CStringAnsi()
{
	if (d) {
		d->release();
		d = nullptr;
	}
}

bool CStringAnsi::isEmpty() const
{
	if (!d) {
		return true;
	}
	if (!d->m_data || !d->m_length) {
		return true;
	}
	return false;
}

const char* CStringAnsi::data() const
{
    if (!d) {
        return nullptr;
    }
    return d->m_data;
}

size_t CStringAnsi::length() const
{
    if (!d) {
        return 0;
    }
    return d->m_length;
}

CStringAnsi CStringAnsi::copy() const {
    return CStringAnsi(this->data());
}

CStringAnsi& CStringAnsi::operator=(const CStringAnsi& other)
{
	if (d) {
		d->release();
		d = 0;
	}
    d = other.d;
    if (d) {
    	d->addRef();
    }
    return *this;
}

CStringAnsi &CStringAnsi::operator = (const char *str)
{
	if (!d) {
		makeNewDataIfNeed();
	} else if (d->refs() > 1) {
		d->release();
		d = CStringAnsiPrivate::createFromCache();
	} else {
		if (!d || d->refs() != 1) {
			assert(false);
		}
	}

    assert( d );
    (*d) = str;
    return *this;
}

bool CStringAnsi::operator ==(const CStringAnsi &str) const {
    if (d == str.d) {
        return true;
    }
    if (d == 0 || str.d == 0) {
        if (d == 0 && str.length() == 0) {
            return true;
        }
        if (this->length() == 0 && str.d == 0) {
            return true;
        }
        return false;
    }
    if (d->m_length != str.d->m_length) {
        return false;
    }
    if (!memcmp(d->m_data, str.d->m_data, d->m_length)) {
        return true;
    }
    return false;
}

bool CStringAnsi::operator ==(const char *str) const {
    if ((!d || (!d->m_data || !d->m_length)) && !str) {
        return true;
    }
    if (d && d->m_data == str) {
        return true;
    }
    return operator==(CStringAnsi(str, -1, true));
}

bool CStringAnsi::operator !=(const CStringAnsi &str) const {
    return !operator==(str);
}

bool CStringAnsi::operator !=(const char *str) const {
    return !operator==(str);
}

bool CStringAnsi::operator <(const CStringAnsi &str) const {
    const char* data = this->data();
    const char* data1 = str.data();

    if (!data && !data1) {
        return false;
    } else if (!data && data1) {
        return true;
    } else if (data && !data1) {
        return false;
    } else if (!data || !data1) {
        return false;
    }
    if (0 > strcmp(d->m_data, str.d->m_data)) {
        return true;
    }
    return false;
}

bool CStringAnsi::operator <(const char *str) const {
    return operator<(CStringAnsi(str, (size_t) -1, true));
}

CStringAnsi& CStringAnsi::operator+=(const CStringAnsi& str)
{
    if (!str.d) {
        return *this;
    }
    makeNewDataIfNeed();
    *d += *str.d;
    return *this;
}

CStringAnsi& CStringAnsi::operator+=(const char* str)
{
    if (str == nullptr) {
        return *this;
    }
    makeNewDataIfNeed();
    *d += str;
    return *this;
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

void CStringAnsi::makeNewDataIfNeed()
{
    if (d && (1 == d->refs())) {
        return;
    }
    if (d && (1 != d->refs()))
    {
        CStringAnsiPrivate* data = CStringAnsiPrivate::createFromCache();
        if (!data) {
            assert(0 == "not enought memory");
            return;
        }
        *data = *d;
        d->release();
        d = data;
    }
    else
    {
        assert(!d);
        d = CStringAnsiPrivate::createFromCache();
    }
    assert(d);
    if (d) {
        d->addRef();
    }
}


/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
//                       global operators                         //
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
const CStringAnsi operator+(const CStringAnsi &s1, const CStringAnsi &s2)
{
    CStringAnsi t(s1);
    t += s2;
    return t;
}

const CStringAnsi operator+(const CStringAnsi &s1, const char* s2)
{
    CStringAnsi t(s1);
    t += s2;
    return t;
}
