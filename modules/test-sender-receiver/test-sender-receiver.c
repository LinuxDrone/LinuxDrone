#include "test-sender-receiver.helper.h"


void test_sender_receiver_run (module_test_sender_receiver_t *module)
{
#ifdef XENO
	long last_print_time = rt_timer_read();
	long print_period = rt_timer_ns2ticks(1000000000);
#else
	long last_print_time = apr_time_now();
	long print_period = 1000000;
#endif
	
    int count_reads=0;
	

    int cycle=0;
    while(1) {
#ifndef XENO
        //apr_sleep(module->module_info.common_params.main_task_period);
#endif   

        get_input_data(&module->module_info);

        // проверим, обновились ли данные
        if(module->module_info.updated_input_properties!=0)
        {
            // есть новые данные
            count_reads++;
#ifdef XENO
			if (rt_timer_read() - last_print_time > print_period)
#else
			if (apr_time_now() - last_print_time > print_period)
#endif            
            {
                printf("count_reads=%i\n", count_reads);
#ifdef XENO
				last_print_time = rt_timer_read();
#else
				last_print_time = apr_time_now();
#endif            
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
