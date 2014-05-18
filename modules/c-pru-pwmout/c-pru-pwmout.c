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

#include "c-pru-pwmout.helper.h"
#include "pru-functions.h"

bool initPwmOutput(pru_info_t *pru_info, uint8_t *m_sharedMem, char *pathBin, uint32_t *m_pwm, uint32_t *m_period);
void setChannelPeriod(uint8_t *m_sharedMem, int channel, uint32_t period);
void setChannelPulseWidth(uint8_t *m_sharedMem, int channel, uint32_t pw);


void c_pru_pwmout_run (module_c_pru_pwmout_t *module)
{
    int cycle=0;
    // Указатель на структуру с входными данными модуля.
    input_t* input;
    // Указатель на обслуживающую структуру блока PRU
    pru_info_t *pru_info;

    uint32_t m_pwm[12];
    uint32_t m_period[12];
    uint8_t *m_sharedMem;

    int i;

    while(1) {
        get_input_data(module);

        // проверим, обновились ли данные
        if(module->module_info.updated_input_properties!=0)
        {
            // есть новые данные
            input = (input_t*)module->module_info.input_data;

            m_pwm[0] = (uint32_t)input->pwm1;
            m_pwm[1] = (uint32_t)input->pwm2;
            m_pwm[2] = (uint32_t)input->pwm3;
            m_pwm[3] = (uint32_t)input->pwm4;
            m_pwm[4] = (uint32_t)input->pwm5;
            m_pwm[5] = (uint32_t)input->pwm6;
            m_pwm[6] = (uint32_t)input->pwm7;
            m_pwm[7] = (uint32_t)input->pwm8;
            m_pwm[8] = (uint32_t)input->pwm9;
            m_pwm[9] = (uint32_t)input->pwm10;
            m_pwm[10] = (uint32_t)input->pwm11;
            m_pwm[11] = (uint32_t)input->pwm12;

        }
        else
        {
            // вышел таймаут
        }


        for(i=0;i<12;i++) {
            // Проверка входных данных на диапазон
            if(m_pwm[i]<1000) m_pwm[i] = 1000;
            if(m_pwm[i]>2000) m_pwm[i] = 2000;
            // Обновление данных периода шим в блоке PRU
            setChannelPulseWidth(m_sharedMem,i,m_pwm[i]);
        }

        // Эти данные следует добыть из разделяемой памяти, если они не придут через трубу
        module->module_info.refresh_input_mask =
                pwm1  |
                pwm2  |
                pwm3  |
                pwm4  |
                pwm5  |
                pwm6  |
                pwm7  |
                pwm8  |
                pwm9  |
                pwm10 |
                pwm11 |
                pwm12;

        cycle++;
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
bool initPwmOutput(pru_info_t *pru_info, uint8_t *m_sharedMem, char *pathBin, uint32_t *m_pwm, uint32_t *m_period)
{
    SetPathBinPru(pru_info, pathBin);
    EnablePru(pru_info);

    if (!InitPRU(pru_info)) {
        printf("Failed InitPRU()\n");
        return false;
    }

    m_sharedMem = (uint8_t *)GetSharedMem(pru_info);

    int i;
    for(i=0;i<12;i++) {
        m_pwm[i]=1000;
        setChannelPulseWidth(m_sharedMem, i,m_pwm[i]);
    }

    for(i=0;i<4;i++) {
        // 50Hz
        m_period[i]=20000;
        setChannelPeriod(m_sharedMem, i,m_period[i]);
    }
    for(i=4;i<8;i++) {
        // 400Hz
        m_period[i]=2500;
        setChannelPeriod(m_sharedMem, i,m_period[i]);
    }
    for(i=8;i<12;i++) {
        // 200Hz
        m_period[i]=5000;
        setChannelPeriod(m_sharedMem, i,m_period[i]);
    }


    if (!RunPru(pru_info))
    {
        printf("Failed RunPru()\n");
        return false;
    }
    return true;
}


/**
 * @param m_sharedMem - указатель на расшаренную память блока PRU
 * @brief Функция задания периода pwm
 * @param channel - номер канала
 * @param period - период в мкс.
 */
void setChannelPeriod(uint8_t *m_sharedMem, int channel, uint32_t period)
{
    if((channel >=0) && (channel < 12)) {
        *(unsigned long *)(m_sharedMem + 0x204 + (channel*0x10)) = period * 200;
    }
}


/**
 * @param m_sharedMem - указатель на расшаренную память блока PRU
 * @brief Функция задания длительности ипульса pwm
 * @param channel - номер канала
 * @param pw - длительность импульса в мкс.
 */
void setChannelPulseWidth(uint8_t *m_sharedMem, int channel, uint32_t pw)
{
    if((channel >=0) && (channel < 12)) {
        if((pw > 200) && (pw < 2200)) {
            *(unsigned long *)(m_sharedMem + 0x200 + (channel*0x10)) = pw * 200;
        }
    }
}
