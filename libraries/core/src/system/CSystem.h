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

#ifndef CSYSTEM_H
#define CSYSTEM_H

#include <unistd.h>
class CSystem
{
public:
    CSystem();

    static void sleep( int ms );

	static void* alignedAlloc(size_t size, size_t alignment = 16);
	static void alignedFree(void* ptr);
};

#endif // CSYSTEM_H
