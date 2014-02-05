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

#include "CMutex.h"

CMutex::CMutex()
{
#ifdef XENO_SERVICES
	rt_mutex_create(&m_handle, 0);
#endif
}

CMutex::~CMutex()
{
#ifdef XENO_SERVICES
	rt_mutex_delete(&m_handle);
#endif
}

void CMutex::lock()
{
#ifdef XENO_SERVICES
	rt_mutex_acquire(&m_handle, TM_INFINITE);
#endif
}

bool CMutex::tryLock(int timeOutMs /*= TM_NONBLOCK*/)
{
#ifdef XENO_SERVICES
	RTIME delay;
	if (timeOutMs == TM_NONBLOCK) {
		delay = TM_NONBLOCK;
	} else {
		delay = timeOutMs*1000000;
	}
	return rt_mutex_acquire(&m_handle, delay) != 0 ? false : true;
#else
	return false;
#endif
}

void CMutex::unlock()
{
#ifdef XENO_SERVICES
	rt_mutex_release(&m_handle);
#endif
}
