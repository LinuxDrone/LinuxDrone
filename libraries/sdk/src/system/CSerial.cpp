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

#include "CSerial.h"

CSerial::CSerial()
{
    
}
CSerial::~CSerial()
{

}
	
bool CSerial::isOpened()
{
    return m_Opened;
}

CString CSerial::getPortName()
{
    return m_portName;
}

int CSerial::getPortSpeed()
{
    return m_portSpeed;
}

CString CSerial::getPortFile()
{
    return m_portFile;
}
        
bool CSerial::setPortFile(CString const &portFile)
{
    m_portFile = portFile;
}
bool CSerial::setPortSpeed(int portSpeed)
{
    m_portSpeed = portSpeed;
}
bool CSerial::setPortName(CString const &portName)
{
    m_portName = portName;
}