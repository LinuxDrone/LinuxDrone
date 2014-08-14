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

#include "c-scale.h"
#include "c-scale.helper.h"
#include "math.h"

// Загрузка параметров из базы.
void settingsLoad(scaleChParams_t *cfg, params_c_scale_t *params);
// Обработка данных
bool calcScale(scaleChParams_t *cfg_ch, float *in_data, float *out);

/**
 * @brief c_pid_run Функция рабочего потока модуля
 * @param module Указатель на структуру модуля
 */
void c_scale_run (module_c_scale_t *module)
{
    bool printEnable = false;

    // Указатель на структуру с входными данными модуля.
    input_t* input;

    long last_print_time = (long)rt_timer_read();
    long print_period = rt_timer_ns2ticks(1000000000);

    // Структура данных, содержащая информацию о параметрах pid модуля
    scaleChParams_t	scaleParams[3];

    // Входные данные модуля
    float in_data[3];

    // Выходные данные модуля
    float out[3];

    settingsLoad(scaleParams, &module->params_c_scale);

    while(1) {
        get_input_data(&module->module_info);

        if(rt_timer_read() - last_print_time > print_period)
        {
            printEnable = true;
            last_print_time = rt_timer_read();
        }

        // проверим, обновились ли данные
        if(module->module_info.updated_input_properties!=0)
        {
            // есть новые данные
            input = (input_t*)module->module_info.input_data;
            // Чтение входных данных
            in_data[Roll]   = input->in_Roll;
            in_data[Pitch]  = input->in_Pitch;
            in_data[Yaw]    = input->in_Yaw;


            // Запускаем расчет SumScale для каждого канала.
            // Если входные данные не входят в входной диапазон то игнирируем их.
            uint8_t ch;
            for (ch = 0; ch < 3; ch++) {
                if(!calcScale(&scaleParams[ch], &in_data[ch], &out[ch])) continue;
            }

            // Вывод данных из модуля
            outScale_t* outScale;
            checkout_outScale(module, &outScale);
            outScale->roll  = out[Roll];
            outScale->pitch = out[Pitch];
            outScale->yaw   = out[Yaw];
            checkin_outScale(module, &outScale);
        }
        else
        {
            // вышел таймаут
        }

        // Эти данные следует добыть из разделяемой памяти, если они не придут через трубу
        //module->module_info.refresh_input_mask = in_Roll | in_Pitch | in0_Yaw;


        //if(printEnable) printf("out[Roll] = %f\n",out[Roll]);


        printEnable = false;
    }
}


/* ---------------  FUNCTIONS -----------------------*/

/**
 * @brief Читает данные из базы MongoDB
 * И сохраняет в структуре sumScaleParams_t
 */
void settingsLoad(scaleChParams_t *cfg, params_c_scale_t *params)
{
     cfg[Roll].out_RangeLow     = params->out_RollRangeLow;
     cfg[Roll].out_RangeCenter  = params->out_RollRangeCenter;
     cfg[Roll].out_RangeHigh    = params->out_RollRangeHigh;
     cfg[Roll].in_Invert        = params->in_RollInvert;
     cfg[Roll].in_RangeLow      = params->in_RollRangeLow;
     cfg[Roll].in_RangeCenter   = params->in_RollRangeCenter;
     cfg[Roll].in_RangeHigh     = params->in_RollRangeHigh;

     cfg[Pitch].out_RangeLow    = params->out_PitchRangeLow;
     cfg[Pitch].out_RangeCenter = params->out_PitchRangeCenter;
     cfg[Pitch].out_RangeHigh   = params->out_PitchRangeHigh;
     cfg[Pitch].in_Invert       = params->in_PitchInvert;
     cfg[Pitch].in_RangeLow     = params->in_PitchRangeLow;
     cfg[Pitch].in_RangeCenter  = params->in_PitchRangeCenter;
     cfg[Pitch].in_RangeHigh    = params->in_PitchRangeHigh;

     cfg[Yaw].out_RangeLow      = params->out_YawRangeLow;
     cfg[Yaw].out_RangeCenter   = params->out_YawRangeCenter;
     cfg[Yaw].out_RangeHigh     = params->out_YawRangeHigh;
     cfg[Yaw].in_Invert         = params->in_YawInvert;
     cfg[Yaw].in_RangeLow       = params->in_YawRangeLow;
     cfg[Yaw].in_RangeCenter    = params->in_YawRangeCenter;
     cfg[Yaw].in_RangeHigh      = params->in_YawRangeHigh;


     // Вычисление масштабных кофэффициентов
     uint8_t ch;
     for (ch = 0; ch < 3; ch++) {
         cfg[ch].scaleLow = fabs((cfg[ch].out_RangeCenter - cfg[ch].out_RangeLow) / (cfg[ch].in_RangeCenter - cfg[ch].in_RangeLow));
         cfg[ch].scaleHigh = fabs((cfg[ch].out_RangeHigh - cfg[ch].out_RangeCenter) / (cfg[ch].in_RangeHigh - cfg[ch].in_RangeCenter));

         if(cfg[ch].in_Invert) {
             cfg[ch].scaleLow = -cfg[ch].scaleLow;
             cfg[ch].scaleHigh = -cfg[ch].scaleHigh;
         }
     }
}


/**
 * @brief Масштабирование и суммирование сигналов
 */
bool calcScale(scaleChParams_t *cfg_ch, float *in_data, float *out)
{
    float op0 = 0.0f;

    // Проверка входного сигнала, на вхождение в границы диапазона
    if(*in_data < cfg_ch->in_RangeLow | *in_data > cfg_ch->in_RangeHigh) return false;

    // Масштабируем канал
    op0 = (*in_data - cfg_ch->in_RangeCenter);
    if(*in_data < cfg_ch->in_RangeCenter) {
        op0 *= cfg_ch->scaleLow;
    } else {
        op0 *= cfg_ch->scaleHigh;
    }

    // Суммируем
    *out = op0 + cfg_ch->out_RangeCenter;

    // Ограничение выходного диапазона
    if(*out < cfg_ch->out_RangeLow) *out = cfg_ch->out_RangeLow;
    if(*out > cfg_ch->out_RangeHigh) *out = cfg_ch->out_RangeHigh;

    return true;
}
