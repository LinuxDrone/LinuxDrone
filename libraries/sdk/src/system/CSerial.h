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

class CSerial
{
public:
	CSerial();        
	~CSerial();
    	
	virtual int serial_write(CString &ata) = 0;
	virtual int serial_read(CString &data, size_t size) = 0;
        
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
    CString m_portName = "";
    int m_portSpeed = 0;    
    CString m_portFile = "";
    bool m_Opened = false;    
};
