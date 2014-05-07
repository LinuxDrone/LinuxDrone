#include "../include/test_receiver.helper.h"
#include "../../../services/i2c/include/i2c.h"

RT_TASK task_i2c_service;
bool binded_task_i2c_service = false;

RT_TASK_MCB request_block;
RT_TASK_MCB response_block;

void test_receiver_run (module_test_receiver_t *module)
{
    // Подготовим буфер для передачи запроса
    request_block.data = calloc(1, sizeof(i2c_req_t));
    request_block.size = sizeof(i2c_req_t);

    // Подготовим буфер для приема ответа
    response_block.data = calloc(1, sizeof(i2c_res_t));
    response_block.size = sizeof(i2c_res_t);


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
            int err = rt_task_bind	(&task_i2c_service, TASK_NAME_I2C, TM_INFINITE);
            if(err!=0)
            {
                rt_task_sleep(rt_timer_ns2ticks(100000000));
                // TODO: print error

                continue;
            }
            else
            {
                binded_task_i2c_service=true;
            }
        }


        ssize_t received = rt_task_send(&task_i2c_service, &request_block, &response_block, TM_INFINITE);
        if(received<0)
        {
            //TODO: print error
            continue;
        }


        // Скажем какие данные следует добыть из разделяемой памяти, если они не придут через трубу
        module->module_info.refresh_input_mask = in1 | in2 | in3 | in4;

        // Наглое считывание данных из разделяемой памяти
        //int res = refresh_input(module);

        cycle++;
    }
}
