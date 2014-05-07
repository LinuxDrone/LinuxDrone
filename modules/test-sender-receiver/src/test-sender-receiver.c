#include "../include/test_sender_receiver.helper.h"

void test_sender_receiver_run (module_test_sender_receiver_t *module)
{
    int cycle=0;
    while(1) {
        get_input_data(module);

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
        objOutput1->out1 = input->in1*2;
        objOutput1->out2 = input->in1*3+cycle;
        checkin_Output1(module, &objOutput1);

        Output2_t* objOutput2;
        checkout_Output2(module, &objOutput2);
        objOutput2->out3 = cycle;
        checkin_Output2(module, &objOutput2);

        // Скажем какие данные следует добыть из разделяемой памяти, если они не придут через трубу
        module->module_info.refresh_input_mask = in1 | in2;

        // Наглое считывание данных из разделяемой памяти
        //int res = refresh_input(module);

        cycle++;
    }
}
