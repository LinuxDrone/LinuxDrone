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

void c_pru_pwmout_run (module_c_pru_pwmout_t *module)
{
    int cycle=0;
    input_t* input;

    while(1) {
        get_input_data(module);

        // проверим, обновились ли данные
        if(module->module_info.updated_input_properties!=0)
        {
            // есть новые данные
            input = (input_t*)module->module_info.input_data;
        }
        else
        {
            // вышел таймаут
        }

        input->pwm1 = 0;
        input->pwm2 =9;

        // Скажем какие данные следует добыть из разделяемой памяти, если они не придут через трубу
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
                pwm11;

        cycle++;
    }
}
