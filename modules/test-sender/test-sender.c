#include "test_sender.helper.h"

void test_sender_run (module_test_sender_t *module)
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


        Output1_t* objOutput1;
        checkout_Output1(module, &objOutput1);
        objOutput1->char_out = cycle;
        objOutput1->short_out = cycle*2;
        objOutput1->int_out = cycle*3;
        objOutput1->long_out = cycle*4;
        objOutput1->long_long_out = cycle*6;
        objOutput1->float_out = cycle*7;
        objOutput1->double_out = cycle*8;

        char buffer [33];
        printf ("Enter a number: ");
        itoa (objOutput1->short_out,buffer,10);
        objOutput1->string_out = buffer;

        checkin_Output1(module, &objOutput1);

        Output2_t* objOutput2;
        checkout_Output2(module, &objOutput2);
        objOutput2->out3 = cycle*4;
        checkin_Output2(module, &objOutput2);

        cycle++;
    }
}
