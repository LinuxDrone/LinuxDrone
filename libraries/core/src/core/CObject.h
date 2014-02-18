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

#ifndef __core__cobject__
#define __core__cobject__

#include <stdint.h>
#include <vector>
#include "../thread/CMutex.h"

class CObject
{
public:
    CObject();
    virtual ~CObject();

	virtual uint32_t addRef();
	virtual uint32_t release();
	uint32_t refs();

    virtual void onUpdate(float timeElapsed);

protected:
    uint32_t m_refs;
    CMutex   m_mutexRefs;

    virtual void selfDelete();
};

#define SAFE_RELEASE(x) if(x) {x->release();x=0;}
#define SAFE_DELETE(x) if (x) {delete x;x=0;}
#endif /* defined(__core__cobject__) */
