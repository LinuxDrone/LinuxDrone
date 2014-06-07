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

#include "CGPSParser.h"
#include "CGPSParserNMEA.h"
#include "CGPSParserSiRFBin.h"

class CGPSParserFactory
{
public:
	//"validValues" : ["NMEA","NMEA SiRF","NMEA uBlox","Binary SiRF","Binary uBlox"],
   	static CGPSParser* createGPSParser(CString parserType)
    {
        if(parserType == "NMEA SiRF")
        {
            return new CGPSParserNMEA;
        }
        else if(parserType == "NMEA uBlox")
        {
            return new CGPSParserNMEA;
        }
        else if(parserType == "Binary SiRF")
        {
            return new CGPSParserSiRFBin;
        }
    }
};