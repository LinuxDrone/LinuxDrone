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
#include "BSONElement.h"
#include "../system/Logger.h"
#include <assert.h>
#include <string.h>

BSONElement::BSONElement()
{
    m_nameSize = 0;
}

BSONElement::BSONElement(const BSONElement& other)
{
    operator=(other);
}

BSONElement::BSONElement(const char* data)
{
    if (data == 0) {
        return;
    }
    int size = calcSize(data);
    if (size == 0) {
        return;
    }
    m_data = CByteArray(data, size, true);
    m_nameSize = int (strlen(data+1)+1);
}

BSONElement& BSONElement::operator=(const BSONElement& other)
{
    m_data = other.m_data;
    m_nameSize = other.m_nameSize;
    return *this;
}

int BSONElement::calcSize(const char* data)
{
    if (data == 0) {
        return 0;
    }
    char c = *data;
    BSONType type = (BSONType)c;

    int nameSize = int (strlen(data+1))+1;
    const char* value = (data + nameSize + 1);
    
    int size = 0;
    switch (type) {
        case BSONType_EOO:
        case BSONType_Undefined:
        case BSONType_jstNULL:
        case BSONType_MinKey:
        case BSONType_MaxKey:
            break;
        case BSONType_Bool:
            size = 1;
            break;
        case BSONType_NumberInt:
            size = 4;
            break;
        case BSONType_Timestamp:
        case BSONType_Date:
        case BSONType_NumberLong:
        case BSONType_NumberDouble:
            size = 8;
            break;
        case BSONType_jstOID:
            size = 12;
            break;
        case BSONType_Code:
        case BSONType_Symbol:
        case BSONType_String:
            size = *((int*)value)+4;
            break;
        case BSONType_DBRef:
            size = *((int*)value)+4+12;
            break;
        case BSONType_CodeWScope:
        case BSONType_Object:
        case BSONType_Array:
            size = *((int*)value);
            break;
        case BSONType_BinData:
            size = *((int*)value)+4+1;
            break;
        case BSONType_RegEx:
        {
            const char *p = value;
            size_t len1 = strlen(p);
            p = p + len1 + 1;
            size_t len2;
            len2 = strlen(p);
            size = (int) (len1 + 1 + len2 + 1);
        }
            break;
            
        default:
            Logger() << "BSONElement: bad type " << int (type);
            assert(false);
            break;
    }
    size += nameSize + 1;
    return size;
}

bool BSONElement::isEmpty() const
{
    return m_data.isEmpty();
}

BSONType BSONElement::type() const
{
    if (m_data.isEmpty()) {
        return BSONType_MinKey;
    }
    char c = *m_data.data();
    return (BSONType)c;
}

bool BSONElement::isObject() const
{
    if (isEmpty()) {
        return false;
    }
    switch (type()) {
        case BSONType_Object:
        case BSONType_Array:
            return true;
            
        default:
            break;
    }
    return false;
}

int BSONElement::size() const
{
    return m_data.size();
}

CString BSONElement::name() const
{
    if (isEmpty()) {
        return CString();
    }
    return CString(m_data.data()+1);
}

void BSONElement::toString(CString& s, bool includeFieldName /*= true*/, bool full /*= false*/) const
{
    if (includeFieldName && type() != BSONType_EOO) {
        s << name() << ": ";
    }
    switch (type()) {
        case BSONType_MinKey:
            s << "MinKey";
            break;
        case BSONType_EOO:
            s << "EOO";
            break;
        case BSONType_NumberDouble:
            s << *((double*)value());
            break;
        case BSONType_Symbol:
        case BSONType_String:
            s << CString("\"");
            if (!full &&  valueStrSize() > 160) {
                s << CString(valueStr(), 150);
                s << "...\"";
            }
            else {
                s << CString(valueStr(), valueStrSize()-1);
                s << CString("\"");
            }
            break;
        case BSONType_Object:
        case BSONType_Array:
            Obj().toString(s, false, full);
            break;
        case BSONType_BinData:
            s << "Bin data. unsupport";
            break;
        case BSONType_Undefined:
            s << "Undefined";
            break;
        case BSONType_jstOID:
            s << "jstOID. unsupported";
            break;
        case BSONType_Bool:
            s << boolean();
            break;
        case BSONType_Date:
            s << "Date. unsupported";
            break;
        case BSONType_jstNULL:
            s << "null";
            break;
        case BSONType_RegEx:
            s << "RegExp. unsupported";
            break;
        case BSONType_DBRef:
            s << "DBRef. unsupported";
            break;
        case BSONType_Code:
            s << "Code. unsupported";
            break;
        case BSONType_CodeWScope:
            s << "CodeWScope. unsupported";
            break;
        case BSONType_NumberInt:
            s << *((int*)value());
            break;
        case BSONType_Timestamp:
            s << "TimeStamp. unsupported";
            break;
        case BSONType_NumberLong:
            s << *((int64_t*)value());
            break;
        case BSONType_MaxKey:
            s << "MaxKey";
            break;
    }
}

CString BSONElement::toString(bool includeFieldName /*= true*/, bool full /*= false*/) const
{
    CString s;
    toString(s, includeFieldName, full);
    return s;
}

/////////////////////////////////////////////////////////////////////
//                               Values                            //
/////////////////////////////////////////////////////////////////////

bool BSONElement::isNumber() const
{
    return false;
}

const char* BSONElement::value() const
{
    return m_data.data()+m_nameSize+1;  // type
}

BSONObj BSONElement::Obj() const
{
    if (!isObject()) {
        Logger() << "invalid parameter: expected an object (" << name() << ")";
        return BSONObj();
    }
    const char* val = value();
    return BSONObj(val);
}

CString BSONElement::String() const
{
    return chk(BSONType_String).valueStr();
}

double BSONElement::Number() const
{
    switch (type())
    {
        case BSONType_NumberDouble:
        case BSONType_NumberInt:
        case BSONType_NumberLong:
            return true;
            
        default:
            break;
    }
    return false;
}

double BSONElement::Double() const
{
    return *((double*)chk(BSONType_NumberDouble).value());
}

int64_t BSONElement::Long() const
{
    return *((double*)chk(BSONType_NumberLong).value());
}

int BSONElement::Int() const
{
    return *((double*)chk(BSONType_NumberInt).value());
}

bool BSONElement::Bool() const
{
    return *((double*)chk(BSONType_Bool).boolean());
}

double BSONElement::numberDouble() const
{
    switch( type() ) {
        case BSONType_NumberDouble:
            return *reinterpret_cast<const double*>(value());
        case BSONType_NumberInt:
            return *reinterpret_cast< const int* >(value());
        case BSONType_NumberLong:
            return (double) *reinterpret_cast<const long long*>(value());
        default:
            return 0;
    }
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

bool BSONElement::boolean() const
{
    return *value() ? true : false;
}

int BSONElement::valueStrSize() const
{
    return *((int*)value());
}

const char* BSONElement::valueStr() const
{
    return value()+4;
}

const BSONElement& BSONElement::chk(BSONType t) const
{
    if (t != type()) {
        Logger() << "wrong type for field (" << name() << ") " << type() << " != " << t;
        assert(false);
    }
    return *this;
}

const BSONElement& BSONElement::chk(bool val) const
{
    if (!val) {
        Logger() << "nexpected or missing type value in BSON object";
        assert(false);
    }
    return *this;
}
