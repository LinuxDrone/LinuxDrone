//--------------------------------------------------------------------
// This file was created as a part of the LinuxDro*ne project:
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
#include "CSerialUART.h"

class CSerialBus
{
public:
    enum SerialType
    {
        SerialType_UART,
        SerialType_RT,
        SerialType_Unknown,
    };
    
    static CSerial* createSerial(SerialType type)
    {
        if(type == SerialType_RT)
        {
            return new CSerialUART;
        }
        else
        {
            return new CSerialUART;
        }
    }
};