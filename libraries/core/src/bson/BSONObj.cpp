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

#include "BSONObj.h"
#include "BSONObjIterator.h"

BSONObj::BSONObj()
{
}

BSONObj::BSONObj(const char* data)
{
    if (data == 0) {
        return;
    }
    int size = *((int*)data);
    m_data = CByteArray(data, size);
    enumElements();
}

BSONObj::BSONObj(const BSONObj& other)
{
    operator=(other);
}

BSONObj::~BSONObj()
{
}

BSONObj& BSONObj::operator=(const BSONObj& other)
{
    m_data = other.m_data;
    enumElements();
    return *this;
}

bool BSONObj::isEmpty() const
{
    return m_data.size() <= 5;
}

CByteArray BSONObj::data() const
{
    return m_data;
}

void BSONObj::toString(CString& s, bool isArray /*= false*/, bool full /*= false*/) const
{
    if (isEmpty()) {
        s << "{}";
        return;
    }
    
    s << (isArray ? "[" : "{");

    BSONObjIterator it(*this);
    bool first = true;
    while (it.more()) {
        if (first) {
            first = false;
        } else {
            s << ", ";
        }
        BSONElement elem = it.next();
        elem.toString(s, !isArray, full);
    }
    
    s << (isArray ? "]" : "}");
}

CString BSONObj::toString(bool isArray /*= false*/, bool full /*= false*/) const
{
    CString s;
    toString(s, isArray, full);
    return s;
}

/////////////////////////////////////////////////////////////////////
//                              elements                           //
/////////////////////////////////////////////////////////////////////

size_t BSONObj::numFields() const
{
    return m_elements.size();
}

BSONElement BSONObj::operator[](const CString& name) const
{
    if (m_elements.count(name) == 0) {
        return BSONElement();
    }
    return m_elements.at(name);
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

void BSONObj::enumElements()
{
    m_elements.clear();
    BSONObjIterator it(*this);
    while (it.more()) {
        BSONElement elem = it.next();
        m_elements[elem.name()] = elem;
    }
}
