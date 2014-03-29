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

#include "CGPSProtocolNMEAuBlox.h"


CGPSProtocolNMEAuBlox::CGPSProtocolNMEAuBlox()
{

}
CGPSProtocolNMEAuBlox::~CGPSProtocolNMEAuBlox()
{

}

std::vector<CString> CGPSProtocolNMEAuBlox::getInitCommands()
{
	std::vector<CString> l_retcommands;
	//l_retcommands.push_back("$PSRF100,1,####,8,1,0*05\r\n"); //return NMEA Protocol - #### will be replaced with current velocity
	l_retcommands.push_back("$PUBX,40,GGA,0,0,0,0,0,0*5A\r\n"); //Off GGA
	l_retcommands.push_back("$PUBX,40,GLL,0,0,0,0,0,0*5C\r\n"); //Off GLL
	l_retcommands.push_back("$PUBX,40,GSA,0,0,0,0,0,0*4E\r\n"); //Off GSA
	l_retcommands.push_back("$PUBX,40,GSV,0,0,0,0,0,0*59\r\n"); //Off GSV
	l_retcommands.push_back("$PUBX,40,RMC,0,0,0,0,0,0*47\r\n"); //Off RMC
	l_retcommands.push_back("$PUBX,40,VTG,0,0,0,0,0,0*5E\r\n"); //Off VTG
	l_retcommands.push_back("$PUBX,40,GRS,0,0,0,0,0,0*5D\r\n"); //Off GRS
	l_retcommands.push_back("$PUBX,40,GST,0,0,0,0,0,0*5B\r\n"); //Off GST
	l_retcommands.push_back("$PUBX,40,ZDA,0,0,0,0,0,0*44\r\n"); //Off ZDA
	l_retcommands.push_back("$PUBX,40,GPQ,0,0,0,0,0,0*5D\r\n"); //Off GPQ
	l_retcommands.push_back("$PUBX,40,GBS,0,0,0,0,0,0*4D\r\n"); //Off GBS
	l_retcommands.push_back("$PUBX,40,DTM,0,0,0,0,0,0*46\r\n"); //Off DTM
	l_retcommands.push_back("$PUBX,40,TXT,0,0,0,0,0,0*43\r\n"); //Off TXT
	return l_retcommands;
}

CString	CGPSProtocolNMEAuBlox::getQueryCommand()
{
	return "$EIGPQ,GGA*27\r\n";
}

int CGPSProtocolNMEAuBlox::getEndSentence()
{
	return 10;
}