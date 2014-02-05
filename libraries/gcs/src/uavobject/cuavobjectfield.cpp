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

#include "cuavobjectfield.h"
#include "system/Logger"
#include <assert.h>

#include "my_memory"

CUAVObjectField::CUAVObjectField()
{
	m_fieldId     = 0;
	m_owner       = 0;
	m_type        = FieldType_INT8;
	m_elemSize    = 0;
	m_numElements = 1;
}

CUAVObjectField::~CUAVObjectField()
{
}

/////////////////////////////////////////////////////////////////////
//                        базовые параметры                        //
/////////////////////////////////////////////////////////////////////

void CUAVObjectField::setFieldId(uint32_t fieldId)
{
	m_fieldId = fieldId;
}

uint32_t CUAVObjectField::fieldId() const
{
	return m_fieldId;
}

void CUAVObjectField::setOwner(CUAVObject* object)
{
	m_owner = object;
}

CUAVObject* CUAVObjectField::owner()
{
	return m_owner;
}

void CUAVObjectField::setName(const CString& name)
{
	m_name = name;
}

CString CUAVObjectField::name() const
{
	return m_name;
}

void CUAVObjectField::setUnits(const CString& units)
{
	m_units = units;
}

CString CUAVObjectField::units() const
{
	return m_units;
}

/////////////////////////////////////////////////////////////////////
//                               value                             //
/////////////////////////////////////////////////////////////////////

void CUAVObjectField::setType(FieldType type)
{
	m_type = type;
	switch (type) {
		case FieldType_INT8:
			m_elemSize = sizeof(int8_t);
			break;
		case FieldType_INT16:
			m_elemSize = sizeof(int16_t);
			break;
		case FieldType_INT32:
			m_elemSize = sizeof(int32_t);
			break;
		case FieldType_UINT8:
			m_elemSize = sizeof(uint8_t);
			break;
		case FieldType_UINT16:
			m_elemSize = sizeof(uint16_t);
			break;
		case FieldType_UINT32:
			m_elemSize = sizeof(uint32_t);
			break;
		case FieldType_Float32:
			m_elemSize = sizeof(float);
			break;
		case FieldType_Enum:
			m_elemSize = sizeof(uint8_t);
			break;
		case FieldType_String:
			m_elemSize = 0;
			break;
		case FieldType_BitField:
			m_elemSize = 1;
			assert(false);
			break;
	}
	calcDataSize();
}

CUAVObjectField::FieldType CUAVObjectField::type() const
{
	return m_type;
}

CString CUAVObjectField::typeAsString() const
{
	switch (m_type) {
		case FieldType_INT8:
			return CString("int8");
		case FieldType_INT16:
			return CString("int16");
		case FieldType_INT32:
			return CString("int32");
		case FieldType_UINT8:
			return CString("uint8");
		case FieldType_UINT16:
			return CString("uint16");
		case FieldType_UINT32:
			return CString("uint32");
		case FieldType_Float32:
			return CString("float");
		case FieldType_Enum:
			return CString("enum");
		case FieldType_String:
			return CString("string");
		case FieldType_BitField:
			return CString("bitfield");
	}
	return CString();
}

void CUAVObjectField::setNumElements(size_t num)
{
	m_numElements = num;
	calcDataSize();
}

size_t CUAVObjectField::numElements() const
{
	return m_numElements;
}

void CUAVObjectField::setValue(const CVariant& value, uint32_t element /*= 0*/)
{
	if (element >= m_numElements) {
		return;
	}
	switch (m_type) {
		case FieldType_INT8:
		{
			int8_t* data = (int8_t*)m_data.data();
			data[element] = int8_t(value.toInt());
		}
			break;
		case FieldType_INT16:
		{
			int16_t* data = (int16_t*)m_data.data();
			data[element] = int16_t(value.toInt());
		}
			break;
		case FieldType_INT32:
		{
			int32_t* data = (int32_t*)m_data.data();
			data[element] = int32_t(value.toInt());
		}
			break;
		case FieldType_UINT8:
		{
			uint8_t* data = (uint8_t*)m_data.data();
			data[element] = uint8_t(value.toInt());
		}
			break;
		case FieldType_UINT16:
		{
			uint16_t* data = (uint16_t*)m_data.data();
			data[element] = uint16_t(value.toInt());
		}
			break;
		case FieldType_UINT32:
		{
			uint32_t* data = (uint32_t*)m_data.data();
			data[element] = uint32_t(value.toInt());
		}
			break;
		case FieldType_Float32:
		{
			float* data = (float*)m_data.data();
			data[element] = float(value.toDouble());
		}
			break;
		case FieldType_Enum:
		{
			assert(false);
		}
			break;
		case FieldType_String:
			m_strings[element] = value.toString();
			break;
		case FieldType_BitField:
			assert(false);
			break;
	}
}

CVariant CUAVObjectField::value(uint32_t element /*= 0*/) const
{
	if (element >= m_numElements) {
		return CVariant();
	}
	switch (m_type) {
		case FieldType_INT8:
		{
			int8_t* data = (int8_t*)m_data.data();
			return CVariant(data[element]);
		}
		case FieldType_INT16:
		{
			int16_t* data = (int16_t*)m_data.data();
			return CVariant(data[element]);
		}
		case FieldType_INT32:
		{
			int32_t* data = (int32_t*)m_data.data();
			return CVariant(data[element]);
		}
		case FieldType_UINT8:
		{
			uint8_t* data = (uint8_t*)m_data.data();
			return CVariant(data[element]);
		}
		case FieldType_UINT16:
		{
			int16_t* data = (int16_t*)m_data.data();
			return CVariant(data[element]);
		}
		case FieldType_UINT32:
		{
			uint32_t* data = (uint32_t*)m_data.data();
			return CVariant(data[element]);
		}
		case FieldType_Float32:
		{
			float* data = (float*)m_data.data();
			return CVariant(data[element]);
		}
		case FieldType_Enum:
		{
			assert(false);
			return CVariant();
		}
		case FieldType_String:
		{
			return m_strings[element];
		}
		case FieldType_BitField:
		{
			assert(false);
		}
	}
	return CVariant();
}


void CUAVObjectField::toLogger()
{
	Logger() << "field name:" << m_name << ", num elems = " << m_numElements;
	Logger log;
	log << "\t";
	for (size_t i = 0;i<m_numElements;i++) {
		log << value(i);
	}
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

void CUAVObjectField::calcDataSize()
{
	if (m_type == FieldType_String) {
		m_strings.resize(m_numElements);
		m_data.setSize(0);
	} else if (m_type == FieldType_BitField) {
		m_strings.clear();
		assert(false);
	} else {
		m_strings.clear();
		m_data.setSize(uint32_t(m_numElements*m_elemSize));
		memset(const_cast<char*>(m_data.data()), 0, m_numElements);
	}
}
