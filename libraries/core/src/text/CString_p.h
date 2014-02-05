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

#ifndef __core__cstring_p__
#define __core__cstring_p__

#include <stdint.h>

class CStringPrivate
{
public:
    CStringPrivate();
    CStringPrivate(const CStringPrivate& src);
    ~CStringPrivate();

    uint32_t addRef();
    uint32_t release();
    uint32_t refs() const;
    
    void setSize(int size);
    void clear();
    
    CStringPrivate& operator=(const CStringPrivate& src);
    CStringPrivate& operator=(const char* str);
    
    CStringPrivate& operator+=(const CStringPrivate& other);
    CStringPrivate& operator+=(const char* str );
    
    void prepend(CStringPrivate& str);
    void prepend(const char* str);
    void insert(const CStringPrivate& str, int start);
    void insert(const char* str, int start);
    void remove(int start, int len);
    
    bool contains(const CStringPrivate& str) const;
    bool contains(const char* str) const;
    int find(const CStringPrivate& what, int from = 0);
    int find(const char* what, int from = 0);
    
    int length() const;
    int numChars() const;
    
public:
    uint32_t   m_refs;
    char     * m_data;
    int        m_length;
    int        m_numChars;
    
    void recalcLength();
};

#endif /* defined(__core__cstring_p__) */
