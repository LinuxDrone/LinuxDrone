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

#include "CTelemetry.h"
#include "system/Logger"
#include "transport/CTransportNetwork.h"

#include <mongo/bson/bsontypes.h>

#define TELEMETRY_COMMAND "command"
#define TELEMETRY_COMMAND_ADD_QUERY "Subscribe"
#define TELEMETRY_COMMAND_REMOVE_QUERY "Unsubscribe"

#define TELEMETRY_INSTANCENAME "instanceName"
#define TELEMETRY_IDNAME "idName"
#define TELEMETRY_UPDATETIME "updateTime"
#define TELEMETRY_OUTS "outs"
#define TELEMETRY_PORT 3128

CTelemetry::CTelemetry() :
	CModule(1024)
{
	m_moduleSystem = 0;
	m_terminate    = false;
	m_lastTime     = 0;
	m_transport    = 0;
}

CTelemetry::~CTelemetry()
{
	stop();
	removeAllQueries();
	deinitNetwork();
}

bool CTelemetry::init(const mongo::BSONObj& initObject)
{
	bool ret = CModule::init(initObject);
	if (!ret) {
		return false;
	}
	initNetwork();
	return true;
}

bool CTelemetry::start()
{
	m_lastTime = rt_timer_read();

	CModule::start(this, &CTelemetry::funcMainTask);
	return true;
}

void CTelemetry::setModuleSystem(CModuleSystem* system)
{
	m_moduleSystem = system;
}

CModuleSystem* CTelemetry::modulesystem()
{
	return m_moduleSystem;
}

/////////////////////////////////////////////////////////////////////
//                            queries                              //
/////////////////////////////////////////////////////////////////////

bool CTelemetry::addQuery(const mongo::BSONObj& queryObj, void* device)
{
	if (queryObj.isEmpty()) {
		return false;
	}
	QUERY query;
	query.device = device;
	if (!queryObj.hasElement(TELEMETRY_INSTANCENAME) ||
		!queryObj.hasElement(TELEMETRY_IDNAME) ||
		!queryObj.hasElement(TELEMETRY_UPDATETIME) ||
		!queryObj.hasElement(TELEMETRY_OUTS)) {
		return false;
	}
	mongo::BSONObjIterator it(queryObj);
	while (it.more()) {
		mongo::BSONElement elem = it.next();
		if (CString(TELEMETRY_INSTANCENAME) == elem.fieldName()) {
			query.instanceName = elem.String().c_str();
		} else if (CString(TELEMETRY_IDNAME) == elem.fieldName()) {
			query.idName = elem.String().c_str();
		} else if (CString(TELEMETRY_UPDATETIME) == elem.fieldName()) {
			query.updateTime = float (elem.Number())*1000000;
		} else if (CString(TELEMETRY_OUTS) == elem.fieldName()) {
			mongo::BSONObj outs = elem.Obj();
			mongo::BSONObjIterator it(outs);
			while (it.more()) {
				mongo::BSONElement elem = it.next();
				query.outs.insert(elem.String().c_str());
			}
		}
	}
	CMutexSection locker(&m_mutexQueries);
	m_queries.push_back(query);
	return true;
}

void CTelemetry::removeAllQueries()
{
	CMutexSection locker(&m_mutexQueries);
	m_queries.clear();
}

void CTelemetry::removeQuery(void* device, const CString& idName)
{
	CMutexSection locker(&m_mutexQueries);
	for (auto it = m_queries.begin();it<m_queries.end();it++) {
		if (it->device == device && it->idName == idName) {
			m_queries.erase(it);
			return;
		}
	}
}

//===================================================================
//  p r o t e c t e d   f u n c t i o n s
//===================================================================

void CTelemetry::funcMainTask()
{
	CMutexSection locker(&m_mutexQueries);
	RTIME currentTime = rt_timer_read();
	RTIME r_elapsed = currentTime - m_lastTime;
	m_lastTime = currentTime;
	std::map<CString, mongo::BSONObj> objects;

	if (m_queries.empty()) {
		m_task.sleep(100);
	}

	for (QUERY& query:m_queries) {
		query.elapsedTime += r_elapsed;
		if (query.updateTime <= query.elapsedTime) {
			query.elapsedTime = 0;
			if (!objects.count(query.instanceName)) {
				mongo::BSONObj obj = CModule::recvBsonFromHeap(query.instanceName);
				if (obj.isEmpty()) {
					continue;
				}
				objects[query.instanceName] = obj;
			}
			mongo::BSONObj obj = objects[query.instanceName];
			mongo::BSONObjBuilder builder;
			builder << "idName" << query.idName.data();
			for (auto itOut = query.outs.begin();itOut!=query.outs.end();itOut++) {
				if (obj.hasElement((*itOut).data())) {
					mongo::BSONElement elem = obj[(*itOut).data()];
					builder << elem.fieldName() << elem;
				}
			}
			obj = builder.obj();
			sendToDevice(obj, query.device);
		}
	}
}

//===================================================================
//  n e t w o r k
//===================================================================

void CTelemetry::initNetwork()
{
	m_transport = new CTransportNetwork(TELEMETRY_PORT);
	if (!m_transport) {
		return;
	}
	m_transport->setNotifyFuncAddedNewDevice([this](CTransport* transport, void* device) mutable
			{
				addedNewDevice(transport, device);
			});
	m_transport->setNotifyFuncRemoveDevice([this](void* device) mutable
			{
				removeDevice(device);
			});
	m_transport->setNotifyFuncReceivedData([this](void* device, const mongo::BSONObj& obj)
			{
				receivedData(device, obj);
			});
	m_transport->init();
}

void CTelemetry::deinitNetwork()
{
	m_transport->deinit();
}

void CTelemetry::sendToDevice(const mongo::BSONObj& obj, void* device)
{
	CMutexSection locker(&m_mutexDevices);
	if (!m_devices.count(device)) {
		return;
	}
	CTransport* transport = m_devices[device];
	if (!transport) {
		return;
	}
	transport->dataSend(device, CByteArray(obj.objdata(), obj.objsize(), true));
}

//-------------------------------------------------------------------
//  n o t i f y
//-------------------------------------------------------------------

void CTelemetry::addedNewDevice(CTransport* transport, void* device)
{
	CMutexSection locker(&m_mutexDevices);
	m_devices[device] = transport;
}

void CTelemetry::removeDevice(void* device)
{
	{
		CMutexSection locker(&m_mutexDevices);
		m_devices.erase(device);
	}
	{// remove all queries from device
		CMutexSection locker(&m_mutexQueries);
		for (auto it = m_queries.begin();it<m_queries.end();it++) {
			if (device == (*it).device) {
				m_queries.erase(it);
				it--;
			}
		}
	}
}

void CTelemetry::receivedData(void* device, const mongo::BSONObj& obj)
{
	if (obj.isEmpty()) {
		return;
	}
	if (!obj.hasElement(TELEMETRY_COMMAND)) {
		return;
	}
	mongo::BSONElement elem = obj[TELEMETRY_COMMAND];
	if (elem.type() != mongo::String) {
		return;
	}
	CString cmd = elem.String().c_str();
	if (cmd == TELEMETRY_COMMAND_ADD_QUERY) {
		addQuery(obj, device);
	} else if (cmd == TELEMETRY_COMMAND_REMOVE_QUERY) {
		if (!obj.hasElement(TELEMETRY_IDNAME)) {
			return;
		}
		removeQuery(device, obj[TELEMETRY_IDNAME].String().c_str());
	}
}
