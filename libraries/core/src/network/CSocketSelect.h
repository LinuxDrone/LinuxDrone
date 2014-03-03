
#pragma once

#include "CSocket.h"
#include "../thread/CMutex.h"
#include <vector>

class CSocket;

class CSocketSelect
{
public:
	CSocketSelect();
	~CSocketSelect();

// sockets set
	void addRecvSocket(CSocket* socket);
	void removeRecvSocket(CSocket* socket);
	void addSendSocket(CSocket* socket);
	void removeSendSocket(CSocket* socket);

// work
	void select(int timeout = 0);

private:
	std::vector<CSocket*> m_recvSocket;
	std::vector<CSocket*> m_sendSocket;
	int                   m_startRecv;
	int                   m_startSend;
	CMutex                m_mutexData;

	bool existSocketInVector(CSocket* socket, const std::vector<CSocket*>& array);
	bool removeSocketFromVector(CSocket* socket, std::vector<CSocket*>& array);
	void makeFdSet(const std::vector<CSocket*>& array, fd_set& set, int& num, SOCKET& s_max, int& start);
};
