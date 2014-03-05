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

#include "CImplSerialRegular.h"
      
bool CImplSerialRegular::portOpen()
{
    if (isOpened()) 
    {
        return false;
    }

    struct termios l_serialconfig; // port configuration - baud rate, parity, etc
    long l_baudrate; // will receive the baudrate
    
    m_fhandler = open((char *)m_portName.data(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (m_fhandler < 0) 
    {
        Logger() << "Failed to open serial port (" << m_portName << ")";
	m_Opened = false;
        m_fhandler=0;
	return false;
    }
    tcgetattr(m_fhandler, &l_serialconfig); //Gets the current options for the port

    switch(m_portSpeed) //Creating speed
    {
        case 4800:
            l_baudrate = B4800;
            break;
        case 9600:
            l_baudrate = B9600;
            break;
        case 19200:
            l_baudrate = B19200;
            break;
        case 38400:
            l_baudrate = B38400;
            break;
        case 57600:
            l_baudrate = B57600;
            break;            
        case 115200:
            l_baudrate = B115200;
            break;
    }
    cfsetispeed(&l_serialconfig, l_baudrate); //in baudrate
    cfsetospeed(&l_serialconfig, l_baudrate); //out baudrate
    l_serialconfig.c_cflag = CS8|CREAD|CLOCAL; //Doesnt make handshake and enable reading from the port, 8 bits per byte enable reading
    tcsetattr(m_fhandler, TCSANOW, &l_serialconfig); //Apply configurations right now
        
    return true;
}

bool CImplSerialRegular::portClose()
{
    if (!isOpened()) 
    {
        return true;
    }
    close(m_fhandler);    
    m_portFile = CString();
    m_portName = CString();
}

int CImplSerialRegular::serial_write(const void* data, size_t size)
{
    if (!isOpened()) 
    {
        return -1;
    }
    int len = write(m_fhandler, data, size);
    return len;
}

int CImplSerialRegular::serial_read(void* data, size_t size)
{
    if (!isOpened()) 
    {
        return -1;
    }
    int len = read(m_fhandler, data, size);
    return len;
}
