#include "ap-motors-matrix.helper.h"

extern "C" {
void ap_motors_matrix_run (module_ap_motors_matrix_t *module)
{
    int cycle=0;
    while(1) {
        get_input_data((module_t*)module);

        // проверим, обновились ли данные
        if(module->module_info.updated_input_properties!=0)
        {
            // есть новые данные
        }
        else
        {
            // вышел таймаут
        }

        input_t* input = (input_t*)module->module_info.input_data;

        Output1_t* objOutput1;
        checkout_Output1(module, &objOutput1);
        /*
        objOutput1->char_out = input->in1*2+cycle;
        objOutput1->short_out = input->in1*3+cycle;
        objOutput1->int_out = input->in1*4+cycle;
        objOutput1->long_out = input->in1*5+cycle;
        objOutput1->long_long_out = input->in1*6+cycle;
        objOutput1->float_out = input->in1*7+cycle;
        objOutput1->double_out = input->in1*8+cycle;
        char buffer_string_out [32];
        snprintf(buffer_string_out, 32, "data: %d", cycle);
        objOutput1->string_out = buffer_string_out;
*/
        checkin_Output1(module, &objOutput1);


        // Скажем какие данные следует добыть из разделяемой памяти, если они не придут через трубу
        //module->module_info.refresh_input_mask = in1 | in2;

        // Принудительное считывание данных из разделяемой памяти
        //int res = refresh_input(module);

        cycle++;
    }
}

void ap_motors_matrix_command (ap_motors_matrix_command_t type_command, void* params)
{
    switch (type_command)
    {
    case cmd_command0:
        break;

    case cmd_command1:
        break;

    case cmd_command2:
        break;

    default:
        printf("ap_motors_matrix_command. Unknown command: %i.\n", type_command);
    }
}
}
