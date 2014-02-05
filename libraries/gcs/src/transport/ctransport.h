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

#ifndef __gcs__ctransport__
#define __gcs__ctransport__

#include "core/CObject"
#include "core/CByteArray"
#include "signals/Signals"

class CTransport : public CObject
{
public:
	enum TransportType {
		TransportType_Unknown,
		TransportType_Queue,
		TransportType_QueueXeno,
		TransportType_Network
	};
public:
    CTransport();
    virtual ~CTransport();

	static CTransport* createTransport(TransportType type);

	virtual TransportType type() const;

	virtual bool start();
	virtual bool startServer();
	virtual bool stop();
	virtual bool started() const;

// io
	virtual bool send(const CByteArray& data);
	
// signals
	CL_Signal_v2<CTransport*, const CByteArray&>& signalRecievedData();

protected:
	CByteArray m_dataForSend;
	CMutex     m_mutexData;
	
	CL_Signal_v2<CTransport*, const CByteArray&> m_signal_recievedData;
};

#endif /* defined(__gcs__ctransport__) */
