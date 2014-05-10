#include "../include/test_receiver.helper.h"
#include "../../../services/i2c/client/i2c_client.h"



void test_receiver_run (module_test_receiver_t *module)
{
    i2c_service_t i2c_service;
    memset(&i2c_service, 0, sizeof(i2c_service_t));
    int session_id=0;

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


        if(!i2c_service.connected)
        {
            connect_i2c_service(&i2c_service);
            continue;
        }


        if(session_id<1)
        {
            session_id = open_i2c(&i2c_service, "/dev/i2c-1");
            continue;
        }




        char* data;
        char dev = 0x68;
        char port = 0x75;
        int len_requested_data = 1;
        int ret_len;

        int res = read_i2c(&i2c_service, session_id, dev, port, len_requested_data, &data, &ret_len);
        if(res<0)
        {
            switch (res)
            {
            case res_error_write_to_i2c:
                printf("Error wtite to i2c device\n");
                break;

            case res_error_ioctl:
                printf("ioctl error:%i\n", res);
                break;

            default:
                print_task_send_error(res);
                break;
            }

        }

        if(ret_len>0)
        {
            printf("mpuId = 0x%02X\n", *data);
        }


        close_i2c(&i2c_service, &session_id);


        // Скажем какие данные следует добыть из разделяемой памяти, если они не придут через трубу
        module->module_info.refresh_input_mask = in1 | in2 | in3 | in4;

        // Наглое считывание данных из разделяемой памяти
        //int res = refresh_input(module);

        cycle++;
    }
}
