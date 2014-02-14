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

#include "CModule.h"
#include "system/CSystem"
#include "system/Logger"
#include <native/timer.h>

CModule::CModule(int stackSize) :
	m_task("", 50, stackSize)
{
	m_runnable     = 0;
	m_terminate    = false;
	m_queueCreated = false;
	m_heapCreated  = false;

	m_period         = (1000/50)*1000000;
	m_notifyOnChange = true;
}

CModule::~CModule()
{
	m_terminate = true;
	while (m_task.started()) {
		m_task.sleep(100);
	}
	SAFE_RELEASE(m_runnable);
}

bool CModule::init(const mongo::BSONObj& initObject)
{
	m_name = initObject["name"].valuestr();
	m_instance = initObject["instance"].valuestr();
	m_task.setTaskName(m_instance);
	if (initObject.hasElement("task_priority")) {
		m_task.setPriority(initObject["task_priority"].Number());
	}
	if (initObject.hasElement("period")) {
		m_period = int (initObject["period"].Number());
	}
	if (initObject.hasElement("notifyOnChange")) {
		m_notifyOnChange = initObject["notifyOnChange"].Bool();
	}
	return true;
}

bool CModule::link(const mongo::BSONObj& link)
{
	if (link.isEmpty()) {
		return false;
	}
	if (instance() != link["inInst"].String().c_str()) {
		CString moduleName = link["inInst"].String().c_str();
		// 'out' link
		CMutexSection locker(&m_mutexLinks);
		if (m_linksOut.count(moduleName) == 0) {
			LINK link;
			m_linksOut[moduleName] = link;
		}
		LINK& link_data = m_linksOut[moduleName];
		link_data.links.push_back(link.copy());
	} else {
		// 'in' link
		CString moduleName = link["outInst"].String().c_str();
		// create pipe
		if (!m_queueCreated && CString("pipe") == link["type"].String().c_str()) {
			CString name = m_instance + CString("inputQueue");
			int err = rt_queue_create(&m_inputQueue, name.data(), 8096, Q_UNLIMITED, Q_FIFO);
			if (err) {
				Logger() << "error creating queue with name = " << name << ". error =" << err;
				return false;
			}
			m_queueCreated = true;
		}
		CMutexSection locker(&m_mutexLinks);
		if (m_linksIn.count(moduleName) == 0) {
			LINK link;
			m_linksIn[moduleName] = link;
		}
		LINK& link_data = m_linksIn[moduleName];
		link_data.links.push_back(link.copy());
	}
	return true;
}

bool CModule::start()
{
	startTask();
	return true;
}

void CModule::stop()
{
}

// module name
CString CModule::name() const
{
	return m_name;
}

// instance name
CString CModule::instance() const
{
	return m_instance;
}


void CModule::setTaskName(const CString& name)
{
	m_task.setTaskName(name);
}

CString CModule::taskName() const
{
	return m_task.taskName();
}

CXenoTask& CModule::task()
{
	return m_task;
}

/////////////////////////////////////////////////////////////////////
//                              data                               //
/////////////////////////////////////////////////////////////////////

void CModule::addData(const mongo::BSONObj& object)
{
	sendObject(object);
}

mongo::BSONObj CModule::data() const
{
	return m_data;
}

//===================================================================
//  p r o t e c t e d   f u n c t i o n s
//===================================================================

void CModule::startTask()
{
	m_terminate = false;
	m_task.start(this, &CModule::mainTask);
}

void CModule::stopTask()
{
	m_terminate = true;
	while (m_task.started()) {
		// TODO: нужно поставить sleep, вот только нужно определиться от какого таска его вызывать
		CSystem::sleep(10);
	}
}

void CModule::mainTask()
{
	RTIME startTime = rt_timer_read();

	while (!m_terminate) {
		if (m_period == -1 && m_notifyOnChange == false) {
			m_task.sleep(10 * 1000000);
			continue;
		}
		RTIME current = rt_timer_read();
		if (m_period != -1 && m_runnable) {
			RTIME result = current - startTime;
			if (result >= m_period) {
				m_runnable->run();
				startTime = current;
			}
		}
		recvObjects();
	}
	SAFE_RELEASE(m_runnable);
}

//-------------------------------------------------------------------
//  q u e u e   o b j e c t s
//-------------------------------------------------------------------

void CModule::sendObject(const mongo::BSONObj& object)
{
	if (object.isEmpty()) {
		return;
	}
//	Logger() << object.toString(false, true).c_str();

	std::map<CString, mongo::BSONElement> objElements;
	{
		mongo::BSONObjIterator objIt(object);
		while (objIt.more()) {
			mongo::BSONElement elem = objIt.next();
			if (elem.isNull()) {
				continue;
			}
			objElements[elem.fieldName()] = elem;
		}
	}
	CMutexSection locker(&m_mutexLinks);
	for (auto it_links = m_linksOut.begin();it_links!=m_linksOut.end();it_links++) {
		LINK& link_data = (*it_links).second;
		if (link_data.links.size() == 0) {
			continue;
		}
		mongo::BSONObjBuilder builder;
		for (auto it = link_data.links.begin();it<link_data.links.end();it++) {
			mongo::BSONObj& link = *it;
//			Logger() << link.toString(false, true).c_str();
			CString outPin = link["outPin"].String().c_str();
			if (objElements.count(outPin) == 0) {
				continue;
			}
			builder << link["inPin"].String() << objElements[outPin];
		}
		if (!link_data.queueCreated) {
			CString name = CString((link_data.links[0])["inInst"].String().c_str()) + CString("inputQueue");
			int err = rt_queue_bind(&link_data.queue, name.data(), TM_NONBLOCK);
			if (err != 0) {
				Logger() << "error binding queue for send data to module. err =" << err;
				continue;
			}
			link_data.queueCreated = true;
		}
		mongo::BSONObj objForSend = builder.obj();
		;
		void* ptr = rt_queue_alloc(&link_data.queue, objForSend.objsize());
		if (!ptr) {
			continue;
		}
		memcpy(ptr, objForSend.objdata(), objForSend.objsize());
		int err = rt_queue_send(&link_data.queue, ptr, objForSend.objsize(), Q_NORMAL);
		if (err < 0) {
			rt_queue_free(&link_data.queue, ptr);
			Logger() << "error writing in queue. err =" << err;
		}
	}
}

void CModule::recvObjects()
{
	if (!m_queueCreated) {
		if (m_period == -1) {
			m_task.sleep(1000);
		} else {
			m_task.sleep(100);
		}
		return;
	}
	void* ptr = 0;
	unsigned long long timeout = TM_NONBLOCK;
	if (m_period == -1) {
		timeout = 1000;
	}
	int readed = rt_queue_receive(&m_inputQueue, &ptr, timeout);
	if (readed < 0) {
		CSystem::sleep(2);
		return;
	}
	Logger() << "readed =" << readed;
	mongo::BSONObj obj = mongo::BSONObj((char*)ptr).copy();
	rt_queue_free(&m_inputQueue, ptr);

	CMutexSection locker(&m_mutexData);
	// merge new data object with our
	std::map<CString, mongo::BSONElement> elements;
	{
		mongo::BSONObjIterator objIt(m_data);
		while (objIt.more()) {
			mongo::BSONElement elem = objIt.next();
			if (elem.isNull()) {
				continue;
			}
			elements[elem.fieldName()] = elem;
		}
	}
	{
		mongo::BSONObjIterator objIt(obj);
		while (objIt.more()) {
			mongo::BSONElement elem = objIt.next();
			CString name = elem.fieldName();
			elements[name] = elem;
		}
	}
	mongo::BSONObjBuilder builder;
	for (auto it = elements.begin();it!=elements.end();it++) {
		builder.append((*it).second);
	}
	m_data = builder.obj();
	if (m_notifyOnChange) {
		recievedData(m_data);
	}
}

//-------------------------------------------------------------------
//  n o t i f y
//-------------------------------------------------------------------

void CModule::recievedData(const mongo::BSONObj& /*obj*/)
{
}
