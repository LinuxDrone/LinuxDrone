
#pragma once

#include "../text/CString.h"
#include <vector>

class CSocketName
{
public:
	CSocketName();
	CSocketName(uint16_t port);
	CSocketName(const CString& address, uint16_t port);
	~CSocketName();

	static CString hostName();
	static bool enumIps(std::vector<CString>& ips);
	static void clearVector(std::vector<CString>& ips);
	static CString ipToBroadcast(const CString& ip);
	static bool isIpAddressInString(const CString& address);

	void setName(const CString& address, uint16_t port);
	void setAddress(const CString& address);
	CString address() const;
	void setPort(uint16_t port);
	uint16_t port() const;
	bool operator ==(const CSocketName& other) const;

	CString lookupIpv4() const;
	CString lookupHostname() const;
	CSocketName toIpv4();
	CSocketName toHostname();
	void fromSockaddr(int domain, struct sockaddr *addr, int len);
	void toSockaddr(int domain, struct sockaddr *pAddr, int len) const;

private:
	uint16_t m_port;
	CString m_address;
};
