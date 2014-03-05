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

#include "CAbstractSerial.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <termios.h>

#include <sys/types.h>
#include <sys/stat.h>

class CImplSerialRegular : public CAbstractSerial
{
public:
    
    int serial_write(const void* data, size_t size);
    int serial_read(void* data, size_t size);
        
    bool portOpen();
    bool portClose();        
    
    private:
        int m_fhandler;
};
