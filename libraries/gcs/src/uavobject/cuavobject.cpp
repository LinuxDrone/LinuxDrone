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

#include "cuavobject.h"
#include "cuavobjectfield.h"
#include "my_memory"

CUAVObject::CUAVObject()
{
    m_name        = "unknown name";
    m_objId       = 0;
    m_instId      = 0;
    m_parent      = 0;
	m_lastInstId  = 0;
	m_lastFieldId = 0;
}

CUAVObject::~CUAVObject()
{
	removeAllInstances();
	removeAllFields();
}

CUAVObject* CUAVObject::createObject(const CString& name)
{
	CUAVObject* obj = new CUAVObject();
	if (!obj) {
		return 0;
	}
	obj->addRef();
	obj->setName(name);
	return obj;
}

/////////////////////////////////////////////////////////////////////
//                         базовые параметры                       //
/////////////////////////////////////////////////////////////////////

void CUAVObject::setName(const CString& name)
{
    m_name = name;
}

CString CUAVObject::name() const
{
    return m_name;
}

void CUAVObject::setObjectId(uint32_t objId)
{
    m_objId = objId;
}

uint32_t CUAVObject::objectId() const
{
    return m_objId;
}

void CUAVObject::setInstanceId(uint32_t instId)
{
    m_instId = instId;
}

uint32_t CUAVObject::instanceId() const
{
    return m_instId;
}

void CUAVObject::setParent(CUAVObject* parent)
{
    m_parent = parent;
}

CUAVObject* CUAVObject::parent()
{
    return m_parent;
}

CUAVObject* CUAVObject::mainObject()
{
	if (m_parent) {
		return m_parent->mainObject();
	}
	return this;
}

std::vector<uint32_t> CUAVObject::makePathToObject() const
{
    std::vector<uint32_t> path;
    CUAVObject* parent = m_parent;
    path.push_back(m_instId);
    while (m_parent) {
        path.insert(path.begin(), parent->m_instId);
        parent = parent->m_parent;
    }
    path.insert(path.begin(), m_objId);
    return path;
}

/////////////////////////////////////////////////////////////////////
//                              instance                           //
/////////////////////////////////////////////////////////////////////

CUAVObject* CUAVObject::createInstance()
{
	CUAVObject* obj = new CUAVObject();
	if (!obj) {
		return 0;
	}
	obj->setObjectId(m_objId);
	obj->addRef();
	return obj;
}

void CUAVObject::addInstance(CUAVObject* instance)
{
	if (!instance) {
		return;
	}
	instance->setInstanceId(allocIdForInstance());
	instance->setParent(this);
	m_instances[instance->instanceId()] = instance;
	m_instIds.push_back(instance->instanceId());
	instance->addRef();
}

void CUAVObject::removeAllInstances()
{
	while (m_instIds.size()) {
		removeInstanceById(*m_instIds.begin());
	}
}

bool CUAVObject::removeInstanceById(uint32_t instId)
{
	if (!m_instances.count(instId)) {
		return false;
	}
	CUAVObject* obj = m_instances[instId];
	m_instances.erase(instId);
	for (auto it = m_instIds.begin();it<m_instIds.end();it++) {
		if (instId == *it) {
			m_instIds.erase(it);
			break;
		}
	}
	SAFE_RELEASE(obj);
	return true;
}

int CUAVObject::numInstances() const
{
	return int (m_instances.size());
}

CUAVObject* CUAVObject::instanceById(uint32_t instId)
{
	if (m_instances.count(instId) == 0) {
		return 0;
	}
	return m_instances[instId];
}

const std::vector<uint32_t>& CUAVObject::instnces() const
{
	return m_instIds;
}

/////////////////////////////////////////////////////////////////////
//                                fields                           //
/////////////////////////////////////////////////////////////////////

CUAVObjectField* CUAVObject::createField(const CString& name /*= CString()*/, CUAVObjectField::FieldType type /*= CUAVObjectField::FieldType_UINT8*/, size_t numElements /*= 1*/)
{
	CUAVObjectField* field = new CUAVObjectField();
	if (!field) {
		return 0;
	}
	field->setName(name);
	field->setType(type);
	field->setNumElements(numElements);
	addField(field);
	return field;
}

void CUAVObject::addField(CUAVObjectField* field)
{
	if (!field) {
		return;
	}
	field->setOwner(this);
	field->setFieldId(allocIdForField());
	field->addRef();

	m_fields[field->fieldId()] = field;
	m_fieldIds.push_back(field->fieldId());
}

void CUAVObject::removeAllFields()
{
	while (m_fieldIds.size()) {
		removeFieldById(*m_fieldIds.begin());
	}
}

bool CUAVObject::removeFieldById(uint32_t fieldId)
{
	if (m_fields.count(fieldId) == 0) {
		return false;
	}
	CUAVObjectField* field = m_fields[fieldId];
	m_fields.erase(fieldId);
	for (auto it = m_fieldIds.begin();it<m_fieldIds.end();it++) {
		if (fieldId == *it) {
			m_fieldIds.erase(it);
			break;
		}
	}
//	m_signal_instanceRemoved.invoke(obj);
	SAFE_RELEASE(field);
	return true;
}

size_t CUAVObject::numFields() const
{
	return uint32_t (m_fieldIds.size());
}

CUAVObjectField* CUAVObject::fieldById(uint32_t fieldId)
{
	if (m_fields.count(fieldId) == 0) {
		return 0;
	}
	return m_fields[fieldId];
}

CUAVObjectField* CUAVObject::fieldByName(const CString& name)
{
	for (auto it = m_fields.begin();it!=m_fields.end();it++) {
		if ((*it).second->name() == name) {
			return (*it).second;
		}
	}
	return 0;
}

const std::vector<uint32_t>& CUAVObject::fieldIds() const
{
	return m_fieldIds;
}

//===================================================================
//  p r i  v a t e   f u n c t i o n s
//===================================================================

uint32_t CUAVObject::allocIdForInstance()
{
	if (m_freeInstId.size()) {
		uint32_t instId = m_freeInstId[0];
		m_freeInstId.erase(m_freeInstId.begin());
		return instId;
	}
	return ++m_lastInstId;
}

void CUAVObject::freeInstanceId(uint32_t instId)
{
	m_freeInstId.push_back(instId);
}

uint32_t CUAVObject::allocIdForField()
{
	if (m_freeFieldId.size()) {
		uint32_t fieldId = m_freeFieldId[0];
		m_freeFieldId.erase(m_freeFieldId.begin());
		return fieldId;
	}
	return ++m_lastFieldId;
}

void CUAVObject::freeFieldId(uint32_t fieldId)
{
	m_freeFieldId.push_back(fieldId);
}
