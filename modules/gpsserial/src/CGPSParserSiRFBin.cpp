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

#include "CGPSParserSiRFBin.h"

CGPSParserSiRFBin::CGPSParserSiRFBin()
{
	m_Longitude = CString();
	m_Latitude = CString();
	m_Altitude = CString();
	m_Speed = CString();	
	m_numberofSatellities = CString();
	m_gpsFix = CString();
	m_Time = CString();	
	m_Sentence = CString();
	m_validChecksum = false;
}
	
CGPSParserSiRFBin::~CGPSParserSiRFBin()
{

}

CGPSParserSiRFBin::CGPSParserSiRFBin(CByteArray sentence)
{
	
}

bool CGPSParserSiRFBin::parseSentence()
{
	m_validChecksum = false;
	m_payloadsize=0;	
	if(m_Sentence[0] == 0xa0)
	{
		if(m_Sentence[1] == 0xa2)
		{
	        m_payloadsize = (swaplow(m_Sentence[2],m_Sentence[3]) & 0x7FFF);
            //Logger() << "Payload:" << m_payloadsize;
            m_year = swaplow(m_Sentence[15],m_Sentence[16]);
            //Logger() << "Ano:" << m_year;
            m_seconds = ((int)swaplow(m_Sentence[21],m_Sentence[22])/1000);
            //Logger() << "Sec:" << m_seconds;
            m_lat1 =(double ) swaphigh(m_Sentence[27],m_Sentence[28],m_Sentence[29],m_Sentence[30])* 1e-07;

            int m_hour = m_Sentence[19];
            int m_min = m_Sentence[20];

            //Logger() << "Lat:" << m_lat1;
            m_lon1 =(double )  swaphigh(m_Sentence[31],m_Sentence[32],m_Sentence[33],m_Sentence[34]) * 1e-07;
            //Logger() << "Lon:" << m_lon1;
            m_alt1 =(double ) swaphigh(m_Sentence[35],m_Sentence[36],m_Sentence[37],m_Sentence[38]) * 1e-02;
            //Logger() << "Alt:" << m_alt1;
            m_fix1 = swaplow(m_Sentence[7],m_Sentence[8]);
            //Logger() << "Fix:" << m_fix1;
           	if(m_lon1 < 0)
           	{
           		m_Longitude = CString((m_lon1*-1)) + " W";
           	}
           	else
           	{
				m_Longitude = CString(m_lon1) + " E";
			}
			if(m_lat1 < 0)
			{
				m_Latitude = CString((m_lat1*-1)) + " S";
			}
			else
			{
				m_Latitude = CString(m_lat1) + " N";
			}

			m_Altitude = CString(m_alt1);
			m_Speed = CString(0);	
			m_numberofSatellities = CString((int)m_Sentence[92]);
			m_gpsFix = CString(m_fix1);	
			CString l_min = CString(m_min);					
			if(CString(l_min).size() == 1)
			{
				l_min = "0" + l_min;
			}
			CString l_sec = CString(m_seconds);
			if(l_sec.size() ==1)
			{
				l_sec = "0" + l_sec;	
			}

			m_Time = CString(m_hour) + ":" + CString(l_min) + ":" + CString(l_sec);

            m_validChecksum = checkSum();
        }
    }    
	return m_validChecksum;
}

unsigned int CGPSParserSiRFBin::swaplow(unsigned char b1, unsigned char b2)
{
  unsigned int result = (unsigned  int) b1 << 8 | b2;
  return result;
}

int CGPSParserSiRFBin::swaphigh(unsigned char b1, unsigned char b2,unsigned char b3, unsigned char b4)
{
  unsigned int t1 = swaplow(b3,b4);
  unsigned int t2 = swaplow(b1,b2);
  int result;
  t2 = t2 << 16;
  result =t2 | t1;
  return result;
}

bool CGPSParserSiRFBin::checkSum()
{
	int m_chksum=0;
	int l_chksentence = swaplow(m_Sentence[m_payloadsize+4],m_Sentence[m_payloadsize+5]) & 0x7FFF;
	for (int i=4; i<m_payloadsize+4; i++ )
	{
		m_chksum += m_Sentence[i];
	}
	m_chksum = m_chksum & 0x7FFF;
	
	if(l_chksentence==m_chksum)
	{
		return true;
	}
	else
	{
		return false;
	}
}	