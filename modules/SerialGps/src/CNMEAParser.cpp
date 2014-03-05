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

#include "CNMEAParser.h"

CNMEAParser::CNMEAParser()
{

}
CNMEAParser::~CNMEAParser()
{

}
bool CNMEAParser::parseNMEA(CString sentence)
{
    return true;
}

int CNMEAParser::calc_checksum(CString sentence)
{
    int l_count = 0;
    int l_sentenceposition=0;
    int l_digitanalisysposition = 0;
    int l_checkpartial[5];
    int l_curdigit =0 ;
    while (l_sentenceposition <=3 )
    {
        if(sentence[l_sentenceposition] == '\0')
        {
            break;
        }
        else if ((sentence[l_sentenceposition] >='A' && sentence[l_sentenceposition] <= 'F') || (sentence[l_sentenceposition] >='a' && sentence[l_sentenceposition] <= 'f'))
        {
            l_checkpartial[l_sentenceposition] = (sentence[l_sentenceposition] & 0x0f) + 9;
        }
        else if (sentence[l_sentenceposition] > 0x29 && sentence[l_sentenceposition] < 0x40 )
        {
            l_checkpartial[l_sentenceposition] = sentence[l_sentenceposition] & 0x0f;
        }
        else
        {
            break;
        }
        l_sentenceposition++;
    }
    l_count = l_sentenceposition;    
    l_digitanalisysposition = l_sentenceposition -1;

    for(l_sentenceposition = 0; l_sentenceposition< l_count; l_sentenceposition++)
    {
        l_curdigit = l_curdigit | (l_checkpartial[l_sentenceposition] << (l_digitanalisysposition << 2));
        l_digitanalisysposition--;
    }
    return l_curdigit;        
}
