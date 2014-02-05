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

#include "cmodule.h"
#include "system/CSystem"
#include "system/Logger"

CModule::CModule(const CString& taskName, int priority, int stackSize) :
	m_task(taskName, priority, stackSize)
{
	m_runnable     = 0;
	m_terminate    = false;
	m_queueCreated = false;
	m_heapCreated  = false;
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
	return true;
}

bool CModule::link(const mongo::BSONObj& link)
{
	if (link.isEmpty()) {
		return false;
	}
	CString moduleName;
	if (link["outInst"].String().c_str() == this->instance()) {
		moduleName = link["inInst"].String().c_str();
	} else {
		moduleName = link["outInst"].String().c_str();
	}
	if (moduleName == instance()) {
		// 'out' link
		CMutexSection locker(&m_mutexLinks);
		if (m_linksOut.count(moduleName) == 0) {
			LINK link;
			m_linksOut[moduleName] = link;
		}
		LINK& link_data = m_linksOut[moduleName];
		link_data.links.push_back(link);
	} else {
		// 'in' link
		// create pipe
		if (!m_queueCreated && CString("pipe") == link["type"].String().c_str()) {
			CString name = m_instance + CString("inputQueue");
			int err = rt_pipe_create(&m_inputQueue, name.data(), P_MINOR_AUTO, 100);
			if (err) {
				Logger() << "error creating pipe with name = " << name << ". error =" << err;
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
		link_data.links.push_back(link);
	}
	return true;
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
	while (!m_terminate) {
		if (m_runnable) {
			m_runnable->run();
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
		mongo::BSONObjBuilder builder;
		for (auto it = link_data.links.begin();it<link_data.links.end();it++) {
			mongo::BSONObj& link = *it;
			CString outPin = link["outPin"].String().c_str();
			if (objElements.count(outPin) == 0) {
				continue;
			}
			builder << link["inPin"].String() << objElements[outPin];
		}
		if (!link_data.pipeCreated) {

		}
	}
}

void CModule::recvObjects()
{
//	CMutexSection locker(&m_mutexQueue);
//	for (auto it = m_dataQueue.begin();it!=m_dataQueue.end();it++) {
//		CUAVObject* object = *it;
//		if (!object) {
//			continue;
//		}
//		recievedData(object);
//		SAFE_RELEASE(object);
//	}
//	m_dataQueue.clear();
}

//void CModule::addToQueue(CUAVObject* data)
//{
//	if (!data) {
//		return;
//	}
//	CMutexSection locker(&m_mutexQueue);
//	m_dataQueue.push_back(data);
//	data->addRef();
//}

//-------------------------------------------------------------------
//  n o t i f y
//-------------------------------------------------------------------

//void CModule::recievedData(CUAVObject* /*data*/)
//{
//}
