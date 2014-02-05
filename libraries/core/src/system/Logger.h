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

#ifndef __core__logger__
#define __core__logger__

#include "../text/CString.h"
#include <fstream>
#include <string>
#include <sstream>

class CStringList;
class CVariant;

enum manipulator {
    HEX,
    DEC
};

class Logger
{
public:
	Logger();
	~Logger();

    Logger& operator<<(const char* str);
    Logger& operator<<(const CString& str);
    Logger& operator<<(const CByteArray& data);
//    Logger& operator<<(const CStringList& list);
    template<typename T>
    Logger& operator<<(const T& val) {
        if (HEX == m_man)
        {
            std::ostringstream stream;
            stream << std::hex << val;
            m_text << stream.str().c_str();
        }
        else
            m_text << " " << CString(val);
        return *this;
    }

private:
    CString            m_text;
    manipulator        m_man;
};

#endif /* defined(__core__logger__) */
