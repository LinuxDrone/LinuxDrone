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

#include "CXenoTask.h"
#include "../system/Logger.h"

CXenoTask::CXenoTask(const CString& taskName, int priority, int stackSize) :
	m_taskName(taskName),
	m_stackSize(stackSize),
	m_priority(priority)
{
	m_runnable  = 0;
	m_created   = false;
	m_recreate  = false;
	m_running   = false;
}

CXenoTask::~CXenoTask()
{
	SAFE_RELEASE(m_runnable);
}

/////////////////////////////////////////////////////////////////////
//                             params                              //
/////////////////////////////////////////////////////////////////////

void CXenoTask::setTaskName(const CString& name)
{
	m_taskName = name;
	m_recreate = true;
}

CString CXenoTask::taskName() const
{
	return m_taskName;
}

void CXenoTask::setStackSize(int size)
{
	m_stackSize = size;
	m_recreate = true;
}

int CXenoTask::stackSize() const
{
	return m_stackSize;
}

void CXenoTask::setPriority(int priority)
{
	m_priority = priority;
	if (m_running) {
		int err = rt_task_set_priority(&m_task, m_priority);
		if (err) {
			Logger() << __FUNCTION__ << ": error set priority in task. error = " << err;
		}
	}
}

int CXenoTask::priority() const
{
	return m_priority;
}

/////////////////////////////////////////////////////////////////////
//                              run                                //
/////////////////////////////////////////////////////////////////////

void CXenoTask::start(CL_Runnable* runnable)
{
	if (started()) {
		return;;
	}
	if (!runnable) {
		return;
	}
	m_runnable = runnable;
	m_runnable->addRef();
	if (m_created && m_recreate) {
		destroyTask();
	}
	startTask();
}

bool CXenoTask::started() const
{
	return m_running;
}

bool CXenoTask::suspend()
{
	int err = rt_task_suspend(&m_task);
	if (err != 0) {
		Logger() << __FUNCTION__ << ": error suspending Xenomai task. error = " << err;
		return false;
	}
	return true;
}

bool CXenoTask::resume()
{
	int err = rt_task_resume(&m_task);
	if (err != 0) {
		Logger() << __FUNCTION__ << ": error resuming Xenomai task. error = " << err;
		return false;
	}
	return true;
}

void CXenoTask::sleep(int miliseconds)
{
	RTIME delay = miliseconds*rt_timer_ns2ticks(1000000);
	rt_task_sleep(delay);
}

void* CXenoTask::handle()
{
	return &m_task;
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

bool CXenoTask::createTask()
{
	if (m_created) {
		return true;
	}
	static const int taskMode = 0;
	int err = rt_task_create(&m_task, m_taskName.data(), m_stackSize, m_priority, 0/*taskMode*/);
	if (err != 0) {
		Logger() << __FUNCTION__ << ": error creating new Xenomai task. error = " << err;
		return false;
	}
	m_created  = true;
	m_recreate = false;
	return true;
}

void CXenoTask::destroyTask()
{
	if (!m_created) {
		return;
	}
	rt_task_delete(&m_task);
	m_created = false;
}

void CXenoTask::startTask()
{
	if (!m_created) {
		if (!createTask()) {
			return;
		}
	}
	int err = rt_task_start(&m_task, &CXenoTask::mainRun, this);
	if (err != 0) {
		Logger() << __FUNCTION__ << ": error starting Xenomai task. error = " << err;
		return;
	}
}

void CXenoTask::mainRun(void* param)
{
	CXenoTask* task = static_cast<CXenoTask*>(param);
	if (!task) {
		return;
	}
	task->m_running = true;
	if (task->m_runnable) {
		task->m_runnable->run();
	}
	SAFE_RELEASE(task->m_runnable);
	task->m_running = false;
	task->destroyTask();
}
