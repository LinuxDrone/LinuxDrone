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

#ifndef __gcs__uavobject__
#define __gcs__uavobject__

#include "core/CObject"
#include "text/CString"
#include "cuavobjectfield.h"
#include <map>

class CUAVObjectField;

class CUAVObject : public CObject
{
public:
    CUAVObject();
    virtual ~CUAVObject();

    static CUAVObject* createObject(const CString& name = CString());

// базовые параметры
    void setName(const CString& name);
    CString name() const;
    void setObjectId(uint32_t objId);
    uint32_t objectId() const;
    void setInstanceId(uint32_t instId);
    uint32_t instanceId() const;
    void setParent(CUAVObject* parent);
    CUAVObject* parent();
	CUAVObject* mainObject();			// возвращает указатель на родителя дерева
    std::vector<uint32_t> makePathToObject() const;
	
// instance
	CUAVObject* createInstance();
	void addInstance(CUAVObject* instance);
	void removeAllInstances();
	bool removeInstanceById(uint32_t instId);
	int numInstances() const;
	CUAVObject* instanceById(uint32_t instId);
	const std::vector<uint32_t>& instnces() const;
	
// fields
	CUAVObjectField* createField(const CString& name = CString(), CUAVObjectField::FieldType type = CUAVObjectField::FieldType_UINT8, size_t numElements = 1);
	void addField(CUAVObjectField* field);
	void removeAllFields();
	bool removeFieldById(uint32_t fieldId);
	size_t numFields() const;
	CUAVObjectField* fieldById(uint32_t fieldId);
	CUAVObjectField* fieldByName(const CString& name);
	const std::vector<uint32_t>& fieldIds() const;

private:
    CString      m_name;
    uint32_t     m_objId;
    uint32_t     m_instId;
    CUAVObject * m_parent;
	
	std::map<uint32_t, CUAVObject*> m_instances;
	std::vector<uint32_t> m_instIds;
	uint32_t              m_lastInstId;
	std::vector<uint32_t> m_freeInstId;

	std::map<uint32_t, CUAVObjectField*> m_fields;
	std::vector<uint32_t>               m_fieldIds;
	uint32_t              m_lastFieldId;
	std::vector<uint32_t> m_freeFieldId;
	
	uint32_t allocIdForInstance();
	void freeInstanceId(uint32_t intsId);
	uint32_t allocIdForField();
	void freeFieldId(uint32_t fieldId);
};

#endif /* defined(__gcs__uavobject__) */
