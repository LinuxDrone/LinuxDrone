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
void settingsLoad(pidParams_t *pidParam);

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
    double dT;

    //enum  channel ch;

    SRTIME  timeTick, timeTickold;

    settingsLoad(pidParam);

    while(1) {
        get_input_data((module_t *)module);

        if(rt_timer_read() - last_print_time > print_period)
        {
            printEnable = true;
            last_print_time = rt_timer_read();
        }

        timeTick = (SRTIME)rt_timer_read();
        dT = ((float)((rt_timer_ticks2ns(timeTick  - timeTickold)))/1000000000.0f);
        timeTickold = timeTick;

        // проверим, обновились ли данные
        if(module->module_info.updated_input_properties!=0)
        {
            // есть новые данные
            input = (input_t*)module->module_info.input_data;

            in_err[Roll]  = (double)input->errRoll;
            in_err[Pitch] = (double)input->errPitch;
            in_err[Yaw]   = (double)input->errYaw;

            // Запускаем расчет PID для каждого канала.
            uint8_t ch;
            for (ch = 0; ch < 3; ch++) {
                out[ch]= calcPID(pidParam, in_err[ch], dT);
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


        //if(printEnable) printf("pidParam[Yaw].Kp = %f\n",pidParam[Yaw].Kp);
        printEnable = false;
    }
}


/* ---------------  FUNCTIONS -----------------------*/

/**
 * @brief Читает данные из базы MongoDB
 * И сохраняет в структуре attitude_data_t
 */
void settingsLoad(pidParams_t *pidParam)
{
  setPID(&pidParam[Roll],  ROLL_KP, ROLL_KI,ROLL_KD,  ROLL_ILIMIT);
  setPID(&pidParam[Pitch], ROLL_KP, ROLL_KI,ROLL_KD,  ROLL_ILIMIT);
  setPID(&pidParam[Yaw],   ROLL_KP, ROLL_KI,ROLL_KD,  ROLL_ILIMIT);
}

