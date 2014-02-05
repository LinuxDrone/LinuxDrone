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

#ifndef __gcs__ctransportqueuexeno__
#define __gcs__ctransportqueuexeno__

#include "ctransport.h"
#include "system/CQueueXeno"
#include "thread/CThread"
#include "thread/CMutex"

class CTransportQueueXeno : public CTransport
{
public:
	CTransportQueueXeno();
	virtual ~CTransportQueueXeno();

	virtual TransportType type() const;

	virtual bool start();
	virtual bool startServer();
	virtual bool stop();
	virtual bool started() const;
	
	void setQueueName(const CString& name);
	CString queueName() const;

// io
	virtual bool send(const CByteArray& data);

private:
	CString    m_queueName;
	CQueueXeno m_queue;
	
	bool    m_server;
	CThread m_thread;
	bool    m_terminate;
	
	void threadFunc();
};

#endif /* defined(__gcs__ctransportqueuexeno__) */
