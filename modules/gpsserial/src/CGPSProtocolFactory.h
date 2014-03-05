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

#include "text/CString"
#include "CGPSProtocolNMEASirf.h"
#include "CGPSProtocolNMEAuBlox.h"


class CGPSProtocolFactory
{
public:
	//"validValues" : ["NMEA","NMEA SiRF","NMEA uBlox","Binary SiRF","Binary uBlox"],
   	static CGPSProtocol* createGPSProtocol(CString protocolType)
    {
        if(protocolType == "NMEA SiRF")
        {
            return new CGPSProtocolNMEASirf;
        }
        else if(protocolType == "NMEA uBlox")
        {
            return new CGPSProtocolNMEAuBlox;
        }
    }
};