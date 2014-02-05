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

#include "ctransport.h"
#include "ctransportqueue.h"
#include "ctransportqueuexeno.h"
#include "ctransportnetwork.h"

CTransport::CTransport()
{
}

CTransport::~CTransport()
{
}

CTransport* CTransport::createTransport(TransportType type)
{
	CTransport* transport = 0;
	switch (type) {
		case TransportType_Unknown:
			return 0;
		case TransportType_Queue:
			transport = new CTransportQueue();
			break;
		case TransportType_QueueXeno:
			transport = new CTransportQueueXeno();
			break;
		case TransportType_Network:
			transport = new CTransportNetwork();
			break;
	}
	if (!transport){
		return 0;
	}
	transport->addRef();
	return transport;
}

CTransport::TransportType CTransport::type() const
{
	return TransportType_Unknown;
}

bool CTransport::start()
{
	return false;
}

bool CTransport::startServer()
{
	return false;
}

bool CTransport::stop()
{
	return false;
}

bool CTransport::started() const
{
	return false;
}

/////////////////////////////////////////////////////////////////////
//                                io                               //
/////////////////////////////////////////////////////////////////////

bool CTransport::send(const CByteArray& data)
{
	if (data.isEmpty()) {
		return true;
	}
	if (!started()) {
		return false;
	}
	CMutexSection locekr(&m_mutexData);
	m_dataForSend.append(data);
	return true;
}

/////////////////////////////////////////////////////////////////////
//                               signals                           //
/////////////////////////////////////////////////////////////////////

CL_Signal_v2<CTransport*, const CByteArray&>& CTransport::signalRecievedData()
{
	return m_signal_recievedData;
}
