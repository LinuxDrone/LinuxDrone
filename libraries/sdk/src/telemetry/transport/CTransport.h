
#pragma once

#include "thread/CMutex"
#include "core/CByteArray"
#include <vector>
#include <functional>

namespace mongo {
class BSONObj;
}

class CTransport
{
protected:
	class BaseDevice {
	public:
		BaseDevice(void* device = 0) {
			m_transport = 0;
			m_device    = device;
		}
		virtual ~BaseDevice() {
		}

		CTransport * m_transport;

		void*  m_device;
		CMutex m_mutex;
	};
friend class BaseDeice;
public:
	CTransport();
	virtual ~CTransport();

	virtual bool init();
	virtual void deinit();

	virtual bool dataSend(void* device, const CByteArray& data) = 0;

// notify functions
	void setNotifyFuncAddedNewDevice(const std::function<void (CTransport*, void*)>& func);
	void setNotifyFuncRemoveDevice(const std::function<void (void*)>& func);
	void setNotifyFuncReceivedData(const std::function<void (void*, const mongo::BSONObj&)>& func);
	std::function<void (void*, const mongo::BSONObj&)>& notifyFuncReceivedData();

protected:
	std::vector<BaseDevice*> m_devices;
	CMutex m_mutexDevices;

	std::function<void (CTransport*, void*)> m_funcAddedNewDevice;
	std::function<void (void*)> m_funcRemoveDevice;
	std::function<void (void*, const mongo::BSONObj&)> m_funcRecievedData;

public:
// devices
	void addDevice( BaseDevice *device );
	void removeDevice(BaseDevice* device);
};
