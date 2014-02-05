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

#include "CString_p.h"
#include "unicode/gunicode.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "../memmanager/my_memory.h"

CStringPrivate::CStringPrivate()
{
	m_refs     = 0;
	m_data     = 0;
	m_length   = 0;
	m_numChars = 0;
}

CStringPrivate::CStringPrivate(const CStringPrivate& src)
{
    m_refs     = 0;
	m_data     = 0;
	m_length   = 0;
	m_numChars = 0;
	operator=(src);
}

CStringPrivate::~CStringPrivate()
{
    clear();
}

uint32_t CStringPrivate::addRef()
{
    return ++m_refs;
}

uint32_t CStringPrivate::release()
{
	if( !m_refs )
		return 0;
	m_refs--;
	if( !m_refs )
	{
		delete this;
		return 0;
	}
	return m_refs;
}

uint32_t CStringPrivate::refs() const
{
    return m_refs;
}

void CStringPrivate::setSize(int size)
{
	char* data = (char*)my_realloc( m_data, size+1 );
	if (!data)
	{
		assert( 0 == "not enough memory" );
		return;
	}
	m_data = data;
	memset( m_data, 0, size+1 );
	m_length   = size;
	m_numChars = 0;
}

void CStringPrivate::clear()
{
	my_free( m_data );
	m_data   = 0;
	m_length = 0;
	m_numChars = 0;
}

CStringPrivate& CStringPrivate::operator=(const CStringPrivate& src)
{
	if (src.m_data && src.m_length)
	{
		char* data = (char*)my_realloc(m_data, src.m_length+1);
		if (!data)
		{
			assert(0 == "not enough memory");
			return *this;
		}
		m_data = data;
        
		memcpy(m_data, src.m_data, src.m_length);
		m_length   = src.m_length;
		m_data[m_length] = 0x0;
		m_numChars = src.m_numChars;
	}
	else
	{
		m_data     = 0;
		m_length   = 0;
		m_numChars = 0;
	}
	return *this;
}

CStringPrivate& CStringPrivate::operator=(const char* str)
{
	if (!str) {
		clear();
	} else {
		size_t len = strlen( str );
		if( !len )
			clear();
		else
		{
			char* data = (char*)my_realloc( m_data, len+1 );
			if( !data )
				return *this;
			m_data = data;
			memcpy( m_data, str, len );
			m_data[len] = 0x0;
			m_length    = int(len);
			recalcLength();
		}
	}
	return *this;
}

CStringPrivate& CStringPrivate::operator+=(const CStringPrivate& other)
{
	if( !other.m_data || !other.m_length )
		return *this;
	char* data = (char*)my_realloc( m_data, m_length+other.m_length+1 );
	if( !data )
	{
		assert( 0 == "not enough memory" );
		return *this;
	}
	m_data = data;
	memcpy( m_data+m_length, other.m_data, other.m_length );
	m_length += other.m_length;
	m_data[m_length] = 0x0;
	m_numChars += other.m_numChars;
	return *this;
}

CStringPrivate& CStringPrivate::operator+=(const char* str)
{
	if( !str )
		return *this;
	size_t len = strlen(str);
	if (!len) {
		return *this;
    }
	char* data = (char*)my_realloc( m_data, m_length+len+1 );
	if( !data )
	{
		assert( 0 == "not enough memory" );
		return *this;
	}
	m_data = data;
	memcpy( m_data+m_length, str, len );
	m_length += (int)len;
	m_data[m_length] = 0x0;
	m_numChars += g_utf8_pointer_to_offset( str, str+len );
	return *this;
}

void CStringPrivate::prepend(CStringPrivate& str)
{
	if (!str.m_data || !str.m_length) {
		return;
    }
	if (!m_data || !m_length) {
		operator=( str );
    } else {
		int length = m_length+str.m_length;
		char* data = (char*)my_malloc( length+1 );
		if( !data )
		{
			assert( 0 == "not enough memory" );
			return;
		}
		memcpy( data, str.m_data, str.m_length );
		memcpy( data+str.m_length, m_data, m_length );
		data[length] = 0x0;
		my_free( m_data );
		m_data   = data;
		m_length = length;
		recalcLength();
	}
}

void CStringPrivate::prepend(const char* str)
{
	if( !str )
		return;
	if( !m_data || !m_length )
		operator=( str );
	else
	{
		int len = (int)strlen( str );
		if( !len )
			return;
		int length = len + m_length;
		char* data = (char*)my_malloc( length+1 );
		if( !data )
		{
			assert( 0 == "not enough memory" );
			return;
		}
		memcpy( data, str, len );
		memcpy( data+len, m_data, m_length );
		data[length] = 0x0;
		my_free( m_data );
		m_data   = data;
		m_length = length;
		recalcLength();
	}
}

void CStringPrivate::insert(const CStringPrivate& str, int start)
{
	if( 0 > start || start > m_length )
		return;
	if( !str.m_data || !str.m_length )
		return;
	if( !m_length )
		operator=( str );
	else
	{
		// выделим кусок памяти
		char* data = (char*)my_realloc( m_data, m_length+str.m_length+1 );
		if( !data )
		{
			assert( 0 == "not enough memory" );
			return;
		}
		m_data = data;
		// теперь раздвинем данные
		int a = 0;
		for( int i = m_length;i>start;i--,a++ )
			m_data[m_length+str.m_length-a-1] = m_data[i-1];
		// а теперь запедалим новые данные
		memcpy( m_data+start, str.m_data, str.m_length );
		m_length += str.m_length;
		m_data[m_length] = 0x0;
		recalcLength();
	}
}

void CStringPrivate::insert(const char* str, int start)
{
	if( 0 > start || start > m_length )
		return;
	if( !str )
		return;
	int len = (int)strlen( str );
	if( !len )
		return;
	if( !m_length )
		operator=( str );
	else
	{
		// выделим кусок памяти
		char* data = (char*)my_realloc( m_data, m_length+len+1 );
		if( !data )
		{
			assert( 0 == "not enough memory" );
			return;
		}
		m_data = data;
		// теперь раздвинем данные
		int a = 0;
		for( int i = m_length;i>start;i--,a++ )
			m_data[m_length+len-a-1] = m_data[i-1];
		// а теперь запедалим новые данные
		memcpy( m_data+start, str, len );
		m_length += len;
		m_data[m_length] = 0x0;
		recalcLength();
	}
}

void CStringPrivate::remove(int start, int len)
{
	if( 0 > start )
		return;
	if( len > m_length-start )
		len = m_length-start;
	// для начала перенесем память
	for( int i = start;i<m_length-len;i++ )
		m_data[i] = m_data[i+len];
	// впедалим завершающий 0
	m_data[m_length-len] = 0;
	// и перевыделим память
	m_length -= len;
	m_data = (char*)my_realloc( m_data, m_length+1 );
	// а теперь осталось пересчитать кол-во реальных символов в строке
	recalcLength();
}

bool CStringPrivate::contains(const CStringPrivate& str) const
{
	if( m_data == str.m_data )
		return true;
	if( (!m_data || !m_length) &&
       (!str.m_data || !str.m_length) )
		return true;
	if( (!m_data || !m_length) ||
       (!str.m_data || !str.m_length) )
		return false;
	if( str.m_length > m_length )
		return false;
	for( int i = 0;i<m_length;i++ )
	{
		if( str.m_length > m_length-i )
			return false;
		if( !memcmp(m_data+i, str.m_data, str.m_length) )
			return true;
	}
	return false;
}

bool CStringPrivate::contains(const char* str) const
{
	if( !str && (!m_data || !m_length) )
		return true;
	if( !str )
		return false;
	int len = (int)strlen( str );
	if( !len || (!m_data || !m_length) )
		return false;
	for( int i = 0;i<m_length;i++ )
	{
		if( len > m_length-i )
			return false;
		if( !memcmp(m_data+i, str, len) )
			return true;
	}
	return false;
}

int CStringPrivate::find(const CStringPrivate& what, int from /*= 0*/)
{
	if( m_data == what.m_data )
		return from == 0 ? 0 : -1;
	if( (!m_data || !m_length) &&
       (!what.m_data || !what.m_length) )
		return from == 0 ? 0 : -1;
	if( (!m_data || !m_length) ||
       (!what.m_data || !what.m_length) )
		return -1;
	if( what.m_length > m_length )
		return -1;
	// определимся со стартовым символом
	if( -1 == from )
		return -1;
	for( int i = from;i<m_length;i++ )
	{
		if( what.m_length > m_length-i )
			return -1;
		if( !memcmp(m_data+i, what.m_data, what.m_length) )
			return i;
	}
	return -1;
}

int CStringPrivate::find(const char* what, int from /*= 0*/)
{
	if( !what && (!m_data || !m_length) )
		return from == 0 ? 0 : -1;
	if( !what )
		return -1;
	int len = (int)strlen( what );
	if( !len || (!m_data || !m_length) )
		return -1;
	// определимся со стартовым символом
	if( -1 == from )
		return -1;
	for( int i = from;i<m_length;i++ )
	{
		if( len > m_length-i )
			return -1;
		if( !memcmp(m_data+i, what, len) )
			return i;
	}
	return -1;
}

int CStringPrivate::length() const
{
	return m_length;
}

int CStringPrivate::numChars() const
{
	return m_numChars;
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

void CStringPrivate::recalcLength()
{
    m_numChars = (int)g_utf8_pointer_to_offset(m_data, m_data+m_length);
}
