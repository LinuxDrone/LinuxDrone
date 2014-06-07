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
	m_ProtocolType = "A";
}
CGPSProtocolNMEAuBlox::~CGPSProtocolNMEAuBlox()
{

}

std::vector<CByteArray> CGPSProtocolNMEAuBlox::getInitCommands()
{
	std::vector<CByteArray> l_retcommands;
	CString l_cmd;

	//l_retcommands.push_back("$PUBX,40,GGA,0,0,0,0,0,0*5A\r\n"); //Off GGA
	l_cmd = "$PUBX,40,GGA,0,0,0,0,0,0*5A\r\n";
	l_retcommands.push_back(CByteArray(l_cmd.data(),l_cmd.size(),false));

	//l_retcommands.push_back("$PUBX,40,GLL,0,0,0,0,0,0*5C\r\n"); //Off GLL
	l_cmd = "$PUBX,40,GLL,0,0,0,0,0,0*5C\r\n";
	l_retcommands.push_back(CByteArray(l_cmd.data(),l_cmd.size(),false));

	//l_retcommands.push_back("$PUBX,40,GSA,0,0,0,0,0,0*4E\r\n"); //Off GSA
	l_cmd = "$PUBX,40,GSA,0,0,0,0,0,0*4E\r\n";
	l_retcommands.push_back(CByteArray(l_cmd.data(),l_cmd.size(),false));

	//l_retcommands.push_back("$PUBX,40,GSV,0,0,0,0,0,0*59\r\n"); //Off GSV
	l_cmd = "$PUBX,40,GSV,0,0,0,0,0,0*59\r\n";
	l_retcommands.push_back(CByteArray(l_cmd.data(),l_cmd.size(),false));

	//l_retcommands.push_back("$PUBX,40,RMC,0,0,0,0,0,0*47\r\n"); //Off RMC
	l_cmd = "$PUBX,40,RMC,0,0,0,0,0,0*47\r\n";
	l_retcommands.push_back(CByteArray(l_cmd.data(),l_cmd.size(),false));

	//l_retcommands.push_back("$PUBX,40,VTG,0,0,0,0,0,0*5E\r\n"); //Off VTG
	l_cmd = "$PUBX,40,VTG,0,0,0,0,0,0*5E\r\n";
	l_retcommands.push_back(CByteArray(l_cmd.data(),l_cmd.size(),false));

	//l_retcommands.push_back("$PUBX,40,GRS,0,0,0,0,0,0*5D\r\n"); //Off GRS
	l_cmd = "$PUBX,40,GRS,0,0,0,0,0,0*5D\r\n";
	l_retcommands.push_back(CByteArray(l_cmd.data(),l_cmd.size(),false));

	//l_retcommands.push_back("$PUBX,40,GST,0,0,0,0,0,0*5B\r\n"); //Off GST
	l_cmd = "$PUBX,40,GST,0,0,0,0,0,0*5B\r\n";
	l_retcommands.push_back(CByteArray(l_cmd.data(),l_cmd.size(),false));

	//l_retcommands.push_back("$PUBX,40,ZDA,0,0,0,0,0,0*44\r\n"); //Off ZDA
	l_cmd = "$PUBX,40,ZDA,0,0,0,0,0,0*44\r\n";
	l_retcommands.push_back(CByteArray(l_cmd.data(),l_cmd.size(),false));

	//l_retcommands.push_back("$PUBX,40,GPQ,0,0,0,0,0,0*5D\r\n"); //Off GPQ
	l_cmd = "$PUBX,40,GPQ,0,0,0,0,0,0*5D\r\n";
	l_retcommands.push_back(CByteArray(l_cmd.data(),l_cmd.size(),false));

	//l_retcommands.push_back("$PUBX,40,GBS,0,0,0,0,0,0*4D\r\n"); //Off GBS
	l_cmd = "$PUBX,40,GBS,0,0,0,0,0,0*4D\r\n";
	l_retcommands.push_back(CByteArray(l_cmd.data(),l_cmd.size(),false));

	//l_retcommands.push_back("$PUBX,40,DTM,0,0,0,0,0,0*46\r\n"); //Off DTM
	l_cmd = "$PUBX,40,DTM,0,0,0,0,0,0*46\r\n";
	l_retcommands.push_back(CByteArray(l_cmd.data(),l_cmd.size(),false));

	//l_retcommands.push_back("$PUBX,40,TXT,0,0,0,0,0,0*43\r\n"); //Off TXT
	l_cmd = "$PUBX,40,TXT,0,0,0,0,0,0*43\r\n";
	l_retcommands.push_back(CByteArray(l_cmd.data(),l_cmd.size(),false));

	return l_retcommands;
}

CByteArray	CGPSProtocolNMEAuBlox::getQueryCommand()
{
	CString ltmp = "$EIGPQ,GGA*27\r\n";
	CByteArray l_cmd = CByteArray(ltmp.data(),ltmp.size(),false);
	return l_cmd;
}