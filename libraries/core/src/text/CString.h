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

#ifndef __core__cstring__
#define __core__cstring__

#include <stdint.h>
#include <string>
#include "../core/CByteArray.h"

class CStringPrivate;

class CString
{
public:
    CString();
    CString(const std::string& str);
    CString(const CString& other);
    CString(const char* str, int len = -1);
    CString(char ch);
    CString(uint64_t ui64, int width = -1, int base = 10, char fillChar = ' ');
    CString(int64_t i64);
    CString(uint32_t ui64);
    CString(int32_t i32);
    CString(uint16_t ui16);
    CString(int16_t i16);
    CString(double val, int precision = 0);
    ~CString();

    bool isEmpty() const;
    // return the number of characters per string
    int length() const;
    // returns the size of the strung
    int size() const;
    const char* data() const;
    bool getCharacters(uint32_t* buffer, const RANGE& range) const;
    uint32_t at(int i) const;
    uint32_t operator[](int i) const;
    CString left(int size) const;
    CString right(int size) const;
    CString mid(int start, int num = -1);
    CString trimmed() const;

    CString& operator=(const CString& other);
    CString& operator=(const char* str);

    CString& operator+=(const CString& str);
    CString& operator+=(const char* str);
    CString& operator+=(char val);
    CString& operator+=(uint64_t ui64);
    CString& operator+=(int64_t i64);
    CString& operator+=(uint32_t ui32);
    CString& operator+=(int32_t i32);
    CString& operator+=(uint16_t ui16);
    CString& operator+=(int16_t i16);
    CString& operator+=(double val);

    CString& operator<<(const char* str);
    CString& operator<<(const CString& text);

    template<typename T>
    CString& operator<<(const T& val) {
        return operator+=(val);
    }

    bool operator==(const CString& str) const;
    bool operator!=(const CString& str) const;
    bool operator<(const CString& str) const;
    bool operator<(const char* str) const;

    CString& prepend(const CString& str);
    CString& prepend(const char* str);
    CString& insert(const CString& what, int from);
    CString& insert(const char* what, int from);
    CString& remove(int start, int len);
    CString& remove(const RANGE& range);
    CString& remove(const CString& str);
    CString& remove(const char* str);
    CString& replace(const CString& subject, const CString& replacement);
    CString& replace(const char* subject, const char* replacement);
    CString& replaceAll(const CString& subject, const CString& replacement);
    bool contains(const CString& str) const;
    bool contains(const char* str) const;
    int  indexOf(const CString& str, int start) const;
    int  lastIndexOf(const CString& str, int start) const;
    bool startsWith(const CString& str) const;

    CString toLower() const;
    CString toUpper() const;

    CString arg(const CString& argument) const;
    CString arg(uint64_t argument, int width, int base = 10, char fillChar = ' ') const;

    int64_t toInt64(bool* success = 0, int base = 10) const;
    uint64_t toUInt64() const;
    double toDouble() const;
    bool toBool() const;

    // find and highlight a substring
    int find(const CString& what, uint32_t from = 0) const;
    int find(const char* what, uint32_t from = 0) const;
    CString substr(int from, int to = -1) const;

private:
    CStringPrivate * d;
    
    void makeNewDataIfNeed();
    RANGE findMinArg() const;
    // converts unichar* array in char*
    // (in the array must be numbers only)
    char* toCharArray(uint32_t* buffer, int length) const;
    // collect char* to convert to number
    char* makeArrayForIntConvert() const;
    bool isWhitespace(uint32_t c) const;
};

bool operator==( const CString& str, const char* str1 );
bool operator==( const char* str1, const CString& str );
const CString operator+(const CString &s1, const CString &s2);
const CString operator+(const CString &s1, const char* s2);

#endif /* defined(__core__cstring__) */
