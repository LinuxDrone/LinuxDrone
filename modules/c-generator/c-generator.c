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

#include "c-generator.h"
#include "c-generator.helper.h"
#include "math.h"

bool printEnable = false;

// Загрузка параметров из базы.
void settingsLoad(generatorParams_t *cfg, params_c_generator_t *params);
// Обработка данных
bool calcGenerator(generatorParams_t *cfg, float *out);

/**
 * @brief c_pid_run Функция рабочего потока модуля
 * @param module Указатель на структуру модуля
 */
void c_generator_run (module_c_generator_t *module)
{
    long last_print_time = (long)rt_timer_read();
    long print_period = rt_timer_ns2ticks(1000000000);

    // Структура данных, содержащая информацию о параметрах pid модуля
    generatorParams_t	generatorParams;


    // Выходные данные модуля
    float out;


    settingsLoad(&generatorParams, &module->params_c_generator);

    while(1) {
        get_input_data(&module->module_info);

        if(rt_timer_read() - last_print_time > print_period)
        {
            printEnable = true;
            last_print_time = rt_timer_read();
        }



        // Запускаем расчет генерируемого сигнала.
        calcGenerator(&generatorParams, &out);


        // Вывод данных из модуля
        outGen_t* outGen;
        checkout_outGen(module, &outGen);
        outGen->pos =  out;
        outGen->inv = -out;
        checkin_outGen(module, &outGen);
        //if(printEnable) printf("out = %f\n",out);

        printEnable = false;
    }
}


/* ---------------  FUNCTIONS -----------------------*/

/**
 * @brief Читает данные из базы MongoDB
 * И сохраняет в структуре generatorParams_t
 */
void settingsLoad(generatorParams_t *cfg, params_c_generator_t *params)
{
     cfg->enState       = params->enState;
     cfg->enMeander     = params->enMeander;
     cfg->enSine        = params->enSine;
     cfg->period        = params->period;
     cfg->dutyCycle     = params->dutyCycle;
     cfg->stateData     = params->stateData;
     cfg->rangeLow      = params->RangeLow;
     cfg->rangeCenter   = params->RangeCenter;
     cfg->rangeHigh     = params->RangeHigh;
}


/**
 * @brief Генерация сигнала по заданным параметрам
 */
bool calcGenerator(generatorParams_t *cfg, float *out)
{

    if(cfg->enState) {
        *out = cfg->stateData;

    } else if(cfg->enMeander){

    } else if(cfg->enSine) {

    }

    // Ограничение выходного диапазона
    if(*out < cfg->rangeLow) *out = cfg->rangeLow;
    if(*out > cfg->rangeHigh) *out = cfg->rangeHigh;

    return true;
}
