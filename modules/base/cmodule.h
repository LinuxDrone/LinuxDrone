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

#ifndef __CMODULE_H__
#define __CMODULE_H__

#include "thread/CRunnable"
#include "xeno/CXenoTask"
#include "thread/CMutex"
#include <native/pipe.h>
#include <native/heap.h>

#include <mongo/bson/bson.h>
#include <map>
#include <vector>
#include "my_memory"

class CModule : public CObject
{
public:
	typedef struct tagLink {
		tagLink() {
			pipeCreated = false;
			heapCreated = false;
		}
		std::vector<mongo::BSONObj> links;
		RT_HEAP heap;
		RT_PIPE pipe;
		bool    pipeCreated;
		bool    heapCreated;

	} LINK, *PLINK;
public:
	CModule(const CString& taskName, int priority, int stackSize);
	virtual ~CModule();

	virtual bool init(const mongo::BSONObj& initObject);
	virtual bool link(const mongo::BSONObj& link);

	// module name
	virtual CString name() const;
	// instance name
	virtual CString instance() const;

	void setTaskName(const CString& name);
	CString taskName() const;
	CXenoTask& task();

	template<class C>
	void start(C *instance, void (C::*member)()) {
		SAFE_RELEASE(m_runnable);
		m_runnable = new CL_RunnableMember_v0<C>(instance, member);
		m_runnable->addRef();
		startTask();
	}

// data
	void addData(const mongo::BSONObj& object);

protected:
	CString       m_name;
	CString       m_instance;

	bool          m_terminate;
	CL_Runnable * m_runnable;
	CXenoTask     m_task;

	RT_PIPE m_inputQueue;
	bool    m_queueCreated;
	RT_HEAP m_outputHeap;
	bool    m_heapCreated;

	std::map<CString, LINK> m_linksIn;
	std::map<CString, LINK> m_linksOut;
	CMutex                  m_mutexLinks;

//	std::list<CUAVObject*> m_dataQueue;
//	CMutex                 m_mutexQueue;

	void startTask();
	void stopTask();
	void mainTask();

// queue objects
	void sendObject(const mongo::BSONObj& obj);
	void recvObjects();
//	void addToQueue(CUAVObject* data);

// notify
//	virtual void recievedData(CUAVObject* data);
};

#endif // __CMODULE_H__
