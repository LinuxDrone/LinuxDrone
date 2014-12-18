#include <sys/mman.h>
#include <getopt.h>
#include "test-sender-receiver.helper.h"

module_test_sender_receiver_t* m_module;

int main (int argc, char *argv[])
{
    mlockall(MCL_CURRENT|MCL_FUTURE);

    setvbuf(stdout, NULL, _IONBF, 0);

    const char* short_options = "h";
    const struct option long_options[] = {
        {"help",no_argument,NULL,'h'},
        {NULL,0,NULL,0}
    };

    int res;
    int option_index;
    opterr=0;
    while ((res=getopt_long(argc,argv,short_options, long_options,&option_index))!=-1){

        switch(res){
            case 'h':
                usage(argv);
            break;

            case '?': default:
                //printf("Found unknown option test-sender main\n");
            break;
        }
    }

    m_module = test_sender_receiver_create(NULL);

    test_sender_receiver_init(m_module, argc, argv);

    params_test_sender_receiver_t* obj = (params_test_sender_receiver_t*)m_module->module_info.specific_params;
    //print_params_test_sender_receiver(obj);

    test_sender_receiver_start(m_module);

    printf("\nPress ENTER for exit\n\n");
    getchar();

    return 0;
}


void test_sender_receiver_run (module_test_sender_receiver_t *module)
{
    long last_print_time = rt_timer_read();
    int count_reads=0;
    long print_period = rt_timer_ns2ticks(1000000000);

    int cycle=0;
    while(1) {
        get_input_data(&module->module_info);

        // проверим, обновились ли данные
        if(module->module_info.updated_input_properties!=0)
        {
            // есть новые данные
            count_reads++;
            if(rt_timer_read() - last_print_time > print_period)
            {
                printf("count_reads=%i\n", count_reads);

                last_print_time = rt_timer_read();
                count_reads=0;
            }
        }
        else
        {
            // вышел таймаут
        }

        input_t* input = (input_t*)module->module_info.input_data;

//printf("input->in1 = %f\n", input->in1);

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
