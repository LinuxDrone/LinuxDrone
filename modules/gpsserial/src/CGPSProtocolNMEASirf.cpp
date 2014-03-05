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

#include "CGPSProtocolNMEASirf.h"


CGPSProtocolNMEASirf::CGPSProtocolNMEASirf()
{

}
CGPSProtocolNMEASirf::~CGPSProtocolNMEASirf()
{

}

std::vector<CString> CGPSProtocolNMEASirf::getInitCommands()
{
	std::vector<CString> l_retcommands;
	//l_retcommands.push_back("$PSRF100,1,####,8,1,0*05\r\n"); //return NMEA Protocol - #### will be replaced with current velocity
	l_retcommands.push_back("$PSRF103,00,00,00,01*24\r\n"); //Off GGA
	l_retcommands.push_back("$PSRF103,01,00,00,01*25\r\n"); //Off GGL
	l_retcommands.push_back("$PSRF103,02,00,00,01*26\r\n"); //Off GSA
	l_retcommands.push_back("$PSRF103,03,00,00,01*27\r\n"); //Off GSV
	l_retcommands.push_back("$PSRF103,04,00,00,01*20\r\n"); //Off RMC
	l_retcommands.push_back("$PSRF103,05,00,00,01*21\r\n"); //Off VTG
	l_retcommands.push_back("$PSRF103,06,00,00,01*22\r\n"); //Off MSS
	l_retcommands.push_back("$PSRF103,08,00,00,01*2C\r\n"); //Off ZDA
	return l_retcommands;
}

CString	CGPSProtocolNMEASirf::getQueryCommand()
{
	return "$PSRF103,00,01,00,01*25\r\n"; // returning query gga
}

int CGPSProtocolNMEASirf::getEndSentence()
{
	return 10;
}

/*
// 115200
$PSRF100,1,115200,8,1,0*05

//Desligando mensagens
//GGA
$PSRF103,00,00,00,01*24
//GGL
$PSRF103,01,00,00,01*25
//GSA
$PSRF103,02,00,00,01*26
//GSV
$PSRF103,03,00,00,01*27
//RMC
$PSRF103,04,00,00,01*20
//VTG
$PSRF103,05,00,00,01*21
//MSS
$PSRF103,06,00,00,01*22
//ZDA
$PSRF103,08,00,00,01*2C

//Query GGA
$PSRF103,00,01,00,01*25
*/