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

#include "c-sum-scale.h"
#include "c-sum-scale.helper.h"
#include "math.h"

// Загрузка параметров из базы.
void settingsLoad(sumScaleChParams_t *cfg);
// Обработка данных
bool calcSumScale(sumScaleChParams_t *cfg_ch, struct in_data *in_err, float *out);

/**
 * @brief c_pid_run Функция рабочего потока модуля
 * @param module Указатель на структуру модуля
 */
void c_sum_scale_run (module_c_sum_scale_t *module)
{
    bool printEnable = false;

    // Указатель на структуру с входными данными модуля.
    input_t* input;

    long last_print_time = (long)rt_timer_read();
    long print_period = rt_timer_ns2ticks(1000000000);

    // Структура данных, содержащая информацию о параметрах pid модуля
    sumScaleChParams_t	sumScaleParams[3], *psumScaleParams;
    psumScaleParams = sumScaleParams;

    // Входные данные модуля
    struct in_data in_err[3];

    // Выходные данные модуля
    float out[3];

    settingsLoad(sumScaleParams);

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
            in_err[Roll].in0   = input->in0_Roll;
            in_err[Roll].in1   = input->in1_Roll;
            in_err[Pitch].in0  = input->in0_Pitch;
            in_err[Pitch].in1  = input->in1_Pitch;
            in_err[Yaw].in0    = input->in0_Yaw;
            in_err[Yaw].in1    = input->in1_Yaw;


            // Запускаем расчет SumScale для каждого канала.
            // Если входные данные не входят в входной диапазон то игнирируем их.
            uint8_t ch;
            for (ch = 0; ch < 3; ch++) {
                if(!calcSumScale(&sumScaleParams[ch], &in_err[ch], &out[ch])) continue;
            }

            // Вывод данных из модуля
            outSumScale_t* outSumScale;
            checkout_outSumScale(module, &outSumScale);
            outSumScale->roll  = out[Roll];
            outSumScale->pitch = out[Pitch];
            outSumScale->yaw   = out[Yaw];
            checkin_outSumScale(module, &outSumScale);
        }
        else
        {
            // вышел таймаут
        }

        // Эти данные следует добыть из разделяемой памяти, если они не придут через трубу
        //module->module_info.refresh_input_mask = in0_Roll | in1_Roll | in0_Pitch | in1_Pitch | in0_Yaw | in1_Yaw;


        //if(printEnable) printf("out[Roll] = %f\n",out[Roll]);


        printEnable = false;
    }
}


/* ---------------  FUNCTIONS -----------------------*/

/**
 * @brief Читает данные из базы MongoDB
 * И сохраняет в структуре sumScaleParams_t
 */
void settingsLoad(sumScaleChParams_t *cfg)
{
     cfg[Roll].degEn            = DEG_ROLL_EN;
     cfg[Roll].out_RangeLow     = OUT_RANGE_LOW_ROLL;
     cfg[Roll].out_RangeCenter  = OUT_RANGE_CENTER_ROLL;
     cfg[Roll].out_RangeHigh    = OUT_RANGE_HIGH_ROLL;
     cfg[Roll].in0_Invert       = IN0_ROLL_INVERT;
     cfg[Roll].in0_RangeLow     = IN0_RANGE_LOW_ROLL;
     cfg[Roll].in0_RangeCenter  = IN0_RANGE_CENTER_ROLL;
     cfg[Roll].in0_RangeHigh    = IN0_RANGE_HIGH_ROLL;
     cfg[Roll].in1_Invert       = IN1_ROLL_INVERT;
     cfg[Roll].in1_RangeLow     = IN1_RANGE_LOW_ROLL;
     cfg[Roll].in1_RangeCenter  = IN1_RANGE_CENTER_ROLL;
     cfg[Roll].in1_RangeHigh    = IN1_RANGE_HIGH_ROLL;

     cfg[Pitch].degEn           = DEG_PITCH_EN;
     cfg[Pitch].out_RangeLow    = OUT_RANGE_LOW_PITCH;
     cfg[Pitch].out_RangeCenter = OUT_RANGE_CENTER_PITCH;
     cfg[Pitch].out_RangeHigh   = OUT_RANGE_HIGH_PITCH;
     cfg[Pitch].in0_Invert      = IN0_PITCH_INVERT;
     cfg[Pitch].in0_RangeLow    = IN0_RANGE_LOW_PITCH;
     cfg[Pitch].in0_RangeCenter = IN0_RANGE_CENTER_PITCH;
     cfg[Pitch].in0_RangeHigh   = IN0_RANGE_HIGH_PITCH;
     cfg[Pitch].in1_Invert      = IN1_PITCH_INVERT;
     cfg[Pitch].in1_RangeLow    = IN1_RANGE_LOW_PITCH;
     cfg[Pitch].in1_RangeCenter = IN1_RANGE_CENTER_PITCH;
     cfg[Pitch].in1_RangeHigh   = IN1_RANGE_HIGH_PITCH;

     cfg[Yaw].degEn             = DEG_YAW_EN;
     cfg[Yaw].out_RangeLow      = OUT_RANGE_LOW_YAW;
     cfg[Yaw].out_RangeCenter   = OUT_RANGE_CENTER_YAW;
     cfg[Yaw].out_RangeHigh     = OUT_RANGE_HIGH_YAW;
     cfg[Yaw].in0_Invert        = IN0_YAW_INVERT;
     cfg[Yaw].in0_RangeLow      = IN0_RANGE_LOW_YAW;
     cfg[Yaw].in0_RangeCenter   = IN0_RANGE_CENTER_YAW;
     cfg[Yaw].in0_RangeHigh     = IN0_RANGE_HIGH_YAW;
     cfg[Yaw].in1_Invert        = IN1_YAW_INVERT;
     cfg[Yaw].in1_RangeLow      = IN1_RANGE_LOW_YAW;
     cfg[Yaw].in1_RangeCenter   = IN1_RANGE_CENTER_YAW;
     cfg[Yaw].in1_RangeHigh     = IN1_RANGE_HIGH_YAW;

     // Вычисление масштабных кофэффициентов
     uint8_t ch;
     for (ch = 0; ch < 3; ch++) {
         cfg[ch].in0_scaleLow = fabs((cfg[ch].out_RangeCenter - cfg[ch].out_RangeLow) / (cfg[ch].in0_RangeCenter - cfg[ch].in0_RangeLow));
         cfg[ch].in0_scaleHigh = fabs((cfg[ch].out_RangeHigh - cfg[ch].out_RangeCenter) / (cfg[ch].in0_RangeHigh - cfg[ch].in0_RangeCenter));
         cfg[ch].in1_scaleLow = fabs((cfg[ch].out_RangeCenter - cfg[ch].out_RangeLow) / (cfg[ch].in1_RangeCenter - cfg[ch].in1_RangeLow));
         cfg[ch].in1_scaleHigh = fabs((cfg[ch].out_RangeHigh -cfg[ch].out_RangeCenter) / (cfg[ch].in1_RangeHigh - cfg[ch].in1_RangeCenter));

         if(cfg[ch].in0_Invert) {
             cfg[ch].in0_scaleLow = -cfg[ch].in0_scaleLow;
             cfg[ch].in0_scaleHigh = -cfg[ch].in0_scaleHigh;
         }
         if(cfg[ch].in1_Invert) {
             cfg[ch].in1_scaleLow = -cfg[ch].in1_scaleLow;
             cfg[ch].in1_scaleHigh = -cfg[ch].in1_scaleHigh;
         }
     }
}


/**
 * @brief Масштабирование и суммирование сигналов
 */
bool calcSumScale(sumScaleChParams_t *cfg_ch, struct in_data *in_err, float *out)
{
    float op0 = 0.0f, op1 = 0.0f;

    // Проверка входного сигнала, на вхождение в границы диапазона
    if(in_err->in0 < cfg_ch->in0_RangeLow | in_err->in0 > cfg_ch->in0_RangeHigh) return false;
    if(in_err->in1 < cfg_ch->in1_RangeLow | in_err->in1 > cfg_ch->in1_RangeHigh) return false;

    // Масштабируем первый канал сумматора
    op0 = (in_err->in0 - cfg_ch->in0_RangeCenter);
    if(in_err->in0 < cfg_ch->in0_RangeCenter) {
        op0 *= cfg_ch->in0_scaleLow;
    } else {
        op0 *= cfg_ch->in0_scaleHigh;
    }
    // Масштабируем второй канал сумматора
    op1 = (in_err->in1 - cfg_ch->in1_RangeCenter);
    if(in_err->in1 < cfg_ch->in1_RangeCenter) {
        op1 *= cfg_ch->in1_scaleLow;
    } else {
        op1 *= cfg_ch->in1_scaleHigh;
    }

    // Суммируем
    *out = op0 + op1 + cfg_ch->out_RangeCenter;

    // Если на выходе суммируются градусы
    // сигнал на выходе должен быть в диапазоне -180..180 deg
    if (cfg_ch->degEn) {
        while (*out < -180.0f) *out += 360.0f;
        while (*out >  180.0f) *out -= 360.0f;
    } else {
        if(*out < cfg_ch->out_RangeLow) *out = cfg_ch->out_RangeLow;
        if(*out > cfg_ch->out_RangeHigh) *out = cfg_ch->out_RangeHigh;
    }
    return true;
}
