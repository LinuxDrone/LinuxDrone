
#pragma once

#include "CTransport.h"
#include "network/CSocket"
#include "network/CSocketSelect"

#include <thread>

class CTransportNetwork : public CTransport
{
protected:
	class Device : public BaseDevice {
	public:
		Device(CSocket* s);
		virtual ~Device() {
			CSocket* socket = static_cast<CSocket*>(m_device);
			if (socket) {
				CTransportNetwork* transport = dynamic_cast<CTransportNetwork*>(m_transport);
				if (transport) {
					transport->m_select.removeRecvSocket(socket);
					transport->m_select.removeSendSocket(socket);
				}
				if (socket->isOpen()) {
					socket->disconnetGraceful(1000);
				}
				delete socket;
				m_device = 0;
			}
		}

		void sendData(const CByteArray& data);
		void onUpdate();

	private:
		CByteArray m_data;
		int        m_start;
		int        m_objSize;
		CByteArray m_dataForSend;
		CMutex     m_mutexDataForSend;

		void recvData();
	};

friend class Deice;
public:
	CTransportNetwork(uint16_t port);
	virtual ~CTransportNetwork();

	virtual bool init() override;
	virtual void deinit() override;

	virtual bool dataSend(void* device, const CByteArray& data) override;

private:
	CSocket       m_socketServer;
	CSocketSelect m_select;
	std::thread   m_thredSelect;
	bool          m_terminate;
	uint16_t      m_port;

	void threadSelectFunc();
	void acceptSocket();
};
