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
#include <vector>
#include "CGPSParser.h"

class CGPSProtocol
{
public:
	virtual std::vector<CString> getInitCommands() = 0;
	virtual CString	getQueryCommand() = 0;
	virtual int getEndSentence() = 0;	
};