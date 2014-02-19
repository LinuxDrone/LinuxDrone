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

#pragma once

#include "thread/CRunnable"
#include "xeno/CXenoTask"
#include "thread/CMutex"
#include <native/queue.h>
#include <native/heap.h>
#include <native/mutex.h>

#include <mongo/bson/bson.h>
#include <map>
#include <vector>
#include "my_memory"

class CModule : public CObject
{
public:
	typedef struct tagLink {
		tagLink() {
			queueCreated = false;
			heapCreated  = false;
			mutexHeap    = 0;
			heapPtr      = 0;
		}
		~tagLink() {
			if (mutexHeap) {
				delete mutexHeap;
				mutexHeap = 0;
			}
			if (heapCreated) {
				rt_heap_unbind(&heap);
			}
		}
		std::vector<mongo::BSONObj> links;
		RT_QUEUE queue;
		bool     queueCreated;

		RT_HEAP  heap;
		bool     heapCreated;
		void   * heapPtr;
		CMutex * mutexHeap;

	} LINK, *PLINK;

public:
	CModule(int stackSize);
	virtual ~CModule();

	virtual bool init(const mongo::BSONObj& initObject);
	virtual bool link(const mongo::BSONObj& link);
	virtual bool start();
	virtual void stop();

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
	mongo::BSONObj data() const;

protected:
	CString       m_name;
	CString       m_instance;
	uint32_t      m_period;
	bool          m_notifyOnChange;

	bool          m_terminate;
	CL_Runnable * m_runnable;
	CXenoTask     m_task;

	RT_QUEUE m_inputQueue;
	bool     m_queueCreated;

	RT_HEAP   m_outputHeap;
	bool      m_heapCreated;
	size_t    m_outputHeapSize;
	CMutex  * m_mutexOutputHeap;
	void    * m_heapPtr;

	std::map<CString, LINK> m_linksIn;
	std::map<CString, LINK> m_linksOut;
	CMutex                  m_mutexLinks;

	mongo::BSONObj m_data;
	std::map<CString, mongo::BSONElement> m_elements;
	CMutex         m_mutexData;

	void startTask();
	void stopTask();
	void mainTask();

// shared heap
	void outBsonToHeap(const mongo::BSONObj& object);
	mongo::BSONObj recvBsonFromHeap(const CString& name);		// name: module instance name
	bool bindHeap(const CString& name, RT_HEAP* heap, void** ptr, CMutex** mutex);

// queue objects
	void sendObject(const mongo::BSONObj& obj);
	void recvObjects();

// notify
	virtual void recievedData(const mongo::BSONObj& data);
};
