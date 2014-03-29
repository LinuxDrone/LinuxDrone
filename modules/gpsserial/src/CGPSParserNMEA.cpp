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

#include "CGPSParserNMEA.h"

CGPSParserNMEA::CGPSParserNMEA()
{
	m_Longitude = CString();
	m_Latitude = CString();
	m_Altitude = CString();
	m_Speed = CString();	
	m_numberofSatellities = CString();
	m_gpsFix = CString();
	m_Time = CString();	
	m_Sentence = CString();
}
	
CGPSParserNMEA::~CGPSParserNMEA()
{

}

bool CGPSParserNMEA::parseSentence()
{
	int l_tmpposicion=0;
	CString l_tmp = CString();
	char l_chksum;
	int l_idxNextString = 0;
	int l_sentenceSize = 0;

	m_Longitude = CString();
	m_Latitude = CString();
	m_Altitude = CString();
	m_Speed = CString();	
	m_numberofSatellities = CString();
	m_gpsFix = CString();
	m_Time = CString();	
	
	l_idxNextString = m_Sentence.find(",",0);	
	CString	l_seqID = m_Sentence.left(l_idxNextString);
	m_validChecksum = false;

	int l_commas =0;
	for(int i=0;i< m_Sentence.size();i++)
	{
		if(m_Sentence[i] == ',') 
		{
			l_commas++;
		}
	}
	if((l_seqID == "$GPGGA") && (l_commas == 14))
	{		
		l_sentenceSize++;
		//$GPGGA,223715.506,,,,,0,00,,,M,0.0,M,,0000*55
		//Getting time
		l_tmpposicion = l_idxNextString+1;
		l_idxNextString = m_Sentence.find(",",l_tmpposicion);
		m_Time = m_Sentence.substr(l_tmpposicion,l_idxNextString);
		l_sentenceSize++;
		l_tmpposicion = l_idxNextString+1;
		if(m_Sentence[l_tmpposicion] == ',')
		{
			l_idxNextString++;
			m_Latitude="0";
		}
		else
		{
			l_idxNextString = m_Sentence.find(",",l_tmpposicion);
			m_Latitude = m_Sentence.substr(l_tmpposicion,l_idxNextString);			
		}

		l_sentenceSize++;
		l_tmpposicion = l_idxNextString+1;
		if(m_Sentence[l_tmpposicion] == ',')
		{
			l_idxNextString++;
			m_LatitudeDir=" ";
		}
		else
		{
			l_idxNextString = m_Sentence.find(",",l_tmpposicion);
			m_LatitudeDir = m_Sentence.substr(l_tmpposicion,l_idxNextString);			
		}

		l_sentenceSize++;
		l_tmpposicion = l_idxNextString+1;
		if(m_Sentence[l_tmpposicion] == ',')
		{
			l_idxNextString++;
			m_Longitude="0";
		}
		else
		{
			l_idxNextString = m_Sentence.find(",",l_tmpposicion);
			m_Longitude = m_Sentence.substr(l_tmpposicion,l_idxNextString);			
		}
		l_sentenceSize++;
		l_tmpposicion = l_idxNextString+1;
		if(m_Sentence[l_tmpposicion] == ',')
		{
			l_idxNextString++;
			m_LongitudeDir=" ";
		}
		else
		{
			l_idxNextString = m_Sentence.find(",",l_tmpposicion);
			m_LongitudeDir = m_Sentence.substr(l_tmpposicion,l_idxNextString);			
		}

		//Getting GPS Fix
		l_sentenceSize++;
		l_tmpposicion = l_idxNextString+1;
		if(m_Sentence[l_tmpposicion] == ',')
		{
			l_idxNextString++;
			m_gpsFix="0";
		}
		else
		{
			l_idxNextString = m_Sentence.find(",",l_tmpposicion);
			m_gpsFix = m_Sentence.substr(l_tmpposicion,l_idxNextString);			
		}
		//Getting Number of statellites
		l_sentenceSize++;
		l_tmpposicion = l_idxNextString+1;
		if(m_Sentence[l_tmpposicion] == ',')
		{
			l_idxNextString++;
			m_numberofSatellities="0";
		}
		else
		{
			l_idxNextString = m_Sentence.find(",",l_tmpposicion);
			m_numberofSatellities = m_Sentence.substr(l_tmpposicion,l_idxNextString);			
		}
		//Getting HDOP - ignored field
		l_sentenceSize++;
		l_tmpposicion = l_idxNextString+1;
		if(m_Sentence[l_tmpposicion] == ',')
		{
			l_idxNextString++;
		}
		else
		{
			l_idxNextString = m_Sentence.find(",",l_tmpposicion);
		}

		//Getting Altitude
		l_sentenceSize++;
		l_tmpposicion = l_idxNextString+1;
		if(m_Sentence[l_tmpposicion] == ',')
		{
			l_idxNextString++;
			m_Altitude="0";
		}
		else
		{
			l_idxNextString = m_Sentence.find(",",l_tmpposicion);
			m_Altitude = m_Sentence.substr(l_tmpposicion,l_idxNextString);			
		}
		//Checksum
		l_idxNextString = m_Sentence.find("*",l_tmpposicion);
		if(l_idxNextString != 0)
		{
			l_tmp = m_Sentence.substr(l_idxNextString+1);
			if(validade_checksum(m_Sentence))
			{
				m_validChecksum = true;
				//Logger() << "=========================================> Checksum valido <================================================";
			}
			else
			{
				//Logger() << "*****************************************> Checksum Invalido <************************************************";		
				//Logger() << "Retornando false";
			}

		}

		
		//Logger() << "| Time:" << m_Time << " | " << "Lat:" << m_Latitude << " | " << "Lat Dir:" << m_LatitudeDir << " | " << "Long:" << m_Longitude << " | " << "Long Dir:" << m_LongitudeDir << " | " << "Fix:" << m_gpsFix << " | " << "Sat:" << m_numberofSatellities << " | " << "Alt:" << m_Altitude << " | ";
		/*Logger() << "Lat:" << m_Latitude;
		Logger() << "Lat Dir:" << m_LatitudeDir;
		Logger() << "Long:" << m_Longitude;
		Logger() << "Long Dir:" << m_LongitudeDir;
		Logger() << "Fix:" << m_gpsFix;
		Logger() << "Sat:" << m_numberofSatellities;
		Logger() << "Alt:" << m_Altitude;
		Logger() << "Chk:" << l_tmp; */
	}
	//Logger() << "REtorno final parser :" << m_validChecksum;
	return m_validChecksum;
}
bool CGPSParserNMEA::validade_checksum(CString sentence)
{
	char *l_sentence_ref = (char *)sentence.data();
	unsigned int l_interator=0;
	char l_character;
	int l_calcChecksum=0;
	int l_getChecksum=0;

	if (l_sentence_ref[0] != '$' ) 
	{ 
		return false; 
	}
	l_character = l_sentence_ref[1];
	l_calcChecksum = l_character;
	l_interator = 2;
	while(l_character != '*')
   	{
   		l_character = l_sentence_ref[l_interator]; // get next chr
   		if(l_character != '*')
   		{ 
   			l_calcChecksum = l_calcChecksum ^ l_character; 
   		}
   		l_interator++;
   	}
	
	l_getChecksum = 16*hex2dec(l_sentence_ref[l_interator]) + hex2dec(l_sentence_ref[l_interator+1]);

	if(l_calcChecksum != l_getChecksum)
	{
		return false;
	}
	return true;
}

int CGPSParserNMEA::hex2dec(char hexdigit) 
{
	if (int(hexdigit) >= 65)
	{ 
		return int(hexdigit) - 55;
	}
	else 
	{
		return int(hexdigit) - 48;
	}
}

CString CGPSParserNMEA::getFullSentence(const CString newsentence)
{
	int l_checksum=0;
	char *l_tmpfinalsentence = (char *)newsentence.data();
	char l_charref;

	CString l_finalsentence;
	
	for (int i = 0; i < newsentence.length(); i++) 
	{		
		l_charref= l_tmpfinalsentence[i];
		Logger() << "CHAR : " << l_charref;  			
		Logger() << "CHKREF : " << l_checksum;  			
    	l_checksum ^= l_charref;
  	}	 
	Logger() << "CHECKSUM : " << l_checksum;  	
	
  	l_finalsentence = "$" + newsentence + "*" + l_checksum;

  	return l_finalsentence;
}