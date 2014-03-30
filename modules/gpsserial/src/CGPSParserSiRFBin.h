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
#include "system/Logger"
#include "CGPSParser.h"
#include "core/CByteArray"
#include <string.h>

class CGPSParserSiRFBin : public CGPSParser
{
	
public:

	CGPSParserSiRFBin();        
	CGPSParserSiRFBin(CByteArray sentence);        
	~CGPSParserSiRFBin();

	bool parseSentence() override;

private:	
	int m_payloadsize=0;	
	bool m_validChecksum; //If checksum is valid	
	int m_year=0;
	int m_seconds=0;
	double m_lat1=0;
	double m_lon1=0;
	double m_alt1=0;
	int m_fix1=0;
	int m_sat=0;
	
	bool checkSum();
	unsigned int swaplow(unsigned char b1, unsigned char b2);
	int swaphigh(unsigned char b1, unsigned char b2,unsigned char b3, unsigned char b4);
};