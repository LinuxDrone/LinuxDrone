
#include "CSocket.h"
#include "CSocketName.h"
#include "../system/Logger.h"

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include "memory.h"

class CSocketPrivate
{
friend class CSocket;
public:
	CSocketPrivate() {
		m_socket       = INVALID_SOCKET;
		m_connected    = false;
		m_canRecv      = false;
		m_canSend      = false;
		m_lastError    = CSocket::SocketError_Ok;
		m_lastSysError = 0;
	}
	~CSocketPrivate() {
		disconnectAbortive();
	}

	void setSocket(SOCKET socket) {
		closeHandle();
		m_socket = socket;
	}
	SOCKET socket() const {
		return m_socket;
	}

	void closeHandle() {
		if (m_socket == INVALID_SOCKET) {
			return;
		}
		::close(m_socket);
		m_socket = INVALID_SOCKET;

		m_connected    = false;
		m_canRecv      = false;
		m_canSend      = false;
		m_lastError    = CSocket::SocketError_Ok;
		m_lastSysError = 0;
	}
	void disconnectAbortive() {
		if (m_socket == INVALID_SOCKET) {
			return;
		}
		shutdown(m_socket, SHUT_RDWR);
		closeHandle();
	}
	void disconnectGraceful(int timeOut) {
		if (m_socket == INVALID_SOCKET) {
			return;
		}
		shutdown(m_socket, SHUT_RDWR);

		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(m_socket, &rfds);
		timeval tv;
		tv.tv_sec = timeOut/1000;
		tv.tv_usec = (timeOut%1000)*1000;
		select(m_socket+1, &rfds, 0, 0, &tv);

		closeHandle();
	}

private:
	SOCKET               m_socket;
	bool                 m_connected;
	bool                 m_canRecv;
	bool                 m_canSend;
	CSocket::SocketError m_lastError;
	int                  m_lastSysError;

	std::function<void (CSocket*)> m_funcRecv;
};

CSocket::CSocket() :
		d(new CSocketPrivate())
{
}

CSocket::CSocket(const CSocket& other) :
		d(other.d)
{
}

CSocket::CSocket(SOCKET s) :
		d( new CSocketPrivate() )
{
	d->m_socket = s;
	d->m_connected = true;
}

CSocket::~CSocket()
{
}

/////////////////////////////////////////////////////////////////////
//            creating/destroying of socket handle                  //
/////////////////////////////////////////////////////////////////////

bool CSocket::createTcp()
{
	return createHandle(SOCK_STREAM);
}

bool CSocket::createUdp()
{
	return createHandle(SOCK_DGRAM);
}

void CSocket::disconnectAbortive()
{
	d->disconnectAbortive();
}

void CSocket::disconnetGraceful(int timeOut)
{
	d->disconnectGraceful(timeOut);
}

bool CSocket::isOpen() const
{
	return d->m_socket != INVALID_SOCKET;
}

SOCKET CSocket::handle() const
{
	return d->socket();
}

bool CSocket::setNonBlocking(bool nonBlocking /*= false*/)
{
	int nonblocking = nonBlocking == true ? 1 : 0;
	int err = ioctl(d->m_socket, FIONBIO, &nonblocking);
	if (INVALID_SOCKET == err)
	{
		checkError();
		return false;
	}

#ifdef SO_NOSIGPIPE
	int value = nonBlocking == true ? 1 : 0;
	err = setsockopt(d->m_socket, SOL_SOCKET, SO_NOSIGPIPE, (const char *) &value, sizeof(int));
	if (INVALID_SOCKET == err)
	{
		checkError();
		return false;
	}
#endif
	return true;
}

bool CSocket::setRecvTimeout(int millisec)
{
	if (!isOpen())
		return false;
	timeval tv;
	tv.tv_sec = millisec / 1000;
	tv.tv_usec = (millisec % 1000) * 1000;
	int err = setsockopt(d->m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
	if( INVALID_SOCKET == err )
	{
		checkError();
		return false;
	}
	return true;
}

bool CSocket::setEnableBroadcast()
{
	if (!isOpen()) {
		return false;
	}
	int val = 1;
	int err = setsockopt(d->m_socket, SOL_SOCKET, SO_BROADCAST, (const char*)&val, sizeof(val));
	if (INVALID_SOCKET == err)
	{
		checkError();
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////
//                            bind                                 //
/////////////////////////////////////////////////////////////////////

bool CSocket::bind(const CSocketName& address, bool reuseAddress)
{
	if (!isOpen())
		return false;
	if (reuseAddress) {
		int value = 1;
		int result = setsockopt(d->m_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&value, sizeof(int));
		if (INVALID_SOCKET == result) {
			checkError();
			return false;
		}
	}
	sockaddr_in addr;
	address.toSockaddr(AF_INET, (sockaddr*)&addr, sizeof(addr));
	int result = ::bind(d->m_socket, (const sockaddr*)&addr, sizeof(addr));
	if (INVALID_SOCKET == result) {
		checkError();
		return false;
	}
	return true;
}

bool CSocket::listen(int backlog)
{
	if (!isOpen()) {
		return false;
	}
	int err = ::listen(d->m_socket, backlog);
	if (err == INVALID_SOCKET)
	{
		checkError();
		return false;
	}
	return true;
}

bool CSocket::connect(const CSocketName& address, int timeout /*= -1*/)
{
	if (-1 != timeout)
	{
		// для начала переведем сокет в неблокирующий режим
		bool ret = setNonBlocking(true);
		if (ret) {
			return false;
		}
	}

	sockaddr_in addr;
	memset( &addr, 0, sizeof addr );
	address.toSockaddr( AF_INET, (sockaddr*)&addr, sizeof(addr) );
	int err = ::connect(d->m_socket, (sockaddr*)&addr, sizeof(addr));
	if (err != INVALID_SOCKET) {
		d->m_connected = true;
	} else {
		checkError();
		if (timeout == -1 || d->m_lastError != CSocket::SocketError_TryAgain)
			return false;
		else
		{
			fd_set fd;
			FD_ZERO(&fd);
			FD_SET(d->m_socket, &fd);

			timeval tv;
			d->m_lastError = CSocket::SocketError_TryAgain;
			int numTry = 0;
			do
			{
				tv.tv_sec  = timeout / 1000;
				tv.tv_usec = (timeout % 1000) * 1000;
				int num = select(d->m_socket+1, 0, &fd, 0, &tv);
				checkError();
				if (num)
				{
					d->m_connected   = true;
					d->m_lastError    = CSocket::SocketError_Ok;
					d->m_lastSysError = 0;
				}
				else
				{
					Logger() << "end try connect to socket";
					numTry++;
					if (numTry >= 5) {
						break;
					}
				}
			}
			while (d->m_lastError == CSocket::SocketError_TryAgain);
		}
	}
	// вернем назад блокирующий режим
	if (timeout != -1) {
		setNonBlocking(false);
	}
	if (d->m_connected)
		return true;
	return false;
}

CSocket CSocket::accept(CSocketName& address)
{
	sockaddr_in addr;
	socklen_t len = sizeof(addr);
	SOCKET s = ::accept(d->m_socket, (sockaddr*)&addr, &len);
	if (s == INVALID_SOCKET) {
		return CSocket();
	}
	address.fromSockaddr(AF_INET, (sockaddr*)&addr, sizeof(addr));
	CSocket newSocket(s);
	return newSocket;
}

/////////////////////////////////////////////////////////////////////
//                      send/recv of data                          //
/////////////////////////////////////////////////////////////////////

int CSocket::send( const void* buf, int size )
{
	if (!isOpen())
		return SocketError_InvalidCall;
	int sended = (int)::send(d->m_socket, (const char*)buf, size, 0);
	if (0 >= sended)
		checkError();
	d->m_canSend = false;
	return sended;
}

int CSocket::sendTo( const void* buf, int size, const CSocketName& name )
{
	if (!isOpen())
		return SocketError_InvalidCall;
	sockaddr_in addr;
	name.toSockaddr(AF_INET, (sockaddr*)&addr, sizeof(addr));
	int sent = (int)::sendto(d->m_socket, buf, size, 0, (sockaddr*)&addr, sizeof(addr));
	if (0 >= sent) {
		checkError();
	}
	d->m_canSend = false;
	return sent;
}

int CSocket::recv( void* buf, int size )
{
	if (!isOpen())
		return SocketError_InvalidCall;
	int received = (int)::recv(d->m_socket, (char*)buf, size, 0);
	if (0 == received || INVALID_SOCKET == received)
		checkError();
	else
		d->m_canRecv = false;
	return received;
}

int CSocket::recvFrom( void* buf, int len, CSocketName& address )
{
	sockaddr_in addr;
	socklen_t addrLen = sizeof(addr);
	memset(&addr, 0, sizeof(addr));
	int size = (int)::recvfrom(d->m_socket, (char*)buf, len, 0, (sockaddr*)&addr, &addrLen);
	if (INVALID_SOCKET != size && 0 != size) {
		address.fromSockaddr(AF_INET, (sockaddr*)&addr, addrLen);
		d->m_canRecv = false;
	} else {
		checkError();
	}
	return size;
}

/////////////////////////////////////////////////////////////////////
//                               flags                             //
/////////////////////////////////////////////////////////////////////

bool CSocket::isConnected() const
{
	return d->m_connected;
}

void CSocket::setWriteFlag(bool enable)
{
	d->m_canSend = enable;
}

bool CSocket::canWrite() const
{
	return d->m_canSend;
}

void CSocket::setRecvFlag(bool enable)
{
	d->m_canRecv = enable;
	if (enable) {
		d->m_funcRecv(this);
	}
}

bool CSocket::canRecv() const
{
	return d->m_canRecv;
}

/////////////////////////////////////////////////////////////////////
//                              errors                             //
/////////////////////////////////////////////////////////////////////

CSocket::SocketError CSocket::lastError() const
{
	return d->m_lastError;
}

int CSocket::lastSysError() const
{
	return d->m_lastSysError;
}

void CSocket::setRecvFuncNotify(const std::function<void (CSocket*)>& func)
{
	d->m_funcRecv = func;
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

bool CSocket::createHandle(int type)
{
	if (d->m_socket != INVALID_SOCKET) {
		return false;
	}
	d->m_socket = ::socket(AF_INET, type, 0);
	if (d->m_socket == INVALID_SOCKET) {
		checkError();
		return false;
	}
	return true;
}

void CSocket::closeHandle()
{
	d->closeHandle();
}

void CSocket::checkError()
{
	if (errno)
	{
		switch (errno)
		{
		case EBADF:
			d->m_lastError = SocketError_InvalidData;
			break;
		case EFAULT:
			d->m_lastError = SocketError_InvalidParam;
			break;
		case ENOBUFS:
		case ENOMEM:
			d->m_lastError = SocketError_OutOfMemory;
			break;
		case EINTR:
		case EAGAIN:
		case EINPROGRESS:
			d->m_lastError = SocketError_TryAgain;
			break;
		case ETIMEDOUT:
			d->m_lastError = SocketError_TimedOut;
			break;
		default:
			d->m_lastError = SocketError_Fail;
			break;
		}
		d->m_lastSysError = errno;
	}
}
