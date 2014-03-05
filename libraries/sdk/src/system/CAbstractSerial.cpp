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

#include "CAbstractSerial.h"

CAbstractSerial::CAbstractSerial()
{
    
}

CAbstractSerial::CAbstractSerial(CString portFile, int portSpeed,CString portName)
{
    m_portName = portName;
    m_portSpeed = portSpeed;    
    m_portFile = portFile;
    m_Opened = false;
}

CAbstractSerial::~CAbstractSerial()
{

}
	
bool CAbstractSerial::isOpened()
{
    return m_Opened;
}

CString CAbstractSerial::getPortName()
{
    return m_portName;
}

int CAbstractSerial::getPortSpeed()
{
    return m_portSpeed;
}

CString CAbstractSerial::getPortFile()
{
    return m_portFile;
}
        
bool CAbstractSerial::setPortFile(CString portFile)
{
    m_portFile = portFile;
}
bool CAbstractSerial::setPortSpeed(int portSpeed)
{
    m_portSpeed = portSpeed;
}
bool CAbstractSerial::setPortName(CString portName)
{
    m_portName = portName;
}
