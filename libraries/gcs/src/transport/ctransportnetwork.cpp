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

#include "ctransportnetwork.h"
#include "system/CSystem"
#include "socket/CSocketName"

CTransportNetwork::CTransportNetwork()
{
	m_socketType = SocketType_Tcp;
	m_server     = false;
	m_port       = 3443;
}

CTransportNetwork::~CTransportNetwork()
{
}

CTransport::TransportType CTransportNetwork::type() const
{
	return TransportType_Network;
}

bool CTransportNetwork::start()
{
	if (started()) {
		return false;
	}
	m_terminate = false;
	m_thredSelect.start(this, &CTransportNetwork::threadSelect);
	return true;
}

bool CTransportNetwork::startServer()
{
	if (started()) {
		return false;
	}
	m_server = true;
	m_terminate = false;
	m_thredSelect.start(this, &CTransportNetwork::threadSelect);
	return true;
}

bool CTransportNetwork::stop()
{
	if (!started()) {
		return false;
	}
	m_terminate = true;
	m_thredSelect.join();
	return false;
}

bool CTransportNetwork::started() const
{
	return m_thredSelect.started();
}

/////////////////////////////////////////////////////////////////////
//                                 io                              //
/////////////////////////////////////////////////////////////////////

bool CTransportNetwork::send(const CByteArray& data)
{
	if (data.isEmpty()) {
		return true;
	}
	if (!started()) {
		return false;
	}
	if (m_server) {
		CMutexSection locker(&m_mutexClients);
		for (auto it = m_clients.begin();it<m_clients.end();it++) {
			(*it)->send(data);
		}
	} else {
		CMutexSection locker(&m_mutexData);
		m_dataForSend.append(data);
	}
	return true;
}

void CTransportNetwork::setSocketType(SocketType type)
{
	m_socketType = type;
}

CTransportNetwork::SocketType CTransportNetwork::socketType() const
{
	return m_socketType;
}

void CTransportNetwork::setPort(uint16_t port)
{
	m_port = port;
}

uint16_t CTransportNetwork::port() const
{
	return m_port;
}

void CTransportNetwork::setAddress(const CString& address)
{
	m_address = address;
}

CString CTransportNetwork::adress() const
{
	return m_address;
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

bool CTransportNetwork::initSocket()
{
	bool ret = false;
	if (m_socketType == SocketType_Tcp) {
		ret = m_socket.createTcp();
	} else {
		ret = m_socket.createUdp();
	}
	if (!ret) {
		return false;
	}
	if (m_server) {
		ret = m_socket.bind(CSocketName(m_port), true);
		if (!ret) {
			return false;
		}
		ret = m_socket.listen(100);
		if (!ret) {
			return false;
		}
		m_slotAcept = m_select.signal_canRecvFromSocket().connect(this, &CTransportNetwork::acceptSocket);
	} else {
		ret = m_socket.connect(CSocketName(m_address, m_port));
		if (!ret) {
			return false;
		}
		m_select.addSendSocket(&m_socket);

		m_slotRecvData = m_select.signal_canRecvFromSocket().connect(this, &CTransportNetwork::recvData);
	}
	m_select.addRecvSocket(&m_socket);
	return true;
}

void CTransportNetwork::threadSelect()
{
	initSocket();
	while (!m_terminate) {
		m_select.select(50);
		if (!m_server) {
			if (m_socket.canWrite()) {
				CMutexSection locker(&m_mutexData);
				if (!m_dataForSend.isEmpty()) {
					int size = m_socket.send(m_dataForSend.data(), int (m_dataForSend.size()));
					if (size <= 0) {
						m_terminate = true;
					} else {
						m_dataForSend.remove(0, size);
					}
				}
			}
		} else {
			CMutexSection locker(&m_mutexClients);
			for (auto it = m_clients.begin();it<m_clients.end();it++) {
				(*it)->sendImmediatly();
			}
//			CSystem::sleep(10);
		}
	}
}

void CTransportNetwork::acceptSocket(CSocket* socket)
{
	if (socket != &m_socket) {
		return;
	}
	CSocketName address;
	CSocket s = m_socket.accept(address);
	CClient * client = new CClient(this, s);
	client->addRef();
	CMutexSection locker(&m_mutexClients);
	m_clients.push_back(client);
}

void CTransportNetwork::recvData(CSocket* socket)
{
	if (!socket || socket != &m_socket) {
		return;
	}
	char data[1024*8];
	int size = m_socket.recv(data, sizeof data);
	if (size <= 0) {
		m_terminate = true;
	} else {
		m_signal_recievedData.invoke(this, CByteArray(data, size, true));
	}
}

//-------------------------------------------------------------------
//  c l i e n t s
//-------------------------------------------------------------------

void CTransportNetwork::removeAllClients()
{
	CMutexSection locker(&m_mutexClients);
	for (auto it = m_clients.begin();it<m_clients.end();it++) {
		SAFE_RELEASE((*it));
	}
	m_clients.clear();
}

void CTransportNetwork::removeClientByPtr(CClient* client)
{
	if (!client) {
		return;
	}
	CMutexSection locker(&m_mutexClients);
	for (auto it = m_clients.begin();it<m_clients.end();it++) {
		if (*it == client) {
			SAFE_RELEASE((*it));
			m_clients.erase(it);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
//                                                                 //
//            implement of CTransportNetwork::CClient              //
//                                                                 //
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

CTransportNetwork::CClient::CClient(CTransportNetwork* owner, const CSocket& socket)
{
	m_owner  = owner;
	m_socket = socket;
	if (m_owner) {
		m_owner->m_select.addRecvSocket(&m_socket);
		m_owner->m_select.addSendSocket(&m_socket);
		m_slotNeedRecvData = m_owner->m_select.signal_canRecvFromSocket().connect(this, &CTransportNetwork::CClient::needRecvData);
	}
}

CTransportNetwork::CClient::~CClient()
{
	m_socket.disconnectGraceful(100);
	if (m_owner) {
		m_owner->m_select.removeRecvSocket(&m_socket);
		m_owner->m_select.removeSendSocket(&m_socket);
		m_owner->removeClientByPtr(this);
	}
}

void CTransportNetwork::CClient::addDataToSend(const CByteArray& data)
{
	CMutexSection locker(&m_mutexData);
	m_dataToSend.append(data);
}

void CTransportNetwork::CClient::send(const CByteArray& data)
{
	CMutexSection locker(&m_mutexData);
	m_dataToSend.append(data);
}

void CTransportNetwork::CClient::sendImmediatly()
{
	if (!m_socket.canWrite()) {
		return;
	}
	CMutexSection locekr(&m_mutexData);
	if (m_dataToSend.isEmpty()) {
		return;
	}
	int size = m_socket.send(m_dataToSend.data(), int (m_dataToSend.size()));
	if (size <= 0) {
		if (m_owner) {
			this->addRef();
			m_owner->removeClientByPtr(this);
			this->release();
			return;
		}
	} else {
		m_dataToSend.remove(0, int (size));
	}
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

void CTransportNetwork::CClient::needRecvData(CSocket* socket)
{
	if (!socket || socket != &m_socket) {
		return;
	}
	char data[1024*8];
	int size = m_socket.recv(data, sizeof data);
	if (size <= 0) {
		if (m_owner) {
			this->addRef();
			m_owner->removeClientByPtr(this);
			this->release();
			return;
		}
	} else {
		if (m_owner) {
			m_owner->signalRecievedData().invoke(m_owner, CByteArray(data, size, true));
		}
	}
}
