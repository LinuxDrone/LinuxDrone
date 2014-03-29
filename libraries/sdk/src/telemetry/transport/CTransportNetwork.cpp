
#include "CTransportNetwork.h"
#include "network/CSocketName"
#include "system/Logger"
#include <mongo/bson/bson.h>

#include <assert.h>
#include <memory.h>

CTransportNetwork::Device::Device(CSocket* s)
{
	m_device = s;
	s->setRecvFuncNotify([this](CSocket* s) mutable
			{
				recvData();
			});
}

void CTransportNetwork::Device::sendData(const CByteArray& data)
{
	CMutexSection locker(&m_mutexDataForSend);
	m_dataForSend.append(data);
}

void CTransportNetwork::Device::onUpdate()
{
	CSocket* s = static_cast<CSocket*>(m_device);
	if (!s) {
		return;
	}
	CMutexSection locker(&m_mutexDataForSend);
	if (s->canWrite() && !m_dataForSend.isEmpty()) {
		int len = s->send(m_dataForSend.data(), m_dataForSend.size());
		if (0 >= len) {
			locker.unlock();
			m_transport->removeDevice(this);
			return;
		}
		m_dataForSend.remove(0, len);
	}
}

void CTransportNetwork::Device::recvData()
{
	CSocket* s = static_cast<CSocket*>(m_device);
	if (!s) {
		return;
	}
	int len = 0;
	if (m_data.isEmpty()) {
		int size;
		len = s->recv(&size, sizeof(size));
		if (0 >= len) {
			m_transport->removeDevice(this);
			return;
		}
		m_data.setSize(uint32_t (size));
		memcpy(const_cast<char*>(m_data.data()), &size, sizeof(size));
		m_objSize = size;
		m_start = 4;
	}
	len = s->recv((const_cast<char*>(m_data.data()))+m_start, m_objSize-m_start);
	if (0 >= len) {
		m_transport->removeDevice(this);
		return;
	}
	m_start += len;
	assert(m_start <= m_objSize);
	if (m_start == m_objSize) {
		mongo::BSONObj obj = mongo::BSONObj(m_data.data());
		// send received object
		if (m_transport) {
			m_transport->notifyFuncReceivedData()(this, obj);
		}
		m_data = CByteArray();
	}
}


CTransportNetwork::CTransportNetwork(uint16_t port)
{
	m_port = port;
}

CTransportNetwork::~CTransportNetwork()
{
}

bool CTransportNetwork::init()
{
	if (!m_socketServer.createTcp()) {
		Logger() << __PRETTY_FUNCTION__ << ": error creating TCP/IP socket";
		return false;
	}
	if (!m_socketServer.bind(CSocketName(m_port), true)) {
		Logger() << __PRETTY_FUNCTION__ << ": error binding to port = " << m_port;
		m_socketServer.disconnectAbortive();
		return false;;
	}
	if (!m_socketServer.listen(100)) {
		Logger() << "Error start listen socket(server). error = " << m_socketServer.lastError() << ", sysError = " << m_socketServer.lastSysError();
		m_socketServer.disconnectAbortive();
		return false;
	}
	m_socketServer.setRecvFuncNotify([this](CSocket* recvSocket) mutable
			{
				if (recvSocket != &m_socketServer) {
					return;
				}
				acceptSocket();
			});
	m_select.addRecvSocket(&m_socketServer);
	m_thredSelect = std::thread([this]() mutable
	{
		threadSelectFunc();
	});
	return true;
}

void CTransportNetwork::deinit()
{
	m_terminate = true;
	m_thredSelect.join();
}

bool CTransportNetwork::dataSend(void* device, const CByteArray& data)
{
	Device* cDevice = static_cast<Device*>(device);
	if (!cDevice) {
		return false;
	}
	cDevice->sendData(data);
	return true;
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

void CTransportNetwork::threadSelectFunc()
{
	while (!m_terminate) {
		m_select.select(100);
		CMutexSection locker(&m_mutexDevices);
		for (auto it:m_devices) {
			Device* device = dynamic_cast<Device*>(it);
			if (!device) {
				continue;
			}
			device->onUpdate();
		}
	}
}

void CTransportNetwork::acceptSocket()
{
	CSocketName address;
	CSocket s = m_socketServer.accept(address);
	if (!s.isOpen()) {
		return;
	}
	CSocket* p = new CSocket(s);
	m_select.addRecvSocket(p);
	m_select.addSendSocket(p);
	addDevice(new Device(p));
}
