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

#include "CSerialUART.h"
      
bool CSerialUART::portOpen()
{    
    if (isOpened()) 
    {
    	m_Opened = false;
        return false;
    }

    struct termios l_serialconfig; // port configuration - baud rate, parity, etc
    long l_baudrate; // will receive the baudrate
    
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
}
    
int CSerialUART::serialWrite(CByteArray const &data)
{
    if (!isOpened()) 
    {
        return -1;
    }
    int len=0;
    //Logger() << "Size " << data.size();
    if(data.isString())
    {
        CString msg = data.data();
        len = write(m_fhandler, msg.data(), msg.size());
    }
    else
    {    
        len = write(m_fhandler, data.data(), data.size());
    }
    return len;
}

int CSerialUART::serialRead(void *data, size_t size)
{
    int tot_b = 0;
    int bytesRead=0;
    int l_cicle = 0;

    if (!isOpened()) 
    {
    	Logger() << "port closed on read operation";
        return -1;
    }
    ioctl(m_fhandler, FIONREAD, &tot_b);
    while(tot_b == 0)
    {
        ioctl(m_fhandler, FIONREAD, &tot_b);
        l_cicle++;
        //Logger() << "Ciclo:" << l_cicle;
        if(l_cicle >= 400)
        {
            break;
        }
    }    
    bytesRead = read(m_fhandler,data,tot_b);
    return bytesRead;
}