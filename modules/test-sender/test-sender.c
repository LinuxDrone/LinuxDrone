#include <sys/mman.h>
#include "test-sender.helper.h"
#include <getopt.h>

module_test_sender_t* m_module;

int main (int argc, char *argv[])
{
    mlockall(MCL_CURRENT|MCL_FUTURE);
    setvbuf(stdout, NULL, _IONBF, 0);


    m_module = test_sender_create(NULL);

    test_sender_init(m_module, argc, argv);

    params_test_sender_t* obj = (params_test_sender_t*)m_module->module_info.specific_params;
    print_params_test_sender(obj);
    print_common_params(&m_module->module_info.common_params);

    test_sender_start(m_module);

    printf("\nPress ENTER for exit\n\n");
    getchar();

    return 0;
}


void test_sender_run (module_test_sender_t *module)
{
    //printf("params for test_sender\n");
    //print_params_test_sender(&module->params_test_sender);

    int cycle=0;
    while(1) {
        get_input_data(&module->module_info);

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
        objOutput1->short_out = cycle;
        objOutput1->int_out = cycle;
        objOutput1->long_out = cycle;
        objOutput1->long_long_out = cycle;
        objOutput1->float_out = cycle * 0.11;
        objOutput1->double_out = cycle * 0.23;
        char buffer_string_out [32];
        snprintf(buffer_string_out, 32, "data: %i", cycle);
        objOutput1->string_out = buffer_string_out;
//print_Output1(objOutput1);
        checkin_Output1(module, &objOutput1);

        Output2_t* objOutput2;
        checkout_Output2(module, &objOutput2);
        objOutput2->out3 = cycle;
        checkin_Output2(module, &objOutput2);



        cycle++;
    }
}

