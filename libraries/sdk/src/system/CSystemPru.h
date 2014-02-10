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

#pragma once

#include "core/CObject"
#include "thread/CMutex"
#include "text/CString"

#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>


class CSystemPru : public CObject
{
public:
	CSystemPru();
	CSystemPru(const CString& pathToBin, int pruNumber);
	virtual ~CSystemPru();

	// disable Pru, function for debug program without getting bus error
	void DisablePru();
	// init the Pru
	bool Init();
	// load Image file to Pru, the image will not run until ResetPru0 is called
	bool LoadImageToPru(const CString& filename);
	// Run image on Pru
	bool RunPru();
	// Reset Pru,the image won't run
	bool ResetPru();
	// Get shared memory
	void* GetSharedMem();
	// write a unsigned long to memory
	bool WriteUInt32(unsigned long addr, unsigned long data);
	// read a unsigned long from memroy
	bool ReadUInt32(unsigned long addr, unsigned long & data);

protected:
	CString m_binfile;
	int m_pru;

	int mem_fd;
	void *m_sharedMem;

	bool m_enabled;	// flag for if the pru is enabled
	// load image file
	bool LoadImage(unsigned long addr, const CString& filename);

};
