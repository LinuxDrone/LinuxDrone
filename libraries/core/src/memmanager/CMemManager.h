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

#ifndef __CMEM_MANAGER_H__
#define __CMEM_MANAGER_H__

#include <stddef.h>
#include "../libCoreGlobal.h"

#ifdef XENO_SERVICES
#include <native/heap.h>
#endif

class CMemManager
{
public:
	CMemManager();
	~CMemManager();

	static CMemManager* instance();

	void* alloc(size_t size);
	void* realloc(void* ptr, size_t size);
	void free(void* ptr);

private:
#ifdef XENO_SERVICES
	RT_HEAP m_heap;
#else
#endif
};

#endif // __CMEM_MANAGER_H__
