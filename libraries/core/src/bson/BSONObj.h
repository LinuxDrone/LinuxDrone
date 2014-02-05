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

#ifndef __core__bsonobj__
#define __core__bsonobj__

#include "BSONElement.h"
#include "../core/CByteArray.h"
#include <map>

class BSONObj
{
public:
    BSONObj();
    BSONObj(const char* data);
    BSONObj(const BSONObj& other);
    ~BSONObj();

    BSONObj& operator=(const BSONObj& other);
    
    bool isEmpty() const;
    CByteArray data() const;
    void toString(CString& s, bool isArray = false, bool full = false) const;
    CString toString(bool isArray = false, bool full = false) const;
    
// elements
    size_t numFields() const;
    BSONElement operator[](const CString& name) const;

private:
    CByteArray                     m_data;
    std::map<CString, BSONElement> m_elements;
    
    void enumElements();
};

#endif /* defined(__core__bsonobj__) */
