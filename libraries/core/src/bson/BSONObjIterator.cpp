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

#include "BSONObjIterator.h"
#include "BSONObj.h"

BSONObjIterator::BSONObjIterator(const BSONObj& obj)
{
    if (obj.isEmpty()) {
        m_pos = 0;
        m_theEnd = 0;
        return;
    }
    CByteArray data = obj.data();
    m_pos = data.data()+4;
    m_theEnd = data.data()+data.size()-1;
}

bool BSONObjIterator::more() const
{
    return m_pos < m_theEnd;
}

BSONElement BSONObjIterator::next()
{
    BSONElement elem(m_pos);
    m_pos += elem.size();
    return elem;
}
