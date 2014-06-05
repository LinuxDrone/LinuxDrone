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

#include "CSerialBus.h"

CSerial *CSerialBus::createSerial(SerialType type)
{
    switch (type) {
        case SerialType_Unknown:
            return nullptr;
        case SerialType_RT:
        case SerialType_UART:
            return new CSerialUART();
    }
    return nullptr;
}