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

#include "CObject.h"
#include "../memmanager/my_memory.h"

CObject::CObject()
{
    m_refs = 0;
}

CObject::~CObject()
{
    m_refs = 0;
}

uint32_t CObject::addRef()
{
	CMutexSection locker(&m_mutexRefs);
    return ++m_refs;
}

uint32_t CObject::release()
{
	CMutexSection locker(&m_mutexRefs);
	if( !m_refs )
		return 0;
	m_refs--;
	if( !m_refs )
	{
		locker.unlock();
		selfDelete();
		return 0;
	}
	return m_refs;
}

uint32_t CObject::refs()
{
	CMutexSection locker(&m_mutexRefs);
    return m_refs;
}

void CObject::onUpdate(float timeElapsed)
{
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

void CObject::selfDelete()
{
    delete this;
}
