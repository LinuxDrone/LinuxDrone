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

#include "uavobjectmanager.h"
#include "uavobject.h"
#include <assert.h>

UAVObjectManager::UAVObjectManager()
{
}

UAVObjectManager::~UAVObjectManager()
{
    clear();
}

void UAVObjectManager::addObject(UAVObject* object)
{
    if (!object) {
        return;
    }
    if (m_objects.count(object->objectId())) {
        assert(false);
        return;
    }
	OBJECT obj;
	obj.object = object;
//	obj.slotObjectChanged = object->signalInstanceAdded()

    m_objects[object->objectId()] = obj;
	m_ids.push_back(object->objectId());
    object->addRef();
	
	m_signal_objectAdded.invokeQueue(object);
}

void UAVObjectManager::clear()
{
	while (m_ids.size()) {
		removeObjectById(*m_ids.begin());
	}
}

void UAVObjectManager::removeObjectById(uint32_t objId)
{
	if (m_objects.count(objId) == 0) {
		return;
	}
	UAVObject* obj = m_objects[objId].object;
	m_objects.erase(objId);
	for (auto it = m_ids.begin();it<m_ids.end();it++) {
		if(objId == *it) {
			m_ids.erase(it);
			break;
		}
	}
	m_signal_objectRemoved.invoke(obj);
	SAFE_RELEASE(obj);
}

uint32_t UAVObjectManager::numObjects() const
{
	return uint32_t (m_ids.size());
}

UAVObject* UAVObjectManager::objectById(uint32_t objId)
{
	if (m_objects.count(objId) == 0) {
		return 0;
	}
	return m_objects[objId].object;
}

UAVObject* UAVObjectManager::objectByPath(const std::vector<uint32_t>& path)
{
	if (!path.size()) {
		return 0;
	}
	UAVObject* obj = objectById(path[0]);
	if (!obj) {
		return 0;
	}
	for (auto it = path.begin()+1;it<path.end();it++) {
		obj = obj->instanceById(*it);
		if (!obj) {
			return 0;
		}
	}
	return obj;
}

const std::vector<uint32_t>& UAVObjectManager::objectIds() const
{
	return m_ids;
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

//-------------------------------------------------------------------
//  s l o t s
//-------------------------------------------------------------------

void UAVObjectManager::objectChanged(UAVObject* obj)
{
}
