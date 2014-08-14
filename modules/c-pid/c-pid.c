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

#include "c-pid.h"
#include "pid.h"
#include "c-pid.helper.h"
#include "math.h"

// Загрузка параметров для PID регуляторов из базы.
void settingsLoad(pidParams_t *pidParam, params_c_pid_t *param);

/**
 * @brief c_pid_run Функция рабочего потока модуля
 * @param module Указатель на структуру модуля
 */
void c_pid_run (module_c_pid_t *module)
{
    bool printEnable = false;

    // Указатель на структуру с входными данными модуля.
    input_t* input;

    long last_print_time = (long)rt_timer_read();
    long print_period = rt_timer_ns2ticks(1000000000);

    // Структура данных, содержащая информацию о параметрах pid модуля
    pidParams_t	pidParam[3];

    // Входные данные модуля
    double in_err[3];
    // Выходные данные модуля
    double out[3];
    double dT = 0.0d;
    float timeElapsed = 0.0f;

    SRTIME  timeTick, timeTickold;

    memset(in_err, 0, 3*sizeof(double));
    memset(out, 0, 3*sizeof(double));

    settingsLoad(pidParam, &module->params_c_pid);

    while(1) {
        get_input_data((module_t *)module);

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

            // Расчет dT
            timeTick = (SRTIME)rt_timer_read();
            dT = ((float)((rt_timer_ticks2ns(timeTick  - timeTickold)))/1000000000.0f);
            timeTickold = timeTick;
            timeElapsed += dT;

            // Чтение и демпфирование входных данных
            in_err[Roll]  = filtered(&pidParam[Roll], in_err[Roll], (double)input->errRoll);
            in_err[Pitch]  = filtered(&pidParam[Pitch], in_err[Pitch], (double)input->errPitch);
            in_err[Yaw]  = filtered(&pidParam[Yaw], in_err[Yaw], (double)input->errYaw);

            // Запускаем расчет PID для каждого канала.
            // Модуль начинает расчитывать PID через 10 сек после запуска
            uint8_t ch;
            for (ch = 0; ch < 3 && timeElapsed > 10.0f; ch++)
            {
                // Проверка на выход за пределы зоны нечувствительности
                if(in_err[ch] > pidParam[ch].dreadband || in_err[ch] < -pidParam[ch].dreadband)
                {
                    // Расчет PID
                        double tmp = calcPID(&pidParam[ch], in_err[ch], dT);

                    // Для режима с накоплением расчитанных воздействий PID сигнала
                    // Нужен для сохранения величины выходного сигнала после компенсации рассогласования по входу
                    // TODO сделать отдельный модуль для этой цели.
                    if(pidParam[ch].limitAccum > 0.00001d)
                    {
                        out[ch] += tmp * dT;
                        if(out[ch] >  pidParam[ch].limitAccum) out[ch] = pidParam[ch].limitAccum;
                        if(out[ch] < -pidParam[ch].limitAccum) out[ch] = -pidParam[ch].limitAccum;

                    } else out[ch] = tmp; // Обычный выход с PID
                }
            }

            outPID_t* outpid;
            checkout_outPID(module, &outpid);
            outpid->roll  = (float)out[Roll];
            outpid->pitch = (float)out[Pitch];
            outpid->yaw   = (float)out[Yaw];
            checkin_outPID(module, &outpid);

        }
        else
        {
            // вышел таймаут
        }

        // Эти данные следует добыть из разделяемой памяти, если они не придут через трубу
        //module->module_info.refresh_input_mask = errRoll | errPitch | errYaw;


        //if(printEnable) print_params_c_pid(&module->params_c_pid);
        printEnable = false;
    }
}


/* ---------------  FUNCTIONS -----------------------*/

/**
 * @brief Читает данные из базы MongoDB
 * И сохраняет в структуре attitude_data_t
 */
void settingsLoad(pidParams_t *pidParam, params_c_pid_t *param)
{
  setPID(&pidParam[Roll],   param->RollKp, param->RollKi,param->RollKd,  param->RollLimit);
  setPID(&pidParam[Pitch],  param->PitchKp, param->PitchKi,param->PitchKd,  param->PitchLimit);
  setPID(&pidParam[Yaw],    param->YawKp, param->YawKi,param->YawKd,  param->YawLimit);

  pidParam[Roll].filteredAlfa   = param->RollFilteredAlfa;
  pidParam[Roll].limitAccum     = param->RollLimitAccum;
  pidParam[Roll].dreadband      = param->RollDeadband;
  pidParam[Pitch].filteredAlfa  = param->PitchFilteredAlfa;
  pidParam[Pitch].limitAccum    = param->PitchLimitAccum;
  pidParam[Pitch].dreadband     = param->PitchDeadband;
  pidParam[Yaw].filteredAlfa    = param->YawFilteredAlfa;
  pidParam[Yaw].limitAccum      = param->YawLimitAccum;
  pidParam[Yaw].dreadband       = param->YawDeadband;
}

