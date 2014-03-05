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

#pragma once

#include "text/CString"
#include <stddef.h>
#include <stdint.h>
#include "system/Logger"

class CAbstractSerial
{
public:
	CAbstractSerial();	
        CAbstractSerial(CString portFile, int portSpeed,CString portName);
	~CAbstractSerial();
    	
	virtual int serial_write(const void* data, size_t size) = 0;
	virtual int serial_read(void* data, size_t size) = 0;
        
        CString getPortName();
        int getPortSpeed();
        CString getPortFile();
                
        bool setPortFile(CString portFile);
        bool setPortSpeed(int portSpeed);
        bool setPortName(CString portName);
        
        bool isOpened();
        
        virtual bool portOpen() = 0;
        virtual bool portClose() = 0;

protected:
    CString m_portName;
    int m_portSpeed;    
    CString m_portFile;
    bool m_Opened;    
};
