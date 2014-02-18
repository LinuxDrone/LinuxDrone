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

#ifndef __CXENO_TASK_H__
#define __CXENO_TASK_H__

#include "../text/CString.h"
#include "../thread/CRunnable.h"
#include <native/task.h>
#include <native/timer.h>

class CXenoTask
{
public:
	CXenoTask(const CString& taskName, int priority, int stackSize);
	~CXenoTask();

// params
	void setTaskName(const CString& name);
	CString taskName() const;
	void setStackSize(int size);
	int stackSize() const;
	void setPriority(int priority);
	int priority() const;

// run
	void start(CL_Runnable* runnable);
	bool started() const;
	bool suspend();
	bool resume();
	void sleep(int miliseconds);
	void* handle();

	template<class C>
	void start(C *instance, void (C::*member)())
	{
		CL_Runnable *r = new CL_RunnableMember_v0<C>(instance, member);
		r->addRef();
		start(r);
		r->release();
	}

private:
	CString       m_taskName;
	int           m_stackSize;
	int           m_priority;
	CL_Runnable * m_runnable;

	RT_TASK       m_task;
	bool          m_created;
	bool          m_recreate;
	bool          m_running;

	bool createTask();
	void destroyTask();
	void startTask();
	static void mainRun(void* param);
};

#endif // __CXENO_TASK_H__
