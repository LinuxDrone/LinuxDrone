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

#include "ctransportqueue.h"

CTransportQueue::CTransportQueue()
{
    m_createdQueue = false;
    m_msgIdRecv    = 1;
    m_msgIdSend    = 2;
}

CTransportQueue::~CTransportQueue()
{
    close();
}

bool CTransportQueue::open(int key /*= DEFAULT_QUEUE_KEY*/)
{
    bool ret = m_queue.open();
    if (!ret) {
        ret = m_queue.create();
        if (ret) {
            m_createdQueue = true;
        }
    }
    if (!ret) {
        return false;
    }
    return true;
}

void CTransportQueue::close()
{
    if (!m_queue.isOpened()) {
        return;
    }
    if (m_createdQueue) {
        m_queue.destroy();
        m_createdQueue = false;
    }
    m_queue.close();
}

bool CTransportQueue::opened() const
{
    return m_queue.isOpened();
}

/////////////////////////////////////////////////////////////////////
//                          базовые данные                         //
/////////////////////////////////////////////////////////////////////

void CTransportQueue::setRecvMsgId(int msgId)
{
    m_msgIdRecv = msgId;
}

int CTransportQueue::recvdMsgId() const
{
    return m_msgIdRecv;
}

void CTransportQueue::setSendMsgId(int msgId)
{
    m_msgIdSend = msgId;
}

int CTransportQueue::sendMsgId() const
{
    return m_msgIdSend;
}

/////////////////////////////////////////////////////////////////////
//                                  io                             //
/////////////////////////////////////////////////////////////////////

CByteArray CTransportQueue::readPacket()
{
    return CByteArray();
}

//===================================================================
//  p r i v a t e   f u n t i o n s
//===================================================================

void CTransportQueue::recvThread()
{
    while (1) {
        if (!m_queue.isOpened()) {
//            sleep();
            continue;
        }
    }
}
