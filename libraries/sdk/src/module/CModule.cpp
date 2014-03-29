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

	m_task.setTaskName(CString(m_instance.data()));
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
	CStringAnsi name;
	name = m_instance + ".shm";
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
	name = m_instance + ".mt";
	m_mutexOutputHeap = new CMutex(name.data());
	return true;
}

bool CModule::link(const mongo::BSONObj& link)
{
	if (link.isEmpty()) {
		return false;
	}
	if (instance() == link["outInst"].valuestr()) {
		if (CStringAnsi("queue", (size_t) -1, true) != link["type"].valuestr()) {
			return true;
		}
		// 'out' link
		CStringAnsi moduleName(link["inInst"].valuestr(), (size_t) -1, true);
//		CMutexSection locker(&m_mutexLinks);
        bool exist = false;
        CStringAnsi inPin = link["inPin"].valuestr();
        CStringAnsi outPin = link["outPin"].valuestr();
        for (std::pair<CStringAnsi, LINK>& it:m_linksOut) {
            if (it.first == moduleName) {
                exist = true;
                it.second.links.push_back(std::make_pair(outPin, inPin));
                break;
            }
        }
		if (!exist) {
			LINK tmp;
            tmp.inInstance = moduleName.copy();
            tmp.links.push_back(std::make_pair(outPin, inPin));
            m_linksOut.push_back(std::make_pair(moduleName, tmp));
		}
	} else {
		// 'in' link
		CStringAnsi moduleName(link["outInst"].valuestr(), (size_t) -1, true);
		// create pipe
		if (!m_queueCreated && CStringAnsi("queue", (size_t) -1, true) == link["type"].valuestr()) {
			CStringAnsi name = m_instance + CStringAnsi("inputQueue", (size_t)-1, true);
			int err = rt_queue_create(&m_inputQueue, name.data(), 8096, Q_UNLIMITED, Q_FIFO);
			if (err) {
				Logger() << "error creating queue with name = " << name << ". error =" << err;
				return false;
			}
			m_queueCreated = true;
		}
//		CMutexSection locker(&m_mutexLinks);
		if (m_linksIn.count(moduleName) == 0) {
			LINK tmp;
			m_linksIn[moduleName.copy()] = tmp;
		}
		LINK& link_data = m_linksIn[moduleName];
		if (CStringAnsi("memory", (size_t)-1, true) == link["type"].valuestr()) {
			CStringAnsi inPin = link["inPin"].valuestr();
			CStringAnsi outPin = link["outPin"].valuestr();
            link_data.links.push_back(std::make_pair(outPin, inPin));
			m_sharedLinks[inPin] = CStringAnsi(link["outInst"].valuestr());
			m_sharedPinMaping[outPin] = inPin;
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
CStringAnsi CModule::name() const
{
	return m_name;
}

// instance name
CStringAnsi CModule::instance() const
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
        CStringAnsi name = m_instance + CStringAnsi(".sh", (size_t) -1, true);
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

mongo::BSONObj CModule::recvBsonFromHeap(CStringAnsi const &name)
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

bool CModule::bindHeap(CStringAnsi const &name, RT_HEAP *heap, void **ptr, CMutex **mutex)
{
	CStringAnsi heapName = name + CStringAnsi(".shm", (size_t) -1, true);
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
	heapName = name + ".mt";
	if (mutex != 0) {
		*mutex = new CMutex(heapName.data(), true);
	}
	return true;
}

bool CModule::syncSharedMemoryLink(CStringAnsi const &name)
{
	if (m_linksIn.count(name) == 0) {
		return false;
	}
	LINK& link = m_linksIn.at(name);
	if (link.heapSynchronized) {
		return true;
	}
	std::set<CStringAnsi> pins;
	for (const std::pair<CStringAnsi, CStringAnsi>& it:link.links) {
		pins.insert(it.second);
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
		std::set<CStringAnsi> * const elementsSrc /*= 0*/, std::map<CStringAnsi, CStringAnsi> * const pinMapping /*= 0*/)
{
    // TODO: needed refactor of function (slowly)
	mongo::BSONObj result;

	std::map<CStringAnsi, mongo::BSONElement> allElements;
	{
		mongo::BSONObjIterator it(dst);
		while(it.more()) {
			mongo::BSONElement elem = it.next();
			allElements[CStringAnsi(elem.fieldName(), (size_t) -1, true)] = elem;
		}
	}
	mongo::BSONObjIterator it(src);
	while (it.more()) {
		mongo::BSONElement elem = it.next();
		CStringAnsi pinName;

        CStringAnsi fieldName(elem.fieldName(), (size_t) -1, true);
        if (pinMapping && pinMapping->count(fieldName)) {
			pinName = pinMapping->at(fieldName);
		} else {
            pinName = fieldName;
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
		builder << obj.first.data() << obj.second;
	}
	result = builder.obj();
	return result.copy();
}

//-------------------------------------------------------------------
//  i n p u t   v a l u e s
//-------------------------------------------------------------------

bool CModule::hasElement(char const *name) const
{
	if (name == nullptr) {
		return false;
	}
	CStringAnsi cname(name, (size_t) -1, true);
	if (m_sharedLinks.count(cname)) {
		CModule* module = const_cast<CModule*>(this);
        CStringAnsi moduleName = m_sharedLinks.at(cname);
		module->syncSharedMemoryLink(moduleName);
	}
	return m_elementsIn.count(cname) != 0;
}

mongo::BSONElement CModule::element(char const *name) const
{
	if (m_sharedLinks.count(name)) {
		CModule* module = const_cast<CModule*>(this);
		CStringAnsi moduleName = m_sharedLinks.at(name);
		module->syncSharedMemoryLink(moduleName);
	}
	if (!hasElement(name)) {
		return mongo::BSONElement();
	}
	return m_elementsIn.at(CStringAnsi(name, (size_t) -1, true));
}

CString CModule::valueString(char const *elemName) const
{
	mongo::BSONElement elem = element(elemName);
	if (elem.isNull()) {
		return CString();
	}
	return CString(elem.String().c_str());
}

double CModule::valueDouble(char const *elemName) const
{
	mongo::BSONElement elem = element(elemName);
	if (elem.isNull()) {
		return 0.0;
	}
	return elem.Double();
}

int CModule::valueInt(char const *elemName) const
{
	mongo::BSONElement elem = element(elemName);
	if (elem.isNull()) {
		return 0;
	}
	return elem.Int();
}

bool CModule::valueBool(char const *elemName) const
{
	mongo::BSONElement elem = element(elemName);
	return !elem.isNull() && elem.Bool();
}

double CModule::valueNumber(char const *elemName) const
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

    m_tmpObjElements.clear();
	{
		mongo::BSONObjIterator objIt(object);
		while (objIt.more()) {
			mongo::BSONElement elem = objIt.next();
			if (elem.isNull()) {
				continue;
			}
            m_tmpObjElements.push_back(elem);
		}
	}
	CMutexSection locker(&m_mutexLinks);
	for (std::pair<CStringAnsi, LINK>& it_links:m_linksOut) {
		LINK& link_data = it_links.second;
		if (link_data.links.size() == 0) {
			continue;
		}
		mongo::BSONObjBuilder builder;
		for (const std::pair<CStringAnsi, CStringAnsi>& it:link_data.links) {
			CStringAnsi outPin = it.first;
            for (const mongo::BSONElement& elem:m_tmpObjElements) {
                if (outPin == elem.fieldName()) {
                    builder << it.second.data() << elem;
                    break;
                }
            }
		}
		if (!link_data.queueCreated) {
			CStringAnsi name = link_data.inInstance + CStringAnsi("inputQueue", (size_t) -1, true);
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
    m_tmpObjElements.clear();
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
	mongo::BSONObj obj = mongo::BSONObj((char*)ptr);
	// it will be removed later
//	rt_queue_free(&m_inputQueue, ptr);

//	CMutexSection locker(&m_mutexData);
	// merge new data object with our
	std::map<CStringAnsi, mongo::BSONElement> elements;
	{
		mongo::BSONObjIterator objIt(m_dataIn);
		while (objIt.more()) {
			mongo::BSONElement elem = objIt.next();
			if (elem.isNull()) {
				continue;
			}
			elements[CStringAnsi(elem.fieldName(), (size_t) -1, true)] = elem;
		}
	}
	{
		mongo::BSONObjIterator objIt(obj);
		while (objIt.more()) {
			mongo::BSONElement elem = objIt.next();
			CStringAnsi name(elem.fieldName(), (size_t) -1, true);
			elements[name] = elem;
		}
	}
	mongo::BSONObjBuilder builder;
	for (auto it = elements.begin();it!=elements.end();it++) {
		builder.append((*it).second);
	}

	m_dataIn = builder.obj();

	// remove memory retrieved from queue
	rt_queue_free(&m_inputQueue, ptr);

	{
		m_elementsIn.clear();
		mongo::BSONObjIterator it(m_dataIn);
		while (it.more()) {
			mongo::BSONElement elem = it.next();
			m_elementsIn[CStringAnsi(elem.fieldName(), (size_t) -1, true)] = elem;
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
