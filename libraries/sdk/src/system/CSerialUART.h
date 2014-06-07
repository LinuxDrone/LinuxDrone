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

#include "CSerial.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <termios.h>
#include <system/Logger>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "core/CByteArray"

class CSerialUART : public CSerial
{
public:

    virtual int serialWrite(const void* data, size_t size) override;
    virtual int serialWrite(CByteArray const &data) override;
    virtual int serialRead(void *data, size_t size) override;
    virtual size_t bytesToRead() override;

    bool portOpen() override;
    bool portClose() override;        
    
private:
    int m_fhandler;
};