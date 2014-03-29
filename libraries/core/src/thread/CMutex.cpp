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
#include "../system/Logger.h"

CMutex::CMutex(char const *name /*= nullptr*/, bool bind /*= false*/)
{
#ifdef XENO_SERVICES
	m_bind = false;
	int err = 0;
	if (name == nullptr) {
		err = rt_mutex_create(&m_handle, 0);
	} else {
		if (!bind) {
			err = rt_mutex_create(&m_handle, name);
		} else {
			err = rt_mutex_bind(&m_handle, name, TM_NONBLOCK);
			m_bind = true;
		}
	}
	if (err) {
		if (!bind) {
			Logger() << "error creating mutex with name = " << name << ". err = " << err;
		} else {
			Logger() << "error binding mutex with name = " << name << ". err = " << err;
		}
	}
#endif
}

CMutex::~CMutex()
{
#ifdef XENO_SERVICES
	if (!m_bind) {
		rt_mutex_delete(&m_handle);
	} else {
		rt_mutex_unbind(&m_handle);
	}
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
