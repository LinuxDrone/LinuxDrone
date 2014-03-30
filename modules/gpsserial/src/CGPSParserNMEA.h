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


class CGPSParserNMEA : public CGPSParser
{
	
public:

	CGPSParserNMEA();        
	CGPSParserNMEA(CString sentence);        
	~CGPSParserNMEA();

	bool parseSentence() override;

private:	
	
	bool m_validChecksum; //If checksum is valid	
	bool validade_checksum(CString sentence);
	int hex2dec(char hexdigit); 
	CString l_sentence;
};