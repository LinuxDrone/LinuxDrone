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

#ifndef __gcs__ctransportqueue__
#define __gcs__ctransportqueue__

#include "ctransport.h"
#include "system/CQueueIPC"
#include "core/CByteArray"
#include <vector>

class CTransportQueue : public CTransport
{
public:
    CTransportQueue();
    virtual ~CTransportQueue();

    bool open(int key = DEFAULT_QUEUE_KEY);
    void close();
    bool opened() const;
    
// базовые данные
    void setRecvMsgId(int msgId);
    int recvdMsgId() const;
    void setSendMsgId(int msgId);
    int sendMsgId() const;
    
// io
    CByteArray readPacket();

private:
    CQueueIPC m_queue;
    bool      m_createdQueue;
    int       m_msgIdRecv;
    int       m_msgIdSend;
    
    std::vector<CByteArray> m_dataToSend;
    std::vector<CByteArray> m_dataRecieved;
    
    void recvThread();
};

#endif /* defined(__gcs__ctransportqueue__) */
