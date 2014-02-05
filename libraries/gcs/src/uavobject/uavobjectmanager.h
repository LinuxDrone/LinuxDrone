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

#ifndef __gcs__uavobjectmanager__
#define __gcs__uavobjectmanager__

#include <map>
#include <vector>
#include "core/CObject"
#include "signals/Signals"

class UAVObject;

class UAVObjectManager : public CObject
{
private:
	typedef struct tagObject {
		UAVObject * object;
		
		CL_Slot slotObjectChanged;

	} OBJECT, *POBJECT;

public:
    UAVObjectManager();
    ~UAVObjectManager();

    void addObject(UAVObject* object);
    void clear();
	void removeObjectById(uint32_t objId);
	uint32_t numObjects() const;
	UAVObject* objectById(uint32_t objId);
	UAVObject* objectByPath(const std::vector<uint32_t>& path);
	const std::vector<uint32_t>& objectIds() const;

private:
    std::map<uint32_t, OBJECT> m_objects;
	std::vector<uint32_t> m_ids;
	
	CL_Signal_v1<UAVObject*> m_signal_objectAdded;
	CL_Signal_v1<UAVObject*> m_signal_objectRemoved;
    
// slots
    void objectChanged(UAVObject* obj);
};

#endif /* defined(__gcs__uavobjectmanager__) */
