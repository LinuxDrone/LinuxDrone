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

#ifndef __gcs__uavobjectfield__
#define __gcs__uavobjectfield__

#include "core/CObject"
#include "text/CString"
#include "core/CVariant"

class CUAVObject;

class CUAVObjectField : public CObject
{
public:
	enum FieldType
	{
		FieldType_INT8,
		FieldType_INT16,
		FieldType_INT32,
		FieldType_UINT8,
		FieldType_UINT16,
		FieldType_UINT32,
		FieldType_Float32,
		FieldType_Enum,		// реализовать
		FieldType_String,
		FieldType_BitField	// реализовать
	};

public:
    CUAVObjectField();
    virtual ~CUAVObjectField();
	
// базовые параметры
	void setFieldId(uint32_t fieldId);
	uint32_t fieldId() const;
	void setOwner(CUAVObject* object);
	CUAVObject* owner();
	void setName(const CString& name);
	CString name() const;
	void setUnits(const CString& units);
	CString units() const;

// value
	void setType(FieldType type);
	FieldType type() const;
	CString typeAsString() const;
	void setNumElements(size_t num);
	size_t numElements() const;
	void setValue(const CVariant& value, uint32_t element = 0);
	CVariant value(uint32_t element = 0) const;

	void toLogger();

private:
	uint32_t               m_fieldId;
	CUAVObject           * m_owner;
	CString                m_name;
	CString                m_units;
	FieldType              m_type;
	size_t                 m_elemSize;
	uint32_t               m_numElements;
	CByteArray             m_data;
	std::vector<CString>   m_strings;

	void calcDataSize();
};

#endif /* defined(__gcs__uavobjectfield__) */
