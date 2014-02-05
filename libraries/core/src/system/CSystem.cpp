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

#include "CSystem.h"

#ifndef WIN32
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#if defined(HAVE_SYS_SYSCTL_H) && \
    !defined(_SC_NPROCESSORS_ONLN) && !defined(_SC_NPROC_ONLN)
#include <sys/param.h>
#include <sys/sysctl.h>
#endif

#if !defined(__APPLE__) && !defined(__ANDROID__)
#include <execinfo.h>
#endif
#include <cxxabi.h>
#include <cstring>
#include <cstdlib>
#endif

CSystem::CSystem()
{
}

void CSystem::sleep( int ms )
{
#ifdef _WIN32
    Sleep( ms );
#else
    usleep( ms * 1000 );
#endif
}
