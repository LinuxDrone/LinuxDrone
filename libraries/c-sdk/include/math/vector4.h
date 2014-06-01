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

#ifndef _C_SDK_VECTOR_4_H_
#define _C_SDK_VECTOR_4_H_

typedef struct tagVector4 {
    float v[4];

} VECTOR4, *PVECTOR4;

VECTOR4 vector4Make(float x, float y, float z, float w);

#endif // _C_SDK_VECTOR_4_H_
