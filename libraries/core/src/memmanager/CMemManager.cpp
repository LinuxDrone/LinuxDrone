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

#include "CMemManager.h"
#include <stdio.h>
#include <assert.h>

CMemManager::CMemManager() {
#ifdef XENO_SERVICES
	static const int heapSize = 1024 * 1024;
	int err = rt_heap_create(&m_heap, "Memory Heap", heapSize, H_FIFO);
	if (err) {
		printf("%s: error creating Xenomai heap. error = %d\n", __FUNCTION__,
				err);
	}
#endif
}

CMemManager::~CMemManager() {
#ifdef XENO_SERVICES
	rt_heap_delete(&m_heap);
#endif
}

CMemManager* CMemManager::instance() {
	static CMemManager manager;
	return &manager;
}

void* CMemManager::alloc(size_t size) {
#ifdef XENO_SERVICES
	void* ptr;
	int err = rt_heap_alloc(&m_heap, size+sizeof(size_t), TM_INFINITE, &ptr);
	if (err) {
		printf("%s: error allocating memory. error = %d\n", __FUNCTION__, err);
		return 0;
	}
	char* data = (char*)ptr;
	*((size_t*)data) = size;
	data += sizeof(size_t);
	return data;
#else
	return 0;
#endif
}

void* CMemManager::realloc(void* ptr, size_t size)
{
	if (!ptr) {
		return alloc(size);
	}
#ifdef XENO_SERVICES
	char* data = (char*)ptr;
	data -= sizeof(size_t);
	size_t sizeOld = *((size_t*)data);
	if (sizeOld == size) {
		return ptr;
	}
	if (sizeOld > size) {
		*((size_t*)data) = size;
		return ptr;
	}
	void* newPtr = (char*)alloc(size);
	if (!newPtr) {
		return 0;
	}
	memcpy(newPtr, ptr, sizeOld);
	free(ptr);
	return newPtr;
#else
	return 0;
#endif
}

void CMemManager::free(void* ptr)
{
	if (!ptr) {
		return;
	}
#ifdef XENO_SERVICES
	char* data = (char*)ptr;
	data -= sizeof(size_t);
	int err = rt_heap_free(&m_heap, data);
	if (err) {
		printf("%s: error release pointer to memory. error = %d\n", __FUNCTION__, err);
	}
#endif
}
