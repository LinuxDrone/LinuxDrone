
#pragma once

#include <memory>
#include <functional>

#ifndef SOCKET
#define SOCKET int
#endif // SOCKET

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif // INVALID_SOCKET

class CSocketName;
class CSocketPrivate;

class CSocket
{
public:
	enum SocketError {
		SocketError_Ok,
		SocketError_Fail,
		SocketError_InvalidData,
		SocketError_InvalidParam,
		SocketError_InvalidCall,
		SocketError_OutOfMemory,
		SocketError_TryAgain,
		SocketError_TimedOut
	};

public:
	CSocket();
	CSocket(const CSocket& other);
	CSocket(SOCKET s);
	~CSocket();

// creating/destroying of socket handle
	bool createTcp();
	bool createUdp();
	void disconnectAbortive();
	void disconnetGraceful(int timeOut);
	bool isOpen() const;
	SOCKET handle() const;
	bool setNonBlocking(bool nonBlocking = false);
	bool setRecvTimeout(int millisec);
	bool setEnableBroadcast();

// bind
	bool bind( const CSocketName& address, bool reuseAddress );
	bool listen( int backlog );
	bool connect( const CSocketName& address, int timeout = -1 );
	CSocket accept( CSocketName& address );

// send/recv of data
	int send( const void* buf, int size );
	int sendTo( const void* buf, int size, const CSocketName& name );
	int recv( void* buf, int size );
	int recvFrom( void* buf, int len, CSocketName& address );

// flags
	bool isConnected() const;
	void setWriteFlag( bool enable );
	bool canWrite() const;
	void setRecvFlag( bool enable );
	bool canRecv() const;

// errors
	SocketError lastError() const;
	int lastSysError() const;

	void setRecvFuncNotify(const std::function<void (CSocket*)>& func);

private:
	std::shared_ptr<CSocketPrivate> d;

	bool createHandle(int type);
	void closeHandle();
	void checkError();
};
