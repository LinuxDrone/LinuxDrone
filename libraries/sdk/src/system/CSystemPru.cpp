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

#include "CSystemPru.h"
#include "system/Logger"

#define PRUSS_MAX_IRAM_SIZE	0x2000

using namespace std;

CSystemPru::CSystemPru()
{

}

CSystemPru::CSystemPru(const CString& pathToBin, int pruNumber)
{
	m_binfile = pathToBin;
	m_pru = pruNumber;
	m_enabled = true;
	mem_fd = -1;
}

CSystemPru::~CSystemPru()
{
	Logger() << "~CSystemPru";
	if(mem_fd >= 0)
	{
		munmap(m_sharedMem, 0x400);
		close(mem_fd);
	}
}

// Disable the pru
void CSystemPru::DisablePru()
{
	m_enabled = false;
}

// initialize the pru
bool CSystemPru::Init()
{
	if(m_enabled)
	{

		/* open the device */
		// map the device and init the varible before use it
		mem_fd = open("/dev/mem", O_RDWR);
		if (mem_fd < 0) {
			Logger() << "Failed to open /dev/mem (" << strerror(errno) <<")";
			return false;
		}

		LoadImageToPru(m_binfile);

		/* map the shared memory */
		m_sharedMem = mmap(0, 0x400, PROT_WRITE | PROT_READ, MAP_SHARED, mem_fd, 0x4a310000);
		if (m_sharedMem == NULL) {
			Logger() << "Failed to map the device (" << strerror(errno) <<")";
			close(mem_fd);
			return false;
		}
	}
   return true;
}

void * CSystemPru::GetSharedMem()
{
	return m_sharedMem;
}


// load Image file to Pru, the image will not run until ResetPru is called
bool CSystemPru::LoadImageToPru(const CString& filename)
{
	if(m_enabled)
	{
		if(!ResetPru())
		{
			Logger() << "Failed to reset pru" << m_pru;
			return false;
		}
		return LoadImage(0x4a334000+(0x4000*m_pru), filename);	// 0x4a334000 is address of instruction in Pru0
	}
	return true;
}

// Run image on Pru
bool CSystemPru::RunPru()
{
	Logger() << "Run Pru" << m_pru;
	return WriteUInt32(0x4a322000+(0x2000*m_pru), 0xa);
}

// Reset Pru,the image won't run
bool CSystemPru::ResetPru()
{
	Logger() << "Reset Pru" << m_pru;
	return WriteUInt32(0x4a322000+(0x2000*m_pru), 0x10a);
}

// write a unsigned long to memory
bool CSystemPru::WriteUInt32(unsigned long addr, unsigned long data)
{
	if(m_enabled)
	{
		/* Initialize example */
		/* map the shared memory */
		void * pMem = mmap(0, 0x2000, PROT_WRITE | PROT_READ, MAP_SHARED, mem_fd, addr);
		if (pMem == NULL)
		{
			Logger() << "Failed to map the device (" << strerror(errno) <<")";
			close(mem_fd);
			return false;
		}
		*(unsigned long *)(pMem) = data;
		Logger() << "write data to memory";
		munmap(pMem, 0x2000);
		return true;

	}
	return true;
}

// read a unsigned long from memroy
bool CSystemPru::ReadUInt32(unsigned long addr, unsigned long & data)
{
	if(m_enabled)
	{
		/* Initialize example */
		/* map the shared memory */
		void * pMem = mmap(0, 0x4, PROT_WRITE | PROT_READ, MAP_SHARED, mem_fd, addr);
		if (pMem == NULL)
		{
			Logger() << "Failed to map the device (" << strerror(errno) <<")";
			close(mem_fd);
			return false;
		}
		data = *(unsigned long *)(pMem);
		Logger() << "read data from memory";
		munmap(pMem, 0x4);
		return true;
	}
	return true;
}

// load image file
bool CSystemPru::LoadImage(unsigned long addr, const CString& filename)
{
	if(m_enabled)
	{
		FILE *fPtr;

		// Open an File from the hard drive
		fPtr = fopen(filename.data(), "rb");
		if (fPtr == NULL) {
			Logger() << "Image File" << filename.data() << "open failed";
		} else {
			Logger() << "Image File" << filename.data() << "open passed";
		}
		// Read file size
		fseek(fPtr, 0, SEEK_END);
		// read file
		unsigned char fileDataArray[PRUSS_MAX_IRAM_SIZE];
		int fileSize = 0;
		fileSize = ftell(fPtr);

		if (fileSize == 0) {
			Logger() << "File read failed.. Closing program";
			fclose(fPtr);
			return -1;
		}

		fseek(fPtr, 0, SEEK_SET);

		if (fileSize !=
			fread((unsigned char *) fileDataArray, 1, fileSize, fPtr)) {
			Logger() << "WARNING: File Size mismatch";
		}
		fclose(fPtr);
		/* Initialize example */
		/* map the shared memory */
		void * pMem = mmap(0, 0x2000, PROT_WRITE | PROT_READ, MAP_SHARED, mem_fd, addr);
		if (pMem == NULL) {
			Logger() << "Failed to map the device (" << strerror(errno) <<")";
			close(mem_fd);
			return false;
		}
		char * p = (char*)pMem;
		for(int i = 0; i < fileSize; i ++)
		{
			*(p + i) = fileDataArray[i];
		}
		Logger() << "write file to memory";
		munmap(pMem, 0x2000);
		return true;

	}
	return true;
}
