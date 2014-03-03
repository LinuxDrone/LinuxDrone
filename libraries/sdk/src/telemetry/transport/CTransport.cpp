
#include "CTransport.h"

CTransport::CTransport()
{
}

CTransport::~CTransport()
{
}

bool CTransport::init()
{
	return true;
}

void CTransport::deinit()
{
}

/////////////////////////////////////////////////////////////////////
//                        notify functions                         //
/////////////////////////////////////////////////////////////////////

void CTransport::setNotifyFuncAddedNewDevice(const std::function<void (CTransport*, void*)>& func)
{
	m_funcAddedNewDevice = func;
}

void CTransport::setNotifyFuncRemoveDevice(const std::function<void (void*)>& func)
{
	m_funcRemoveDevice = func;
}

void CTransport::setNotifyFuncReceivedData(const std::function<void (void*, const mongo::BSONObj&)>& func)
{
	m_funcRecievedData = func;
}

std::function<void (void*, const mongo::BSONObj&)>& CTransport::notifyFuncReceivedData()
{
	return m_funcRecievedData;
}

//===================================================================
//  p r o t e c t e d   f u n c t i o n s
//===================================================================

//-------------------------------------------------------------------
//  d e v i c e s
//-------------------------------------------------------------------

void CTransport::addDevice(BaseDevice *device)
{
	if (!device) {
		return;
	}
	device->m_transport = this;
	CMutexSection locker(&m_mutexDevices);
	m_devices.push_back(device);
	m_funcAddedNewDevice(this, device);
}

void CTransport::removeDevice(BaseDevice* device)
{
	if (!device) {
		return;
	}
	CMutexSection locker(&m_mutexDevices);
	for (auto it = m_devices.begin();it<m_devices.end();it++) {
		if (device == *it){
			m_devices.erase(it);
			m_funcRemoveDevice(device);
			delete device;
			return;
		}
	}
}
