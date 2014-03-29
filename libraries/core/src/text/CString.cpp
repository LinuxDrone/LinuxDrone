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

#include "CString.h"
#include "CString_p.h"
#include "unicode/gunicode.h"

#include <stdlib.h>
#include <memory.h>
#include <sstream>
#include <assert.h>

#include "../memmanager/my_memory.h"

CString::CString()
{
	d = 0;
}

CString::CString(const std::string& str)
{
    d = 0;
    operator=(str.c_str());
}

CString::CString(const CString& other)
{
	d = 0;
	operator=(other);
}

CString::CString(const char* str, int len /*= -1*/)
{
    d = 0;
    if (!str) {
        return;
	}
    if (-1 == len) {
        operator=( str );
    } else {
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
        d->recalcLength();
    }
}

CString::CString(char ch)
{
    d = 0;
    const char str[2] = {ch, 0};
    *this += str;
}

CString::CString(uint64_t ui64, int width /*= -1*/, int base /*= 10*/, char fillChar /*= ' '*/)
{
    std::stringstream ss;
    if( 16 == base )
        ss << std::hex;
    ss << ui64;
    std::string sstr = ss.str();
    CString str( sstr.c_str() );
    
    char data[256];
    data[0] = fillChar;
    data[1] = 0x0;
    if (-1 != width) {
        while( width > str.length() ) {
            str.prepend(data);
        }
    }
    if (16 == base) {
        str.prepend("0x");
    }
    d = 0;
    operator=(str);
}

CString::CString(int64_t i64)
{
    d = 0;
    operator+=(i64);
}

CString::CString(uint32_t ui64)
{
    d = 0;
    operator+=(ui64);
}

CString::CString(int32_t i32)
{
    d = 0;
    operator+=(i32);
}

CString::CString(uint16_t ui16)
{
    d = 0;
    operator+=(ui16);
}

CString::CString(int16_t i16)
{
    d = 0;
    operator+=(i16);
}

CString::CString(double val, int precision /*= 0*/)
{
    d = 0;
    if (precision > 0)
    {
        std::stringstream ss;
        ss.precision(precision);
        ss << std::fixed << val;
        operator+=(ss.str().c_str());
    }
    else
    {
        operator+=(val);
    }
}

CString::~CString()
{
    if (d)
    {
        d->release();
        d = 0;
    }
}

bool CString::isEmpty() const
{
    if( !d )
        return true;
    if( !d->m_data || !d->m_length )
        return true;
    return false;
}

// return the number of characters per string
int CString::length() const
{
    if( !d )
        return 0;
    return d->numChars();
}

// returns the size of the strung
int CString::size() const
{
    if( !d )
        return 0;
    return d->m_length;
}

const char* CString::data() const
{
    if( !d )
        return 0;
    return d->m_data;
}

bool CString::getCharacters(uint32_t* buffer, const RANGE& range) const
{
	if( !buffer )
		return false;
	if( !d )
		return false;
	if( !d->m_length || !d->m_data )
		return false;
	if( 0 > range.location || range.location >= this->length() )
		return false;
	if( 0 > range.length || range.location+range.length >= this->length()+1 )
		return false;
	int end = range.location+range.length;
	int a = 0, i;
	char* p = d->m_data;
	for( i = 0;i<range.location;i++ )
		p = g_utf8_next_char(p);
	for( int i = range.location;i<end;i++ )
	{
		buffer[a] = g_utf8_get_char( p );
		p = g_utf8_next_char( p );
		a++;
	}
	return true;
}

uint32_t CString::at(int i) const
{
	if( !d )
		return 0;
	if( 0 > i || i >= d->m_numChars )
		return 0;
	if( !d->m_data || !d->m_length )
		return 0;
	int offset = utf8ByteOffset( d->m_data, i, d->m_length );
	return g_utf8_get_char( d->m_data+offset );
}

uint32_t CString::operator[](int i) const
{
    return at( i );
}

CString CString::left(int size) const
{
	if( isEmpty() )
		return CString();
	CString result;
	char* p = d->m_data;
	char* p_next;
	for( int i = 0;i<size;i++ )
	{
		if( i >= length() )
			return result;
		p_next = g_utf8_next_char( p );
		result += CString( p, int(p_next-p) );
		p = p_next;
	}
	return result;
}

CString CString::right(int size) const
{
    if (length()-size <= 0)
        return *this;
    if (!d || !d->m_data)
        return CString();
    int start = length() - size;
    int offset = utf8ByteOffset( d->m_data, start, d->m_length );
    CString str(d->m_data+offset);
    return str;
}

CString CString::mid(int start, int num /*= -1*/)
{
	if( !d )
		return false;
	if (!d->m_length || !d->m_data)
		return false;
	if (0 > start || start >= this->length())
		return false;
    if (-1 == num)
        num = this->length() - start;
	if (0 > num || start+num >= this->length()+1 )
		return false;
	int end = start+num;
    const char* ptr = g_utf8_offset_to_pointer(d->m_data, start);
    const char* ptr_end = g_utf8_offset_to_pointer(d->m_data, end);
    CString result(ptr, int(ptr_end-ptr));
    return result;
}

CString CString::trimmed() const
{
    if (isEmpty())
        return *this;
    int start = 0;
    for (int i = 0;i<length();i++ )
    {
        if (!isWhitespace(at(i)))
            break;
        start++;
    }
    CString result = *this;
    result.remove(0, start);
    start = result.length();
    for (int i = result.length()-1;i>=0;i--)
    {
        if (!isWhitespace(result.at(i)))
            break;
        start--;
    }
    result.remove(start, result.length());
    return result;
}

CString& CString::operator=(const CString& other)
{
	if( d )
		d->release();
	d = other.d;
	if( d )
		d->addRef();
	return *this;
}

CString& CString::operator=(const char* str)
{
	makeNewDataIfNeed();
    
	assert( d );
	(*d) = str;
	return *this;
}

CString& CString::operator+=(const CString& str)
{
	if( !str.d )
		return *this;
	makeNewDataIfNeed();
	*d += *str.d;
	return *this;
}

CString& CString::operator+=( const char* str )
{
	if( !str )
		return *this;
	makeNewDataIfNeed();
	*d += str;
	return *this;
}

CString& CString::operator+=( char val )
{
	char str[2];
	str[0] = val; str[1] = 0x0;
	return operator+=( str );
}

CString& CString::operator+=(uint64_t ui64)
{
	std::stringstream ss;
	ss << ui64;
	std::string str = ss.str();
	return operator+=( str.c_str() );
}

CString& CString::operator+=(int64_t i64)
{
	std::stringstream ss;
	ss << i64;
	std::string str = ss.str();
	return operator+=( str.c_str() );
}

CString& CString::operator+=(uint32_t ui32)
{
	std::stringstream ss;
	ss << ui32;
	std::string str = ss.str();
	return operator+=( str.c_str() );
}

CString& CString::operator+=(int32_t i32)
{
	std::stringstream ss;
	ss << i32;
	std::string str = ss.str();
	return operator+=( str.c_str() );
}

CString& CString::operator+=(uint16_t ui16 )
{
	std::stringstream ss;
	ss << ui16;
	std::string str = ss.str();
	return operator+=( str.c_str() );
}

CString& CString::operator+=(int16_t i16)
{
	std::stringstream ss;
	ss << i16;
	std::string str = ss.str();
	return operator+=( str.c_str() );
}

CString& CString::operator+=(double val)
{
	std::ostringstream ss;
	ss << val;
    
	std::string str = ss.str();
	return operator+=( str.c_str() );
}

CString& CString::operator<<(const char* str)
{
	return operator+=( str );
}

CString& CString::operator<<(const CString& text)
{
	return operator+=(text);
}

bool CString::operator==(const CString& str) const
{
	if( d == str.d )
		return true;
	if (d == 0 || str.d == 0)
	{
		if( d == 0 && str.length() == 0 )
			return true;
		if( this->length() == 0 && str.d == 0 )
			return true;
		return false;
	}
	if( d->m_length != str.d->m_length )
		return false;
	if( !memcmp(d->m_data, str.d->m_data, d->m_length) )
		return true;
	return false;
}

bool CString::operator!=(const CString& str) const
{
	return !operator==( str );
}

bool CString::operator<(const CString& str) const
{
	const char* data = this->data();
	const char* data1 = str.data();
    
	if( !data && !data1 )
		return false;
	else if( !data && data1 )
		return true;
	else if( data && !data1 )
		return false;
	else if( !data || !data1 )
		return false;
	if( 0 > strcmp(d->m_data, str.d->m_data) )
		return true;
	return false;
}

bool CString::operator<(const char* str) const
{
    const char* data = this->data();
    const char* data1 = str;

    if (!data && !data1)
        return false;
    else if (!data && data1)
        return true;
    else if (data && !data1)
        return false;
    else if (!data || !data1)
        return false;
    if (0 > strcmp(d->m_data, str) )
        return true;
    return false;
}

CString& CString::prepend(const CString& str)
{
	if( !str.d )
		return *this;
	makeNewDataIfNeed();
	d->prepend( *str.d );
	return *this;
}

CString& CString::prepend(const char* str)
{
	if( !str )
		return *this;
	makeNewDataIfNeed();
	d->prepend( str );
	return *this;
}

CString& CString::insert(const CString& what, int from)
{
	if( !what.d )
		return *this;
	makeNewDataIfNeed();
	from = utf8ByteOffset( d->m_data, from, d->m_length );
	d->insert( *what.d, from );
	return *this;
}

CString& CString::insert(const char* what, int from)
{
	if( !what )
		return *this;
	makeNewDataIfNeed();
	from = utf8ByteOffset( d->m_data, from, d->m_length );
	d->insert( what, from );
	return *this;
}

CString& CString::remove(int start, int len)
{
	if( 0 > start )
		return *this;
	if( !d )
		return *this;
	makeNewDataIfNeed();
	// получаем размеры в байтах
	start = utf8ByteOffset( d->m_data, start, d->m_length );
	int offset = 0;
	char* data = d->m_data+start;
	for( int i = 0;i<len;i++ )
	{
		offset += g_utf8_skip[static_cast<unsigned char>(*data)];
		data += g_utf8_skip[static_cast<unsigned char>(*data)];
	}
	d->remove( start, offset );
	return *this;
}

CString& CString::remove(const RANGE& range)
{
	return remove( range.location, range.length );
}

CString& CString::remove(const CString& str)
{
	if( !str.d )
		return *this;
	return remove( find(str), str.d->m_numChars );
}

CString& CString::remove(const char* str)
{
	if( !str )
		return*this;
	size_t len = strlen( str );
	if( !len )
		return *this;
	return remove( find(str), (int)g_utf8_pointer_to_offset(str, str+len) );
}

CString& CString::replace(const CString& subject, const CString& replacement)
{
	int start = find( subject );
	if( 0 > start )
		return *this;
	remove( start, subject.d->m_numChars );
	insert( replacement, start );
	return *this;
}

CString& CString::replace(const char* subject, const char* replacement)
{
	if( !subject )
		return *this;
	int start = find( subject );
	if( 0 > start )
		return *this;
	int len = (int)strlen( subject );
	if( !len )
		return *this;
	remove( start, (int)g_utf8_pointer_to_offset(subject, subject+len) );
	insert( replacement, start );
	return *this;
}

CString& CString::replaceAll(const CString& subject, const CString& replacement)
{
	int start = find( subject );
	while( start > 0 )
	{
		remove( start, subject.d->m_numChars );
		insert( replacement, start );
		start = find( subject );
	}
    
	return *this;
}

bool CString::contains(const CString& str) const
{
	if( d == str.d )
		return true;
	if( !d || !str.d )
		return false;
	return d->contains( *str.d );
}

bool CString::contains(const char* str) const
{
	if( !d || !str )
		return false;
	return d->contains( str );
}

int CString::indexOf(const CString& str, int start) const
{
	return find( str, start );
}

int CString::lastIndexOf(const CString& str, int start) const
{
	if( isEmpty() || str.isEmpty() )
		return -1;
	int lastIndex = 0;
	int i = 0;
	while( i != -1 )
	{
		i = indexOf( str, i+1 );
		if( i >= start || -1 == i )
			return lastIndex;
		lastIndex = i;
	}
	return lastIndex;
}

bool CString::startsWith(const CString& str) const
{
    if (str.length() == 0)
        return false;
    int start = this->find(str);
    return start == 0 ? true : false;
}

CString CString::toLower() const
{
	CString str = *this;
	if( !str.d )
		return str;
	str.makeNewDataIfNeed();
	char* result = g_utf8_strdown( str.d->m_data, str.d->m_length );
	if( !result )
		return str;
	*str.d = result;
	my_free( result );
	return str;
}

CString CString::toUpper() const
{
	CString str = *this;
	if( !str.d )
		return str;
	str.makeNewDataIfNeed();
	char* result = g_utf8_strup( str.d->m_data, str.d->m_length );
	if( !result )
		return str;
	*str.d = result;
	my_free( result );
	return str;
}

CString CString::arg(const CString& argument ) const
{
	RANGE range = findMinArg();
	if( 0 > range.location || !range.length )
	{
		assert(false);
		return CString();
	}
	CString str = *this;
	str.remove( range ).insert( argument, range.location );
	return str;
}

CString CString::arg(uint64_t argument, int width, int base /*= 10*/, char fillChar /*= ' '*/) const
{
	return arg( CString(argument, width, base, fillChar) );
}

int64_t CString::toInt64(bool* success /*= 0*/, int base /*= 10*/) const
{
	char* buffer = 0;
    buffer = makeArrayForIntConvert();
	if( !buffer ) {
        if (success)
            *success = false;
		return 0;
    }
    if (success)
        *success = true;
	long int result = strtol(buffer, 0, base);
	my_free(buffer);
	return result;
}

uint64_t CString::toUInt64() const
{
	return (uint64_t)toInt64();
}

double CString::toDouble() const
{
	char* buffer = makeArrayForIntConvert();
	if( !buffer )
		return 0.0;
	double result = atof( buffer );
	my_free( buffer );
	return result;
}

bool CString::toBool() const
{
	if( !d )
		return false;
	if( !d->m_length || !d->m_data )
		return false;
	if( *this == "true" )
		return true;
	if( *this == "0" )
		return false;
	return true;
}

/////////////////////////////////////////////////////////////////////
//                  find and highlight a substring                 //
/////////////////////////////////////////////////////////////////////

int CString::find(const CString& what, uint32_t from /*= 0*/) const
{
	if( !d || !what.d )
		return -1;
	from = utf8ByteOffset( d->m_data, from, d->m_length );
	if( -1 == (int)from )
		return -1;
	int val = d->find( *what.d, from );
	if( -1 == val )
		return -1;
	return (int)g_utf8_pointer_to_offset( d->m_data, d->m_data+val );
}

int CString::find( const char* what, uint32_t from /*= 0*/ ) const
{
	if( !d )
		return -1;
	from = utf8ByteOffset( d->m_data, from, d->m_length );
	if( -1 == (int)from )
		return -1;
	int val = d->find( what, from );
	if( -1 == val )
		return -1;
	return (int)g_utf8_pointer_to_offset( d->m_data, d->m_data+val );
}

CString CString::substr( int from, int to /*= -1*/ ) const
{
	if( 0 > from || from > this->length() )
		return CString();
    if (!d || !d->m_data)
        return CString();
	from = (int)g_utf8_pointer_to_offset( d->m_data, d->m_data+from );
	if( -1 == to )
		to = this->length();
	if( 0 > to || to > this->length() )
		return CString();
	to = (int)g_utf8_pointer_to_offset( d->m_data, d->m_data+to );
	int len = to-from;
	if( 0 > len )
		return CString();
	CString newStr;
	newStr.makeNewDataIfNeed();
	newStr.d->setSize( len );
	memcpy( newStr.d->m_data, d->m_data+from, len );
	newStr.d->m_data[len] = 0x0;
	newStr.d->recalcLength();
	return newStr;
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

void CString::makeNewDataIfNeed()
{
	if( d && (1 == d->refs()) )
		return;
	if( d && (1 != d->refs()) )
	{
		CStringPrivate* data = new CStringPrivate( *d );
		if( !data )
		{
			assert( 0 == "not enought memory" );
			return;
		}
		d->release();
		d = data;
	}
	else
	{
		assert( !d );
		d = new CStringPrivate();
	}
	assert( d );
	if( d )
		d->addRef();
}

RANGE CString::findMinArg() const
{
	int minVal = -1;
	RANGE range;
	range.location = -1;
	range.length   = 0;
    
	if( !d )
		return range;
	if( !d->m_length || !d->m_data )
		return range;
    
	int length = this->length();
	int i, a;
	uint32_t c;
	uint32_t* buffer;
	char* data;
	RANGE calc;
    
	for( i = 0;i<length;i++ )
	{
		c = this->at( i );
		if( c == '%' )
		{
			calc.location = i+1;
			for( a = i+1;a<length;a++ )
			{
				c = this->at( a );
				if( !isdigit(c) )
					break;
			}
			calc.length = a-calc.location;
			if( !calc.length )
				continue;
			buffer = new uint32_t(calc.length);
			assert( buffer );
			if( buffer )
			{
				this->getCharacters( buffer, calc );
				data = toCharArray( buffer, int(calc.length) );
				if( data )
				{
					int val = atoi( data );
					if( -1 == minVal || minVal > val )
					{
						minVal = val;
						range = calc;
						range.location--;
						range.length++;
					}
					my_free( data );
				}
				delete []( buffer );
			}
		}
	}
	return range;
}

// конвертирует unichar* массив в char*
// (в массиве должны быть ТОЛЬКО числа)
char* CString::toCharArray(uint32_t* buffer, int length ) const
{
	if( !buffer || !length )
		return 0;
	char* data = (char*)my_malloc( length+1 );
	if( !data )
		return 0;
	for( int i = 0;i<length;i++ )
		data[i] = (char)buffer[i];
	data[length] = 0x0;
	return data;
}

// собирает char* для конвертации в число
char* CString::makeArrayForIntConvert() const
{
	if( !d )
		return 0;
	if( !d->m_length || !d->m_data )
		return 0;
    
	char* p = d->m_data;
	int length = this->length();
	uint32_t c;
	// для начала проверим на валидность строки
	while( *p )
	{
		c = g_utf8_get_char( p );
		if( !isdigit(c) && c != '.' && c != ',' )
        {
            if ((c < 'a' || c > 'f') &&
                (c < 'A' || c > 'F'))
                return 0;
        }
		p = g_utf8_next_char( p );
	}
	char* buffer = (char*)my_malloc( length+1 );
	if( !buffer )
		return 0;
	p = d->m_data;
	int i = 0;
	while( *p )
	{
		c = g_utf8_get_char( p );
		if( c == ',' )
			c = '.';
		buffer[i] = (char)c;
		i++;
		p = g_utf8_next_char( p );
	}
	buffer[length] = 0x0;
	return buffer;
}

bool CString::isWhitespace(uint32_t c) const
{
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
        return true;
    return false;
}

//===================================================================
// глобальные операторы сравнения
//===================================================================

bool operator==( const CString& str, const char* str1 )
{
	return str.operator ==( str1 );
}

bool operator==( const char* str1, const CString& str )
{
	return str.operator ==(str1 );
}

const CString operator+(const CString &s1, const CString &s2)
{
	CString t(s1); t += s2; return t;
}

const CString operator+(const CString &s1, const char* s2)
{
	CString t(s1); t += CString(s2); return t;
}
