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

#include "Logger.h"
#include "../core/CObject.h"
#include "../core/CByteArray.h"
//#include "../text/cstringlist.h"

#include "../memmanager/my_memory.h"

Logger::Logger()
{
	m_man = DEC;
}

Logger::~Logger()
{
	printf("%s\n", m_text.data());
}

Logger& Logger::operator<<(const char* str)
{
    m_text << " " << str;
    return *this;
}

Logger& Logger::operator<<(const CString& str)
{
    m_text << " " << str;
    return *this;
}

Logger& Logger::operator<<(const CStringAnsi& str) {
    m_text << " " << str.data();
    return *this;
}

Logger& Logger::operator<<(const CByteArray& data)
{
    if (data.isEmpty())
        return *this;
    std::ostringstream stream;
    stream << std::hex;
    unsigned char* ptr = (unsigned char*)data.data();
    for( int i = 0;i<int(data.size());i++ )
    {
        if( *ptr == 0 )
            stream << "00 ";
        else if( *ptr <= 0x0f )
            stream << "0" << std::hex << (int)*ptr << " ";
        else
            stream << std::hex << (int)*ptr << " ";
        ptr++;
    }
    m_text << stream.str().c_str();
    return *this;
}

//Logger& Logger::operator<<(const CStringList& list)
//{
//    CStringList::const_iterator it;
//    int i = 0, size = int(list.size());
//    for( it = list.begin();it!=list.end();it++ )
//    {
//        i++;
//        m_text << (*it);
//        if( i != size )
//            m_text << "\r\n";
//    }
//    return *this;
//}
