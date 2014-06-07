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

#include <assert.h>
#include "CSerialUART.h"
      
bool CSerialUART::portOpen()
{    
    if (isOpened()) 
    {
        return false;
    }

    struct termios l_serialconfig; // port configuration - baud rate, parity, etc
    speed_t l_baudrate = 0; // will receive the baudrate
    
    m_fhandler = open((char *)m_portFile.data(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (m_fhandler < 0) 
    {
        Logger() << "Failed to open serial port (" << m_portName << ")";
        m_Opened = false;
        m_fhandler=0;
    	return false;
    }
    fcntl(m_fhandler, F_SETFL, 0);
    m_Opened = true;
    //Logger() << "Reading attributes (" << m_portName << ")";
    tcgetattr(m_fhandler, &l_serialconfig); //Gets the current options for the port
    //Logger() << "Speed (" << m_portSpeed << ")";
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
        default:
            assert(0 == "invalid speed");
            l_baudrate = 0;
            break;
    }
    //Logger() << "Setting (" << m_portName << ")";
    cfsetispeed(&l_serialconfig, l_baudrate); //in baudrate
    cfsetospeed(&l_serialconfig, l_baudrate); //out baudrate

    l_serialconfig.c_cflag     |= (CLOCAL | CREAD);
    l_serialconfig.c_lflag     &= ~(ICANON | ECHO | ECHOE | ISIG);
    l_serialconfig.c_oflag     &= ~OPOST;

    //l_serialconfig.c_cflag = CS8|CREAD|CLOCAL; //Doesnt make handshake and enable reading from the port, 8 bits per byte enable reading
    //Logger() << "Writting (" << m_portName << ")";
    tcsetattr(m_fhandler, TCSANOW, &l_serialconfig); //Apply configurations right now
    //Logger() << "Goo (" << m_portName << ")";
    return true;
}

bool CSerialUART::portClose()
{
    if (!isOpened()) 
    {
        return true;
    }
    close(m_fhandler);    
    m_portFile = CString();
    m_portName = CString();
    return true;
}


int CSerialUART::serialWrite(const void *data, size_t size)
{
    if (!isOpened())
    {
        return -1;
    }
    if (!data || !size) {
        return 0;
    }
    int len = write(m_fhandler, data, size);
    return len;
}

int CSerialUART::serialWrite(CByteArray const &data)
{
    if (!isOpened()) {
        return -1;
    }
    if (data.isEmpty()) {
        return 0;
    }
    if(data.isString()) {
        CString msg = data.data();
        return serialWrite(msg.data(), (size_t) msg.size());
    }
    return serialWrite(data.data(), data.size());
}

int CSerialUART::serialRead(void *data, size_t size)
{
    if (!isOpened())
    {
        Logger() << "port closed on read operation";
        return -1;
    }
    size_t tot_b = bytesToRead();
    if (!tot_b) {
        return 0;
    }
    if (tot_b > size) {
        tot_b = size;
    }
    int bytesRead = read(m_fhandler,data,tot_b);
    return bytesRead;
}

size_t CSerialUART::bytesToRead()
{
    if (!isOpened()) {
        return 0;
    }
    int l_cicle = 0;
    size_t tot_b = 0;

    ioctl(m_fhandler, FIONREAD, &tot_b);
    while(tot_b == 0)
    {
        ioctl(m_fhandler, FIONREAD, &tot_b);
        l_cicle++;
        if(l_cicle >= 400)
        {
            break;
        }
    }
    return tot_b;
}
