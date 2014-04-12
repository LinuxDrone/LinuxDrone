//--------------------------------------------------------------------
// This file was created as a part of the LinuxDrone project:
//                http://www.linuxdroneBin
//
// Distributed under the Creative Commons Attribution-ShareAlike 4.0
// International License (see accompanying License.txt file or a copy
// at http://creativecommons.org/licenses/by-sa/4.0/legalcode)
//
// The human-readable summary of (and not a substitute for) the
// license: http://creativecommons.org/licenses/by-sa/4.0/
//--------------------------------------------------------------------

#pragma once
#include "core/CByteArray"
#include <vector>
#include "CGPSProtocol.h"
#include "system/Logger"

class CGPSProtocolBinSirf : public CGPSProtocol
{
public:
	CGPSProtocolBinSirf();	
	~CGPSProtocolBinSirf();
	
	std::vector<CByteArray> getInitCommands() override;
	CByteArray	getQueryCommand() override;
};