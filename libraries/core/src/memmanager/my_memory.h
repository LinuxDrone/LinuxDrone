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

#ifndef __MY_MEMORY_H__
#define __MY_MEMORY_H__

#include "CMemManager.h"
#ifndef __APPLE__
#include <malloc.h>
#else
#include <stdlib.h>
#endif

#define my_malloc(len) my_malloc_dbg(len, __FILE__, __FUNCTION__, __LINE__)
#define my_realloc(ptr,len) my_realloc_dbg(ptr, len, __FILE__, __FUNCTION__, __LINE__)
#define my_free(ptr) my_free_dbg(ptr,__FILE__, __FUNCTION__, __LINE__)

void* my_malloc_dbg(size_t len, const char* file, const char* func, long line);
void my_free_dbg(void* ptr, const char* file, const char* func, long line);

#ifdef new
#undef new
#endif

#ifdef delete
#undef delete
#endif

inline void* operator new( size_t s )
{
	return malloc(s);
//	return my_malloc_dbg( s, __FILE__, __FUNCTION__, __LINE__ );
}

inline void operator delete(void* p)
{
	free(p);
//	my_free(p);
}

inline void* my_malloc_dbg(size_t len, const char* file, const char* func, long line )
{
	return malloc(len);
	CMemManager* mm = CMemManager::instance();
	if (!mm) {
		return 0;
	}
	return mm->alloc(len/*, file, func, line*/);
}

inline void* my_realloc_dbg(void* ptr, size_t len, const char* file, const char* func, long line)
{
	return realloc(ptr, len);
	CMemManager* mm = CMemManager::instance();
	if (!mm) {
		return 0;
	}
	return mm->realloc(ptr, len/*, file, func, line*/);
}

inline void my_free_dbg( void* ptr, const char* file, const char* func, long line)
{
	free(ptr);
	return;
	CMemManager* mm = CMemManager::instance();
	if (!mm) {
		return;
	}
	return mm->free(ptr/*, file, func, line*/);
}

#endif // __MY_MEMORY_H__
