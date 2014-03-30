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
	m_ProtocolType = "A";
}

CGPSProtocolNMEASirf::~CGPSProtocolNMEASirf()
{

}

std::vector<CByteArray> CGPSProtocolNMEASirf::getInitCommands()
{
	std::vector<CByteArray> l_retcommands;
	CString l_cmd;
	//l_retcommands.push_back("$PSRF100,1,####,8,1,0*05\r\n"); //return NMEA Protocol - #### will be replaced with current velocity
	//l_retcommands.push_back("$PSRF103,00,00,00,01*24\r\n"); //Off GGA
	l_cmd = "$PSRF103,00,00,00,01*24\r\n";
	l_retcommands.push_back(CByteArray(l_cmd.data(),l_cmd.size(),false));

	//l_retcommands.push_back("$PSRF103,01,00,00,01*25\r\n"); //Off GGL
	l_cmd = "$PSRF103,01,00,00,01*25\r\n";
	l_retcommands.push_back(CByteArray(l_cmd.data(),l_cmd.size(),false));	

	//l_retcommands.push_back("$PSRF103,02,00,00,01*26\r\n"); //Off GSA
	l_cmd = "$PSRF103,02,00,00,01*26\r\n";
	l_retcommands.push_back(CByteArray(l_cmd.data(),l_cmd.size(),false));

	//l_retcommands.push_back("$PSRF103,03,00,00,01*27\r\n"); //Off GSV
	l_cmd = "$PSRF103,03,00,00,01*27\r\n";
	l_retcommands.push_back(CByteArray(l_cmd.data(),l_cmd.size(),false));

	//l_retcommands.push_back("$PSRF103,04,00,00,01*20\r\n"); //Off RMC
	l_cmd = "$PSRF103,04,00,00,01*20\r\n";
	l_retcommands.push_back(CByteArray(l_cmd.data(),l_cmd.size(),false));

	//l_retcommands.push_back("$PSRF103,05,00,00,01*21\r\n"); //Off VTG
	l_cmd = "$PSRF103,05,00,00,01*21\r\n";
	l_retcommands.push_back(CByteArray(l_cmd.data(),l_cmd.size(),false));

	//l_retcommands.push_back("$PSRF103,06,00,00,01*22\r\n"); //Off MSS
	l_cmd = "$PSRF103,06,00,00,01*22\r\n";
	l_retcommands.push_back(CByteArray(l_cmd.data(),l_cmd.size(),false));

	//l_retcommands.push_back("$PSRF103,08,00,00,01*2C\r\n"); //Off ZDA
	l_cmd = "$PSRF103,08,00,00,01*2C\r\n";
	l_retcommands.push_back(CByteArray(l_cmd.data(),l_cmd.size(),false));

	return l_retcommands; 

}

CByteArray	CGPSProtocolNMEASirf::getQueryCommand()
{
	CString ltmp = "$PSRF103,00,01,00,01*25\r\n";
	CByteArray l_cmd = CByteArray(ltmp.data(),ltmp.size(),false);
	return l_cmd; // returning query gga
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