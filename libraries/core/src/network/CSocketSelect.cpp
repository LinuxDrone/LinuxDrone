
#include "CSocketSelect.h"
#include "../system/CSystem.h"
#include <sys/select.h>

CSocketSelect::CSocketSelect()
{
	m_startRecv = 0;
	m_startSend = 0;
}

CSocketSelect::~CSocketSelect()
{
}

/////////////////////////////////////////////////////////////////////
//                        sockets set                              //
/////////////////////////////////////////////////////////////////////

void CSocketSelect::addRecvSocket(CSocket* socket)
{
	if (!socket)
		return;
	CMutexSection locker(&m_mutexData);
	if (existSocketInVector(socket, m_recvSocket)) {
		return;
	}
	m_recvSocket.push_back( socket );
}

void CSocketSelect::removeRecvSocket(CSocket* socket)
{
	if (!socket) {
		return;
	}
	removeSocketFromVector(socket, m_recvSocket);
}

void CSocketSelect::addSendSocket(CSocket* socket)
{
	if (!socket) {
		return;
	}
	CMutexSection locker( &m_mutexData );
	if (existSocketInVector(socket, m_sendSocket)) {
		return;
	}
	m_sendSocket.push_back(socket);
}

void CSocketSelect::removeSendSocket(CSocket* socket)
{
	if (!socket) {
		return;
	}
	removeSocketFromVector(socket, m_sendSocket);
}

/////////////////////////////////////////////////////////////////////
//                               work                              //
/////////////////////////////////////////////////////////////////////

void CSocketSelect::select(int timeout /*= 0*/)
{
	CMutexSection locker(&m_mutexData);
	if (!m_recvSocket.size() && !m_sendSocket.size()) {
		if (timeout != -1) {
			CSystem::sleep(timeout);
		}
		return;
	}

	fd_set rfds;
	fd_set wrds;
	SOCKET s_max = 0;
	int num = 0;

	FD_ZERO(&rfds);
	FD_ZERO(&wrds);

	makeFdSet(m_recvSocket, rfds, num, s_max, m_startRecv);
	makeFdSet(m_sendSocket, wrds, num, s_max, m_startSend);
	locker.unlock();
	if (!num) {
		if (timeout != -1) {
			CSystem::sleep(timeout);
		}
		return;
	}
	timeval tv;
	tv.tv_sec = timeout / 1000;
	tv.tv_usec = (timeout % 1000)*1000;
	num = ::select( s_max+1, &rfds, &wrds, 0, &tv );
	locker.lock();
	if( !num )
		return;

	size_t size = m_recvSocket.size();
	for (CSocket* socket:m_recvSocket)
	{
		if (socket->handle() == INVALID_SOCKET) {
			continue;
		}
		if (FD_ISSET(socket->handle(), &rfds)) {
			socket->setRecvFlag(true);
//			m_signal_canRecvFromSocket.invoke(socket);
		}
		if (size != m_recvSocket.size()) {
			break;
		}
	}
	size = m_sendSocket.size();
	for (CSocket* socket:m_sendSocket)
	{
		if (!socket->isOpen()) {
			continue;
		}
		if (FD_ISSET(socket->handle(), &wrds)) {
			socket->setWriteFlag( true );
		}
		if (size != m_sendSocket.size()) {
			break;
		}
	}
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

bool CSocketSelect::existSocketInVector(CSocket* socket, const std::vector<CSocket*>& array)
{
	if (!socket) {
		return false;
	}
	CMutexSection locker(&m_mutexData);
	for (auto it = array.begin(); it != array.end(); ++it) {
		if (socket == *it) {
			return true;
		}
	}
	return false;
}

bool CSocketSelect::removeSocketFromVector(CSocket* socket, std::vector<CSocket*>& array)
{
	if (!socket) {
		return false;
	}
	CMutexSection locker(&m_mutexData);
	for (auto it = array.begin(); it != array.end(); ++it) {
		if(socket == *it) {
			array.erase(it);
			return true;
		}
	}
	return false;
}

void CSocketSelect::makeFdSet(const std::vector<CSocket*>& array, fd_set& set, int& num, SOCKET& s_max, int& start)
{
	if (array.size() == 0) {
		return;
	}
	if (FD_SETSIZE >= array.size()) {
		start = 0;
	}
	if (start >= int(array.size())) {
		start = 0;
	}
	int end = start;
	CSocket* s = 0;
	for (int i = 0;i<FD_SETSIZE;i++)
	{
		s = array[start];
		if( s )
		{
			if (s->isOpen() && !(&array == &m_sendSocket && s->canWrite())) {
				if( s->handle() > s_max )
					s_max = s->handle();
				num++;
				FD_SET(s->handle(), &set);
			}
		}
		start++;
		if (start >= int(array.size())) {
			start = 0;
		}
		if (start == end) {
			break;
		}
	}
}
