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
#include "CSerialFactory.h"
#include "CAbstractSerial.h"
#include "CImplSerialRegular.h"
#include "CSerialFactory.h"

class CSerialBus : public CSerialFactory
{
public:
    CAbstractSerial *createSerial(CSerialBus::SerialType type )
    {
        if(type == CSerialFactory::SerialType_RTDM)
        {
            return new CImplSerialRegular;
        }
        else
        {
            return new CImplSerialRegular;
        }
    }
    //CAbstractSerial *createSerial(CString *serialType);    
};