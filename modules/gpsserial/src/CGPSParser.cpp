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
#include "CGPSParser.h"


CString CGPSParser::getLongitude()
{
	return m_Longitude;
}

CString	CGPSParser::getLongitudeDir()
{
	return m_LongitudeDir;
}

CString	CGPSParser::getLatitude()
{
	return m_Latitude;
}

CString	CGPSParser::getLatitudeDir()
{
	return m_LatitudeDir;
}

CString	CGPSParser::getAltitude()
{
	return m_Altitude;
}

CString	CGPSParser::getSpeed()
{
	return m_Speed;
}

CString	CGPSParser::getnumberofSatellities()
{
	return m_numberofSatellities;
}

CString	CGPSParser::getGpsFix()
{
	return m_gpsFix;
}

CString	CGPSParser::getTime()
{
	return m_Time;
}

void CGPSParser::setSentence(CString sentence)
{
	m_Sentence = sentence;
}

bool CGPSParser::getValidParser()
{
	return m_validChecksum;
}
