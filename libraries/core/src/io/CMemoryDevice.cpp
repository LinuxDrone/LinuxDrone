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

#include "CMemoryDevice.h"

CMemoryDevice::CMemoryDevice(CByteArray* data)
{
    m_data  = nullptr;
    m_pos   = 0;
    m_bOpen = false;
}

CMemoryDevice::~CMemoryDevice()
{
}

const CByteArray &CMemoryDevice::data() const
{
    if (m_data == nullptr) {
        static CByteArray tmp;
        return tmp;
    }
    return *m_data;
}

CIODevice::DeviceType CMemoryDevice::deviceType() const {
    return CIODevice::deviceType();
}

/////////////////////////////////////////////////////////////////////
//                          open/close                             //
/////////////////////////////////////////////////////////////////////

bool CMemoryDevice::open(uint32_t mode)
{
    if (isOpen()) {
        return false;
    }
    m_bOpen = true;
    m_pos   = 0;
    if ((mode & CIODevice::WriteOnly) && !(mode & CIODevice::ReadOnly)) {
        m_data->setSize(0);
    }
    return true;
}

void CMemoryDevice::close()
{
    m_bOpen = false;
}

bool CMemoryDevice::isOpen() const
{
    return m_bOpen;
}

uint64_t CMemoryDevice::write(const void *data, uint64_t len)
{
    if (!len) {
        return 0;
    }
    if (!m_data) {
        return 0;
    }
    if (m_data->size() < m_pos+len) {
        const int startSize = m_data->size();
        m_data->setSize(uint32_t(m_pos+len));
        if (m_data->size() < m_pos+len) {
            m_data->setSize(startSize);
            return 0;
        }
    }
    if (data) {
        memcpy((char*)m_data->data()+m_pos, data, len);
    }
    m_pos += len;
    return len;
}

uint64_t CMemoryDevice::read(void *data, uint64_t len) const
{
    if (0 > len || !len) {
        return 0;
    }
    if (len > int(size()-m_pos)) {
        len = int(size()-m_pos);
    }
    if (data) {
        memcpy(data, m_data->data()+m_pos, len);
    }
    m_pos += len;
    return len;
}

bool CMemoryDevice::seek(uint64_t pos)
{
    if (pos > (int64_t)size()) {
        pos = size();
    }
    m_pos = pos;
    return true;
}

uint64_t CMemoryDevice::pos() const
{
    return m_pos;
}

uint64_t CMemoryDevice::size() const
{
    if (!m_data) {
        return 0;
    }
    return m_data->size();
}
