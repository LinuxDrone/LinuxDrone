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

#include "c-pru-pwmin.helper.h"
#include "pru-functions.h"

bool initPwmInput(pru_info_t *pru_info, uint8_t **m_sharedMem, char *pathBin, uint32_t *m_pwm);
uint32_t readChannel(uint8_t *m_sharedMem,int ch);


void c_pru_pwmin_run (module_c_pru_pwmin_t *module)
{
    int cycle=0;
    // Указатель на структуру с выходными данными модуля.
    PWM_out_t* mPWMout;
    // Указатель на обслуживающую структуру блока PRU
    pru_info_t *pru_info, m_pru_info;
    pru_info = &m_pru_info;

    uint32_t m_pwm[12];
    uint8_t *m_sharedMem;
    char *pathBin = "/usr/local/linuxdrone/modules/c-pru-pwmin/pru-pwmin.bin";
    // true если инициализация модуля прошла успешно
    bool     bPruInit = false;
    int i;

    long last_print_time = rt_timer_read();
    long print_period = rt_timer_ns2ticks(1000000000);

    while(1) {

        // Читаем входные данные модуля
        get_input_data(module);

        // Инициализация модуля PRU
         if (!bPruInit) {
           if(!initPwmInput(pru_info, &m_sharedMem, pathBin, m_pwm))
            {
                if(rt_timer_read() - last_print_time > print_period)
                {
                    printf("Failed initPwmInput \n");
                    last_print_time = rt_timer_read();
                }
                continue;
            }
            bPruInit = true;
        }

        // проверим, обновились ли данные
        if(module->module_info.updated_input_properties!=0)
        {
            // есть новые данные
        }
        else
        {
            // вышел таймаут
        }


        for(i=0; i<7; i++) {
            m_pwm[i]=readChannel(m_sharedMem, i);
        }

        checkout_PWM_out(module, &mPWMout);
        mPWMout->ch1 = (float)m_pwm[0];
        mPWMout->ch2 = (float)m_pwm[1];
        mPWMout->ch3 = (float)m_pwm[2];
        mPWMout->ch4 = (float)m_pwm[3];
        mPWMout->ch5 = (float)m_pwm[4];
        mPWMout->ch6 = (float)m_pwm[5];
        mPWMout->ch7 = (float)m_pwm[6];
        checkin_PWM_out(module, &mPWMout);
    }
}

// ---------------- Функции ------------------------------------

/**
 * @brief Функция инициализации работы с блоком PRU
 *          Настройка таймингов блока PRU
 *          Загрузка кода для PRU
 * @param pru_info - обслуживающая структура блока PRU
 * @param m_sharedMem - возвращает указатель на расшаренную память блока PRU
 * @param pathBin - указатель на строку с путем к коду для PRU
 * @return Возвращает true если успех
 */
bool initPwmInput(pru_info_t *pru_info, uint8_t **m_sharedMem, char *pathBin, uint32_t *m_pwm)
{
    SetPathBinPru(pru_info, pathBin);
    SetNumPru(pru_info, 0);
    EnablePru(pru_info);

    if (!InitPRU(pru_info)) {
        printf("Failed InitPRU()\n");
        return false;
    }

    *m_sharedMem = (uint8_t *)GetSharedMem(pru_info);

    int i;
    for(i=0;i<12;i++) {
        m_pwm[i]=0;
    }

    if (!RunPru(pru_info))
    {
        printf("Failed RunPru()\n");
        return false;
    }
    return true;
}

/**
 * @brief Функция чтения длительности импульса в канале
 * @param m_sharedMem - указатель на расшаренную память блока PRU
 * @param channel - номер канала
 * @return channelData - длительность импульса в мкс.
 */
uint32_t readChannel(uint8_t *m_sharedMem,int ch)
{
    if((ch >=0) && (ch < 7)) {
        uint32_t channelData = (*(unsigned long *)(m_sharedMem + 0x104 + (0x10*ch))/200);
        if((channelData < 2100) && (channelData > 900)) {
            return channelData;
        }
        //printf("readChannel out off range data input ch%d = %lu", ch, channelData);
    }
    return 0;
}
