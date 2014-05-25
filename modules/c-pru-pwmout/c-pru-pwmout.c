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

bool initPwmOutput(pru_info_t *pru_info, uint8_t **m_sharedMem, char *pathBin, uint32_t *m_pwm, uint32_t *m_period);
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
    char *pathBin = "/usr/local/linuxdrone/modules/c-pru-pwmout/pru-pwmout.bin";
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
           if(!initPwmOutput(pru_info, &m_sharedMem, pathBin, m_pwm, m_period))
            {
                if(rt_timer_read() - last_print_time > print_period)
                {
                    printf("Failed initPwmOutput\n");
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
            input = (input_t*)module->module_info.input_data;

            m_pwm[0] = (uint32_t)input->ch1;
            m_pwm[1] = (uint32_t)input->ch2;
            m_pwm[2] = (uint32_t)input->ch3;
            m_pwm[3] = (uint32_t)input->ch4;
            m_pwm[4] = (uint32_t)input->ch5;
            m_pwm[5] = (uint32_t)input->ch6;
            m_pwm[6] = (uint32_t)input->ch7;
            m_pwm[7] = (uint32_t)input->ch8;
            m_pwm[8] = (uint32_t)input->ch9;
            m_pwm[9] = (uint32_t)input->ch10;
            m_pwm[10] = (uint32_t)input->ch11;
            m_pwm[11] = (uint32_t)input->ch12;

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
                ch1  |
                ch2  |
                ch3  |
                ch4  |
                ch5  |
                ch6  |
                ch7  |
                ch8  |
                ch9  |
                ch10 |
                ch11 |
                ch12;

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
bool initPwmOutput(pru_info_t *pru_info, uint8_t **m_sharedMem, char *pathBin, uint32_t *m_pwm, uint32_t *m_period)
{
    SetPathBinPru(pru_info, pathBin);
    SetNumPru(pru_info, 1);
    EnablePru(pru_info);

    if (!InitPRU(pru_info)) {
        printf("Failed InitPRU()\n");
        return false;
    }

    *m_sharedMem = (uint8_t *)GetSharedMem(pru_info);

    int i;

    for(i=0;i<12;i++) {
        m_pwm[i]=1500;
        setChannelPulseWidth(*m_sharedMem, i,m_pwm[i]);
    }

    for(i=0;i<4;i++) {
        // 50Hz
        m_period[i]=20000;
        setChannelPeriod(*m_sharedMem, i,m_period[i]);
    }
    for(i=4;i<8;i++) {
        // 400Hz
        m_period[i]=2500;
        setChannelPeriod(*m_sharedMem, i,m_period[i]);
    }
    for(i=8;i<12;i++) {
        // 200Hz
        m_period[i]=5000;
        setChannelPeriod(*m_sharedMem, i,m_period[i]);
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
 * @brief Функция задания длительности импульса pwm
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
