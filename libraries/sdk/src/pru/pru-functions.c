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

#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "../include/pru/pru-functions.h"


#define PRUSS_MAX_IRAM_SIZE	0x2000



/**
 * @brief
 * \~russian
 * \~english
 * @param Всем функциям на вход передается адрес структуры,
 *              содержащей информацию по использываемому блоку PRU
 * @param опциональные параметры, адреса регистров, данные для записи.
 * @return
 * \~russian true в случае успеха
 */

// Выбор используемого блока PRU
void SetNumPru(pru_info_t *pru_info, int pruNum)
{
    pru_info->pruNum = pruNum;
}

// Задание пути к файлу с кодом для блока PRU
void SetPathBinPru(pru_info_t *pru_info, char *pathBin)
{
    pru_info->pathBin = pathBin;
}

// enable Pru, function for debug program without getting bus error
void EnablePru(pru_info_t *pru_info)
{
    pru_info->enabled = true;
}

// disable Pru, function for debug program without getting bus error
void DisablePru(pru_info_t *pru_info)
{
    pru_info->enabled = false;
}

// init the Pru
bool InitPRU(pru_info_t *pru_info)
{
    if(pru_info->enabled)
    {
        /* open the device */
        // map the device and init the varible before use it
        pru_info->mem_fd = open("/dev/mem", O_RDWR);
        if (pru_info->mem_fd < 0) {
            printf("Failed to open /dev/mem (%s)\n", strerror(errno));
            return false;
        }

        LoadImageToPru(pru_info);

        /* map the shared memory */
        pru_info->sharedMem = mmap(0, 0x400, PROT_WRITE | PROT_READ, MAP_SHARED, pru_info->mem_fd, 0x4a310000);
        if (pru_info->sharedMem == NULL) {
            printf("Failed to map the device (%s)\n", strerror(errno));
            close(pru_info->mem_fd);
            return false;
        }
    }
   return true;
}

// load Image file to Pru, the image will not run until ResetPru0 is called
bool LoadImageToPru(pru_info_t *pru_info)
{
    if(pru_info->enabled)
    {
        if(!ResetPru(pru_info))
        {
            printf("Failed to reset pru%d\n",pru_info->pruNum);
            return false;
        }
            // 0x4a334000 is address of instruction in Pru0
            unsigned long addr = 0x4a334000+(0x4000*pru_info->pruNum);
            FILE *fPtr;

            // Open an File from the hard drive
            fPtr = fopen(pru_info->pathBin, "rb");
            if (fPtr == NULL) {
                printf("Image File  %s open failed\n", pru_info->pathBin);
            } else {
                printf("Image File  %s open passed\n", pru_info->pathBin);
            }
            // Read file size
            fseek(fPtr, 0, SEEK_END);
            // read file
            unsigned char fileDataArray[PRUSS_MAX_IRAM_SIZE];
            int fileSize = 0;
            fileSize = ftell(fPtr);

            if (fileSize == 0) {
                printf("File read failed.. Closing program\n");
                fclose(fPtr);
                return -1;
            }

            fseek(fPtr, 0, SEEK_SET);

            if (fileSize !=
                fread((unsigned char *) fileDataArray, 1, fileSize, fPtr)) {
                printf("WARNING: File Size mismatch\n");
            }
            fclose(fPtr);
            /* Initialize example */
            /* map the shared memory */
            void * pMem = mmap(0, 0x2000, PROT_WRITE | PROT_READ, MAP_SHARED, pru_info->mem_fd, addr);
            if (pMem == NULL) {
                printf("Failed to map the device (%s)\n", strerror(errno));
                close(pru_info->mem_fd);
                return false;
            }
            char * p = (char*)pMem;
            int i;
            for(i = 0; i < fileSize; i ++)
            {
                *(p + i) = fileDataArray[i];
            }
            printf("write file to memory\n");
            munmap(pMem, 0x2000);
            return true;

    }
    return true;
}

// Run image on Pru
bool RunPru(pru_info_t *pru_info)
{
    printf("Run Pru%d\n", pru_info->pruNum);
    return WriteUInt32(pru_info, 0x4a322000+(0x2000*pru_info->pruNum), 0xa);
}

// Reset Pru,the image won't run
bool ResetPru(pru_info_t *pru_info)
{
    printf("Reset Pru%d\n", pru_info->pruNum);
    return WriteUInt32(pru_info, 0x4a322000+(0x2000*pru_info->pruNum), 0x0);
}

// Get shared memory
void* GetSharedMem(pru_info_t *pru_info)
{
    return pru_info->sharedMem;
}

// write a unsigned long to memory
bool WriteUInt32(pru_info_t *pru_info, unsigned long addr, unsigned long data)
{
    if(pru_info->enabled)
    {
        /* Initialize example */
        /* map the shared memory */
        void * pMem = mmap(0, 0x2000, PROT_WRITE | PROT_READ, MAP_SHARED, pru_info->mem_fd, addr);
        if (pMem == NULL)
        {
            printf("Failed to map the device (%s)\n", strerror(errno));
            close(pru_info->mem_fd);
            return false;
        }
        *(unsigned long *)(pMem) = data;
        printf("write data to memory\n");
        munmap(pMem, 0x2000);
        return true;

    }
    return true;
}

// read a unsigned long from memory
bool ReadUInt32(pru_info_t *pru_info, unsigned long addr, unsigned long *data)
{
    if(pru_info->enabled)
    {
        /* Initialize example */
        /* map the shared memory */
        void * pMem = mmap(0, 0x4, PROT_WRITE | PROT_READ, MAP_SHARED, pru_info->mem_fd, addr);
        if (pMem == NULL)
        {
            printf("Failed to map the device (%s)\n", strerror(errno));
            close(pru_info->mem_fd);
            return false;
        }
        *data = *(unsigned long *)(pMem);
        printf("read data from memory\n");
        munmap(pMem, 0x4);
        return true;
    }
    return true;
}
