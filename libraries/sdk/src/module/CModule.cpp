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

CModule::CModule(int stackSize) :
	m_task("", 50, stackSize)
{
	m_runnable        = 0;
	m_terminate       = false;
	m_queueCreated    = false;
	m_heapCreated     = false;
	m_outputHeapSize  = 512;
	m_mutexOutputHeap = 0;

	m_period         = (1000/50);
	m_notifyOnChange = true;
}

CModule::~CModule()
{
	m_terminate = true;
	while (m_task.started()) {
		m_task.sleep(100);
	}
	SAFE_RELEASE(m_runnable);

	SAFE_DELETE(m_mutexOutputHeap);
	if (m_heapCreated) {
		rt_heap_delete(&m_outputHeap);
	}
}

bool CModule::init(const mongo::BSONObj& initObject)
{
	m_name = initObject["name"].valuestr();
	m_instance = initObject["instance"].valuestr();
	m_task.setTaskName(m_instance);
	if (initObject.hasElement("Task Priority")) {
		m_task.setPriority((int) initObject["Task Priority"].Number());
	}
	if (initObject.hasElement("Task Period")) {
		m_period = int (initObject["Task Period"].Number());
	}
	if (initObject.hasElement("Notify on change")) {
		m_notifyOnChange = initObject["Notify on change"].Bool();
	}
	if(m_period != -1) {
		m_period = (uint32_t) (m_period * rt_timer_ns2ticks(1000));
	}
	CString name;
	name = CString("%1.shm").arg(m_instance);
	int err = rt_heap_create(&m_outputHeap, name.data(), m_outputHeapSize, H_FIFO | H_MAPPABLE | H_SHARED);
	if (err) {
		Logger() << __PRETTY_FUNCTION__ << ": error creating heap for output data. err = " << err;
		return false;
	}
	err = rt_heap_alloc(&m_outputHeap, 0, TM_NONBLOCK, &m_heapPtr);
	if (err) {
		Logger() << "error allocation memory from output heap. error = " << err;
		return false;
	}

	m_heapCreated = true;
	name = CString("%1.mt").arg(m_instance);
	m_mutexOutputHeap = new CMutex(name);
	return true;
}

bool CModule::link(const mongo::BSONObj& link)
{
	if (link.isEmpty()) {
		return false;
	}
	if (instance() == link["outInst"].String().c_str()) {
		if (CString("queue") != link["type"].String().c_str()) {
			return true;
		}
		// 'out' link
		CString moduleName = link["inInst"].String().c_str();
		CMutexSection locker(&m_mutexLinks);
		if (m_linksOut.count(moduleName) == 0) {
			LINK tmp;
			m_linksOut[moduleName] = tmp;
		}
		LINK& link_data = m_linksOut[moduleName];
		link_data.links.push_back(link.copy());
	} else {
		// 'in' link
		CString moduleName = link["outInst"].String().c_str();
		// create pipe
		if (!m_queueCreated && CString("queue") == link["type"].String().c_str()) {
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
			LINK tmp;
			m_linksIn[moduleName] = tmp;
		}
		LINK& link_data = m_linksIn[moduleName];
		link_data.links.push_back(link.copy());
		if (CString("memory") == link["type"].String().c_str()) {
			m_sharedLinks[CString(link["inPin"].String().c_str())] = CString(link["outInst"].String().c_str());
			m_sharedPinMaping[CString(link["outPin"].String().c_str())] = CString(link["inPin"].String().c_str());
		}
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
	return m_dataIn;
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
		if (m_period == -1 && !m_notifyOnChange) {
			m_task.sleep(10);
			continue;
		}
		for (auto it = m_linksIn.begin();it!=m_linksIn.end();it++) {
			(*it).second.heapSynchronized = false;
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
//  s h a r e d   h e a p
//-------------------------------------------------------------------

void CModule::sendBsonToHeap(const mongo::BSONObj& object)
{
	CMutexSection locker(m_mutexOutputHeap);
	size_t size = (size_t) object.objsize();
	if (!m_heapCreated || m_outputHeapSize <= size) {
		if (m_heapCreated) {
			rt_heap_free(&m_outputHeap, m_heapPtr);
			rt_heap_delete(&m_outputHeap);
			m_heapCreated = false;
		}
		m_outputHeapSize = size+1;
		CString name;
		name = CString("%1.sh").arg(m_instance);
		int err = rt_heap_create(&m_outputHeap, name.data(), m_outputHeapSize, H_FIFO);
		if (err) {
			Logger() << __PRETTY_FUNCTION__ << ": error creating heap for output data. err = " << err;
			return;
		}
		err = rt_heap_alloc(&m_outputHeap, 0, TM_NONBLOCK, &m_heapPtr);
		if (err) {
			Logger() << "error allocation memory from output heap. error = " << err;
			return;
		}
	}
	memcpy(m_heapPtr, object.objdata(), size);
}

mongo::BSONObj CModule::recvBsonFromHeap(const CString& name)
{// name: module instance name
	if (name.isEmpty()) {
		return mongo::BSONObj();
	}
	bool needUnbind = false;
	void* ptr = 0;
	RT_HEAP heap;
	CMutex* mutex = 0;

	if (m_linksIn.count(name)) {
		LINK& link = m_linksIn[name];
		if (link.heapCreated) {
			RT_HEAP_INFO info;
			int err = rt_heap_inquire(&link.heap, &info);
			if (err) {
				link.heapCreated = false;
				link.heapPtr = 0;
				SAFE_DELETE(link.mutexHeap);
			} else {
				ptr = link.heapPtr;
				mutex = link.mutexHeap;
			}
		}
		if (!link.heapCreated) {
			bool ret = bindHeap(name, &link.heap, &link.heapPtr, &link.mutexHeap);
			if (!ret) {
				return mongo::BSONObj();
			}
			link.heapCreated = true;
			ptr = link.heapPtr;
			mutex = link.mutexHeap;
		}
	} else {
		bool ret = bindHeap(name, &heap, &ptr, &mutex);
		if (!ret) {
			return mongo::BSONObj();
		}
		needUnbind = true;
	}

	mongo::BSONObj obj;
	{
		CMutexSection locker(mutex);
		obj = mongo::BSONObj(static_cast<const char*>(ptr)).copy();
	}
	if (needUnbind) {
		SAFE_DELETE(mutex);
		rt_heap_unbind(&heap);
	}
	return obj;
}

bool CModule::bindHeap(const CString& name, RT_HEAP* heap, void** ptr, CMutex** mutex)
{
	CString heapName = CString("%1.shm").arg(name);
	int err = rt_heap_bind(heap, heapName.data(), TM_NONBLOCK);
	if (err) {
		Logger() << __PRETTY_FUNCTION__ << ": error bind heap with name(" << heapName << "). error = " << err;
		return false;
	}
	err = rt_heap_alloc(heap, 0, TM_NONBLOCK, ptr);
	if (err) {
		Logger() << __PRETTY_FUNCTION__ << ": error mapping heap memory. error = " << err;
		rt_heap_unbind(heap);
		return false;
	}
	heapName = CString("%1.mt").arg(name);
	if (mutex != 0) {
		*mutex = new CMutex(heapName, true);
	}
	return true;
}

bool CModule::syncSharedMemoryLink(const CString& name)
{
	if (m_linksIn.count(name) == 0) {
		return false;
	}
	LINK& link = m_linksIn.at(name);
	if (link.heapSynchronized) {
		return true;
	}
	std::set<CString> pins;
	for (const mongo::BSONObj& it:link.links) {
		if (CString("memory") == it["type"].String().c_str()) {
			pins.insert(it["inPin"].String().c_str());
		}
	}
	if (!pins.size()) {
		return false;
	}
	mongo::BSONObj obj = recvBsonFromHeap(name);
	if (obj.isEmpty()) {
		return false;
	}
	mongo::BSONObj result = mergeObjects(m_dataIn, obj, &pins, &m_sharedPinMaping);
	m_dataIn = result.copy();
	{
		m_elementsIn.clear();
		mongo::BSONObjIterator it(m_dataIn);
		while (it.more()) {
			mongo::BSONElement elem = it.next();
			m_elementsIn[elem.fieldName()] = elem;
		}
	}
	link.heapSynchronized = true;
	return true;
}

//-------------------------------------------------------------------
//  c o m m o n
//-------------------------------------------------------------------

mongo::BSONObj CModule::mergeObjects(const mongo::BSONObj& dst, const mongo::BSONObj& src,
		const std::set<CString>* elementsSrc /*= 0*/, const std::map<CString, CString>*pinMapping /*= 0*/)
{
	mongo::BSONObj result;

	std::map<CString, mongo::BSONElement> allElements;
	{
		mongo::BSONObjIterator it(dst);
		while(it.more()) {
			mongo::BSONElement elem = it.next();
			allElements[CString(elem.fieldName())] = elem;
		}
	}
	mongo::BSONObjIterator it(src);
	while (it.more()) {
		mongo::BSONElement elem = it.next();
		CString pinName = elem.fieldName();
		if (pinMapping && pinMapping->count(pinName)) {
			pinName = pinMapping->at(pinName);
		}
		if (elementsSrc) {
			if (elementsSrc->count(pinName)) {
				allElements[pinName] = elem;
			}
			continue;
		}
		allElements[pinName] = elem;
	}
	mongo::BSONObjBuilder builder;
	for (auto obj:allElements) {
		builder << obj.first.data();
		builder << obj.second;
	}
	result = builder.obj();
	return result.copy();
}

//-------------------------------------------------------------------
//  i n p u t   v a l u e s
//-------------------------------------------------------------------

bool CModule::hasElement(const CString& name) const
{
	if (m_sharedLinks.count(name)) {
		CModule* module = const_cast<CModule*>(this);
		module->syncSharedMemoryLink(name);
	}
	return m_elementsIn.count(name) != 0;
}

mongo::BSONElement CModule::element(const CString& name) const
{
	if (m_sharedLinks.count(name)) {
		CModule* module = const_cast<CModule*>(this);
		CString moduleName = m_sharedLinks.at(name);
		module->syncSharedMemoryLink(moduleName);
	}
	if (!hasElement(name)) {
		return mongo::BSONElement();
	}
	return m_elementsIn.at(name);
}

CString CModule::valueString(const CString& elemName) const
{
	mongo::BSONElement elem = element(elemName);
	if (elem.isNull()) {
		return CString();
	}
	return CString(elem.String().c_str());
}

double CModule::valueDouble(const CString& elemName) const
{
	mongo::BSONElement elem = element(elemName);
	if (elem.isNull()) {
		return 0.0;
	}
	return elem.Double();
}

int CModule::valueInt(const CString& elemName) const
{
	mongo::BSONElement elem = element(elemName);
	if (elem.isNull()) {
		return 0;
	}
	return elem.Int();
}

bool CModule::valueBool(const CString& elemName) const
{
	mongo::BSONElement elem = element(elemName);
	return !elem.isNull() && elem.Bool();
}

double CModule::valueNumber(const CString& elemName) const
{
	mongo::BSONElement elem = element(elemName);
	if (elem.isNull()) {
		return 0.0f;
	}
	return elem.Number();
}

//-------------------------------------------------------------------
//  q u e u e   o b j e c t s
//-------------------------------------------------------------------

void CModule::sendObject(const mongo::BSONObj& object)
{
	if (object.isEmpty()) {
		return;
	}
	m_dataOut = mergeObjects(m_dataOut, object);
//	Logger() << m_dataOut.toString(false, true).c_str();
	sendBsonToHeap(m_dataOut);

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

		void* ptr = rt_queue_alloc(&link_data.queue, (size_t) objForSend.objsize());
		if (!ptr) {
			continue;
		}
		memcpy(ptr, objForSend.objdata(), (size_t) objForSend.objsize());
		int err = rt_queue_send(&link_data.queue, ptr, (size_t) objForSend.objsize(), Q_NORMAL);
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
			m_task.sleep(1);
		}
		return;
	}
	void* ptr = 0;
	unsigned long long timeout = TM_NONBLOCK;
	if (m_period == -1) {
		timeout = 1000;
	}
	int read = rt_queue_receive(&m_inputQueue, &ptr, timeout);
	if (read < 0) {
		CSystem::sleep(2);
		return;
	}
//	Logger() << "read =" << read;
	mongo::BSONObj obj = mongo::BSONObj((char*)ptr).copy();
	rt_queue_free(&m_inputQueue, ptr);

	CMutexSection locker(&m_mutexData);
	// merge new data object with our
	std::map<CString, mongo::BSONElement> elements;
	{
		mongo::BSONObjIterator objIt(m_dataIn);
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

	m_dataIn = builder.obj();
	{
		m_elementsIn.clear();
		mongo::BSONObjIterator it(m_dataIn);
		while (it.more()) {
			mongo::BSONElement elem = it.next();
			m_elementsIn[elem.fieldName()] = elem;
		}
	}
	if (m_notifyOnChange) {
		receivedData();
	}
}

//-------------------------------------------------------------------
//  n o t i f y
//-------------------------------------------------------------------

void CModule::receivedData()
{
}
