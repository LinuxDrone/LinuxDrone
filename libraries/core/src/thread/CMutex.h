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

#ifndef __core__cmutex__
#define __core__cmutex__

#include "../libCoreGlobal.h"
#ifdef XENO_SERVICES
#include <native/mutex.h>
#else
#define TM_NONBLOCK -1
#endif

class CMutex
{
public:
	CMutex();
	~CMutex();
    
	void lock();
	bool tryLock(int timeOutMs = int (TM_NONBLOCK));
	void unlock();

private:
#ifdef XENO_SERVICES
	RT_MUTEX m_handle;
#endif
};

class CMutexSection
{
public:
	/// \brief Constructs a mutex section.
	CMutexSection( CMutex *mutex, bool lockMutex = true )
	{
		m_mutex     = mutex;
		m_lockCount = 0;
		if( lockMutex )
			lock();
	}
    
	~CMutexSection()
	{
		if (m_lockCount > 0 && m_mutex) {
			while (m_lockCount) {
				unlock();
			}
		}
		m_lockCount = 0;
	}
    
	int lockCount() const
	{
		return m_lockCount;
	}
    
	void lock()
	{
		if( m_mutex )
			m_mutex->lock();
		m_lockCount++;
	}
	bool tryLock()
	{
		if (m_mutex->tryLock())
		{
			m_lockCount++;
			return true;
		}
		return false;
	}
	void unlock()
	{
		if( m_lockCount <= 0 )
			return;
		if( m_mutex )
			m_mutex->unlock();
		m_lockCount--;
	}
    
private:
	CMutex * m_mutex;
	int      m_lockCount;
};

#endif /* defined(__core__cmutex__) */
