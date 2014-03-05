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

#include "text/CString"

class CNMEAParser  
{
public:
	CNMEAParser();
        ~CNMEAParser();
        bool parseNMEA(CString sentence);
        
private:
    long m_latitude;
    long m_longitude;
    long m_altitude;
    CString m_time;
    long m_fix;
    int m_satellites;
    long m_qualitysignal;

    int calc_checksum(CString sentence);
};
        