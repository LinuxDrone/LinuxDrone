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

#ifndef __core__BSONObjIterator__
#define __core__BSONObjIterator__

#include "BSONElement.h"

class BSONObj;

class BSONObjIterator
{
public:
    BSONObjIterator(const BSONObj& obj);
    
    bool more() const;
    BSONElement next();

private:
    const char * m_pos;
    const char * m_theEnd;
};

#endif /* defined(__core__BSONObjIterator__) */
