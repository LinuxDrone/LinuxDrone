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

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
  Структура данных, содержащая информацию о текущем режиме работы блоков PRU
  Адресов доступа к блокам через расшаренную память.
 */

typedef struct
{
    // Указатель на путь к файлу с исполняемым кодом для блока PRU
    char *pathBin;
    // Номер блока PRU (0,1)
    int pruNum;
    // Дескриптор памяти устройства.
    int mem_fd;
    // Указатель на начало расшаренного блока памяти PRU
    void *sharedMem;
    // Флаг наличия включенного состояния блока PRU
    bool enabled;

} pru_info_t;

/**
 * @brief Функции управления работой блока PRU
 * Загрузка исполняемого кода для PRU
 * Получение доступа к расшаренной памяти модуля PRU
 */

// Выбор используемого блока PRU
void SetNumPru(pru_info_t *pru_info, int pruNum);

// Задание пути к файлу с кодом для блока PRU
void SetPathBinPru(pru_info_t *pru_info, char *pathBin);

// enable Pru, function for debug program without getting bus error
void EnablePru(pru_info_t *pru_info);

// disable Pru, function for debug program without getting bus error
void DisablePru(pru_info_t *pru_info);

// init the Pru
bool InitPRU(pru_info_t *pru_info);

// load Image file to Pru, the image will not run until ResetPru0 is called
bool LoadImageToPru(pru_info_t *pru_info);

// Run image on Pru
bool RunPru(pru_info_t *pru_info);

// Reset Pru,the image won't run
bool ResetPru(pru_info_t *pru_info);

// Get shared memory
void* GetSharedMem(pru_info_t *pru_info);

// write a unsigned long to memory
bool WriteUInt32(pru_info_t *pru_info, unsigned long addr, unsigned long data);

// read a unsigned long from memory
bool ReadUInt32(pru_info_t *pru_info, unsigned long addr, unsigned long *data);

#ifdef __cplusplus
}
#endif
