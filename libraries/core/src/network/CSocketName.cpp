
#include "CSocketName.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <memory.h>

CSocketName::CSocketName()
{
	m_port = 0;
}

CSocketName::CSocketName(uint16_t port)
{
	m_port = port;
}

CSocketName::CSocketName(const CString& address, uint16_t port)
{
	m_address = address;
	m_port    = port;
}

CSocketName::~CSocketName()
{
}

CString CSocketName::hostName()
{
	char buf[512];
	gethostname( buf, sizeof(buf) );
	CString str( buf );
	return str;
}

bool CSocketName::enumIps(std::vector<CString>& ips)
{
	CString hostName = CSocketName::hostName();
	hostent *addr = gethostbyname(hostName.data());
	CString ip;
	if (addr)
	{
		for (int i=0;addr->h_addr_list[i];i++)
		{
			ip = CString("%1.%2.%3.%4")
					.arg((unsigned char)addr->h_addr_list[i][0] )
					.arg((unsigned char)addr->h_addr_list[i][1] )
					.arg((unsigned char)addr->h_addr_list[i][2] )
					.arg((unsigned char)addr->h_addr_list[i][3] );
			ips.push_back(ip);
		}
	}
	return true;
}

void CSocketName::clearVector(std::vector<CString>& ips)
{
	ips.clear();
}

CString CSocketName::ipToBroadcast(const CString& ip)
{
	in_addr_t ipv4_address = inet_addr( ip.data() );
	if( ipv4_address == INADDR_NONE )
	{
		hostent *host = gethostbyname( ip.data() );
		if (host == 0)
			return CString();
		ipv4_address = *((in_addr_t *) host->h_addr_list[0]);
	}

	unsigned long addr_long = (unsigned long) ntohl(ipv4_address);
	return CString("%1.%2.%3.255").arg( int((addr_long & 0xff000000) >> 24) ).
		arg( int((addr_long & 0x00ff0000) >> 16) ).
		arg( int((addr_long & 0x0000ff00) >> 8) );
}

bool CSocketName::isIpAddressInString(const CString& address)
{
	if (address.isEmpty()) {
		return false;
	}
	int length = address.length();
	for (int i = 0;i<length; i++) {
		char c = address.at(i);
		if (c != '.' && !isdigit(c)) {
			return false;
		}
	}
	return true;
}

void CSocketName::setName(const CString& address, uint16_t port)
{
	m_address = address;
	m_port = port;
}

void CSocketName::setAddress(const CString& address)
{
	m_address = address;
}

CString CSocketName::address() const
{
	return m_address;
}

void CSocketName::setPort(uint16_t port)
{
	m_port = port;
}

uint16_t CSocketName::port() const
{
	return m_port;
}

bool CSocketName::operator ==(const CSocketName& other) const
{
	if( m_address != other.m_address )
		return false;
	if( m_port != other.m_port )
		return false;
	return true;
}

CString CSocketName::lookupIpv4() const
{
	if( m_address.isEmpty() )
		return CString();
	if( isIpAddressInString(m_address) )
		return m_address;
	in_addr_t ipv4_address = inet_addr( m_address.data() );
	if( ipv4_address == INADDR_NONE )
	{
		hostent *host = gethostbyname( m_address.data() );
		if (host == 0)
			return CString();
		ipv4_address = *((in_addr_t *) host->h_addr_list[0]);
	}

	unsigned long addr_long = (unsigned long) ntohl(ipv4_address);
	return CString("%1.%2.%3.%4").arg( int((addr_long & 0xff000000) >> 24) ).
		arg( int((addr_long & 0x00ff0000) >> 16) ).
		arg( int((addr_long & 0x0000ff00) >> 8) ).
		arg( int((addr_long & 0x000000ff)) );
}

CString CSocketName::lookupHostname() const
{
	in_addr_t ipv4_address = inet_addr( m_address.data() );
	if( ipv4_address != INADDR_NONE )
	{
		in_addr addr;
		memset( &addr, 0, sizeof(in_addr) );
		addr.s_addr = ipv4_address;
		hostent *host = gethostbyaddr( (const char *) &addr, sizeof(in_addr), AF_INET );
		if( host == 0 )
			return CString();
		return CString( host->h_name );
	}
	return m_address;
}

CSocketName CSocketName::toIpv4()
{
	return CSocketName( lookupIpv4(), port() );
}

CSocketName CSocketName::toHostname()
{
	return CSocketName( lookupHostname(), port() );
}

void CSocketName::fromSockaddr(int domain, struct sockaddr *addr, int len)
{
	if (domain != AF_INET)
		return;
	if( (size_t)len < sizeof(sockaddr_in))
		return;

	sockaddr_in *addr_in = (sockaddr_in *) addr;
	if (addr_in->sin_family != AF_INET)
		return;

	unsigned long addr_long = (unsigned long)ntohl( addr_in->sin_addr.s_addr );
	uint16_t port = ntohs( addr_in->sin_port );

	if( addr_in->sin_addr.s_addr == INADDR_ANY )
		m_address = CString();
	else
	{
		CString str_addr = CString("%1.%2.%3.%4").
			arg( int((addr_long & 0xff000000) >> 24) ).
			arg( int((addr_long & 0x00ff0000) >> 16) ).
			arg( int((addr_long & 0x0000ff00) >> 8) ).
			arg( int((addr_long & 0x000000ff)) );
		m_address = str_addr;
	}
	m_port = port;
}

void CSocketName::toSockaddr(int domain, struct sockaddr *pAddr, int len) const
{
	if( domain != AF_INET )
		return;

	if( (size_t)len < sizeof(sockaddr_in))
		return;

	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons( m_port );
	if( m_address.isEmpty() )
		addr.sin_addr.s_addr = INADDR_ANY;
	else
	{
		addr.sin_addr.s_addr = inet_addr( m_address.data() );
		if( addr.sin_addr.s_addr == INADDR_NONE )
		{
			hostent *host = gethostbyname( m_address.data() );
			if (host == 0)
				return;
			addr.sin_addr.s_addr = *((in_addr_t *) host->h_addr_list[0]);
		}
	}
	memcpy( pAddr, &addr, sizeof(sockaddr_in) );
}
