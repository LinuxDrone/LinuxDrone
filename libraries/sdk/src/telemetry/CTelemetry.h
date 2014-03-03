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

#include "../module/cmodule.h"
#include "network/CSocketSelect"
#include <thread>

class CModuleSystem;
class CTransport;

namespace mongo {
class BSONObj;
}

class CTelemetry : public CModule
{
private:
	typedef struct tagQuery
	{
		tagQuery() {
			elapsedTime = 0;
		}
		CString instanceName;
		CString idName;
		std::set<CString> outs;
		void * device;

		RTIME updateTime;
		RTIME elapsedTime;

	} QUERY, *PQUERY;

public:
	CTelemetry();
	~CTelemetry();

	virtual bool init(const mongo::BSONObj& initObject) override;
	virtual bool start() override;

	void setModuleSystem(CModuleSystem* system);
	CModuleSystem* modulesystem();

// queries
	bool addQuery(const mongo::BSONObj& queryObj, void* device);
	void removeAllQueries();
	void removeQuery(void* device, const CString& idName);

protected:
	CModuleSystem * m_moduleSystem;
	std::vector<QUERY> m_queries;
	CMutex             m_mutexQueries;
	RTIME              m_lastTime;

	CTransport                   * m_transport;
	std::map<void*, CTransport*>   m_devices;
	CMutex                         m_mutexDevices;

	void funcMainTask();

// network
	void initNetwork();
	void deinitNetwork();
	void sendToDevice(const mongo::BSONObj& obj, void* device);

// notify
	void addedNewDevice(CTransport* transport, void* device);
	void removeDevice(void* device);
	void receivedData(void* device, const mongo::BSONObj& obj);
};
