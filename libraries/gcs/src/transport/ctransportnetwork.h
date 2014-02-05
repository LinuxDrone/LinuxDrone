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

#ifndef __gcs__ctransportnetwork__
#define __gcs__ctransportnetwork__

#include "ctransport.h"
#include "socket/CSocket"
#include "socket/CSocketSelect"
#include "thread/CThread"

class CTransportNetwork : public CTransport
{
friend class CClient;
public:
	enum SocketType {
		SocketType_Udp,
		SocketType_Tcp
	};
	
	class CClient : public CObject
	{
	public:
		CClient(CTransportNetwork* owner, const CSocket& socket);
		~CClient();
		
		void addDataToSend(const CByteArray& data);
		void send(const CByteArray& data);
		void sendImmediatly();

	private:
		CTransportNetwork * m_owner;
		CSocket             m_socket;
		
		CByteArray m_dataToSend;
		CMutex     m_mutexData;
		
		CL_Slot m_slotNeedRecvData;
		
		void needRecvData(CSocket* socket);
	};

public:
	CTransportNetwork();
	virtual ~CTransportNetwork();

	virtual TransportType type() const;

	virtual bool start();
	virtual bool startServer();
	virtual bool stop();
	virtual bool started() const;

// io
	virtual bool send(const CByteArray& data);

	void setSocketType(SocketType type);
	SocketType socketType() const;
	void setPort(uint16_t port);
	uint16_t port() const;
	void setAddress(const CString& address);
	CString adress() const;

private:
	SocketType    m_socketType;
	CSocket       m_socket;
	CSocketSelect m_select;
	bool          m_server;			// транспорт должен работать в режиме сервера (принимать входящие и общаться со своими клиентами)
	uint16_t      m_port;
	CString       m_address;
	CThread       m_thredSelect;
	bool          m_terminate;
	
	std::vector<CClient*> m_clients;
	CMutex                m_mutexClients;
	
	CL_Slot m_slotAcept;
	CL_Slot m_slotRecvData;
	
	bool initSocket();
	void threadSelect();
	void acceptSocket(CSocket* socket);
	void recvData(CSocket* socket);

// clients
	void removeAllClients();
	void removeClientByPtr(CClient* client);
};

#endif /* defined(__gcs__ctransportnetwork__) */
