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

#ifndef __core__BSONElement__
#define __core__BSONElement__

#include "BSONType.h"
#include "../core/CByteArray.h"
#include "../text/CString.h"

class BSONObj;

class BSONElement
{
public:
    BSONElement();
    BSONElement(const BSONElement& other);
    BSONElement(const char* data);
    
    BSONElement& operator=(const BSONElement& other);
    
    static int calcSize(const char* data);

    bool isEmpty() const;
    BSONType type() const;
    bool isObject() const;
    int size() const;
    CString name() const;

    void toString(CString& s, bool includeFieldName = true, bool full = false) const;
    CString toString(bool includeFieldName = true, bool full = false) const;

// Values
    bool isNumber() const;
    const char* value() const;
    BSONObj Obj() const;
    CString String() const;
    double Number() const;
    double Double() const;
    int64_t Long() const;
    int Int() const;
    bool Bool() const;
    double numberDouble() const;
    
private:
    CByteArray m_data;
    int        m_nameSize;
    
    bool boolean() const;
    int valueStrSize() const;
    const char* valueStr() const;
    const BSONElement& chk(BSONType t) const;
    const BSONElement& chk(bool val) const;
};

#endif /* defined(__core__BSONElement__) */
