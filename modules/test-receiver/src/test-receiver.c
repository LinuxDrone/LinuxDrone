#include "../include/test_receiver.helper.h"
#include "../../../services/i2c/include/i2c.h"

RT_TASK task_i2c_service;
bool binded_task_i2c_service = false;

RT_TASK_MCB request_block2;
RT_TASK_MCB response_block2;

void test_receiver_run (module_test_receiver_t *module)
{

    // Подготовим буфер для передачи запроса
    request_block2.data = calloc(1, sizeof(i2c_req_t));
    request_block2.size = sizeof(i2c_req_t);

    // Подготовим буфер для приема ответа
    response_block2.data = calloc(1, sizeof(i2c_res_t));
    response_block2.size = sizeof(i2c_res_t);


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


        if(!binded_task_i2c_service)
        {
            int err = rt_task_bind(&task_i2c_service, TASK_NAME_I2C, TM_NONBLOCK);
            if(err!=0)
            {
                //print_task_bind_error(err);
                continue;
            }
            else
            {
                binded_task_i2c_service=true;
                //rt_task_sleep(rt_timer_ns2ticks(2000));
            }
        }

        ssize_t received = rt_task_send(&task_i2c_service, &request_block2, &response_block2, TM_INFINITE);
        if(received<0)
        {
            if(received==-ESRCH)
            {
                binded_task_i2c_service=false;
            }
            else
            {
                print_task_send_error(received);
                continue;
            }
        }


        // Скажем какие данные следует добыть из разделяемой памяти, если они не придут через трубу
        module->module_info.refresh_input_mask = in1 | in2 | in3 | in4;

        // Наглое считывание данных из разделяемой памяти
        //int res = refresh_input(module);

        cycle++;
    }
}
