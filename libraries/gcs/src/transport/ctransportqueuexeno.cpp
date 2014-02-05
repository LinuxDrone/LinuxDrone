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

#include "ctransportqueuexeno.h"
#include "system/CSystem"
#include <stdio.h>
#include <errno.h>

CTransportQueueXeno::CTransportQueueXeno()
{
	m_server    = false;
	m_terminate = false;
}

CTransportQueueXeno::~CTransportQueueXeno()
{
}

CTransport::TransportType CTransportQueueXeno::type() const
{
	return TransportType_QueueXeno;
}

bool CTransportQueueXeno::start()
{
	if (started()) {
		return false;
	}
	m_terminate = false;
	m_server = false;
	m_thread.start(this, &CTransportQueueXeno::threadFunc);
	return true;
}

bool CTransportQueueXeno::startServer()
{
	if (started()) {
		return false;
	}
	m_terminate = false;
	m_server = true;
	m_thread.start(this, &CTransportQueueXeno::threadFunc);
	return true;
}

bool CTransportQueueXeno::stop()
{
	if (!started()) {
		return false;
	}
	m_terminate = true;
	m_thread.join();
	return true;
}

bool CTransportQueueXeno::started() const
{
	return m_thread.started();
}

void CTransportQueueXeno::setQueueName(const CString& name)
{
	m_queueName = name;
}

CString CTransportQueueXeno::queueName() const
{
	return m_queueName;
}

// io
bool CTransportQueueXeno::send(const CByteArray& data)
{
	if (data.isEmpty()) {
		return true;
	}
	if (m_queue.isOpened()) {
		int start = 0;
		while (start < int (data.size())) {
			int size = int (m_queue.write(data.data()+start, size_t (data.size()-start)));
			if (size <= 0) {
				if (size == -1 && errno == EAGAIN) {
					printf("error writing (error == EAGAIN)\n");
					CSystem::sleep(10);
					continue;
				}
				printf("queue closed (write). error = %d, errno = %d\n", size, errno);
				m_queue.close();
				exit(1);
				return false;
			}
			start += size;
			if (start < size) {
				printf("try writing\n");
			}
		}
		printf("sended data %d\n", data.size());
		return true;
	}
	return false;
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

void CTransportQueueXeno::threadFunc()
{
	while (!m_terminate) {
		if (!m_queue.isOpened()) {
			if (!m_queue.open(m_queueName.data())) {
				CSystem::sleep(100);
				continue;
			}
//			m_queue.setNonBlock(true);
			printf("queue opened\n");
		}
		// {
		// 	CMutexSection locker(&m_mutexData);
		// 	if (m_dataForSend.size()) {
		// 		int size = int (m_queueWr.write(m_dataForSend.data(), size_t (m_dataForSend.size())));
		// 		if (size <= 0) {
		// 			if (size == -1 && errno == EAGAIN) {
		// 				printf("error writing (error == EAGAIN)\n");
		// 				CSystem::sleep(10);
		// 				continue;
		// 			}
		// 			printf("queue closed\n");
		// 			m_queueWr.close();
		// 			m_queueRd.close();
		// 			m_dataForSend = CByteArray();
		// 			continue;
		// 		} else {
		// 			printf("sended data %d\n", size);
		// 			m_dataForSend.remove(0, size);
		// 		}
		// 	}
		// }
		char data[1024*8];
		int size = int (m_queue.read(data, sizeof data));
		if (size <= 0) {
			if (size == -1 && errno == EAGAIN) {
				CSystem::sleep(10);
				continue;
			}
			printf("queue closed\n");
			m_queue.close();
			{
				CMutexSection locker(&m_mutexData);
				m_dataForSend = CByteArray();
			}
		} else {
//			printf("recieved data %d\n", size);
			m_signal_recievedData.invoke(this, CByteArray(data, size, true));
		}
	}
}
