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

class CGPSParser
{
public:
	CString	getLongitude();
	CString	getLongitudeDir();
	CString	getLatitude();
	CString	getLatitudeDir();
	CString	getAltitude();
	CString	getSpeed();
	CString	getnumberofSatellities();
	CString	getGpsFix();
	CString	getTime();

	void	setSentence(CString sentence);
	bool	getValidParser();

	virtual bool parseSentence() = 0;
	virtual CString getFullSentence(CString newsentence) = 0;

protected:	
	
	bool m_validChecksum;
	CString	m_Longitude;
	CString	m_LongitudeDir;
	CString m_Latitude;
	CString m_LatitudeDir;
	CString	m_Altitude;
	CString	m_Speed;	
	CString m_numberofSatellities;
	CString	m_gpsFix;
	CString m_Time;	
	CString	m_Sentence;
};