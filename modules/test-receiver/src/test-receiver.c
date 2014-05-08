#include "../include/test_receiver.helper.h"
#include "../../../services/i2c/include/i2c.h"

RT_TASK task_i2c_service;
bool binded_task_i2c_service = false;

RT_TASK_MCB request_open_block;
int session_id=0;
RT_TASK_MCB request_data_block;
RT_TASK_MCB response_data_block;


/**
 * @brief open_i2c Открывает сессию с шиной i2c
 * @param bus_name Имя шины. Имя файла девайса i2c
 * @return  > 0 - Идентификатор сессии
 *          ==0 - Недоступен сервис i2c. Следует пытаться снова открыть сессию
 *          < 0 - Ошибка. Распечатать ошибку можно при помощи функции print_task_bind_error
 */
int open_i2c(char* bus_name)
{
    if(!binded_task_i2c_service)
    {
        int err = rt_task_bind(&task_i2c_service, TASK_NAME_I2C, TM_NONBLOCK);
        if(err!=0)
        {
            return 0;
        }
        else
        {
            binded_task_i2c_service=true;
        }
    }

    request_open_block.data = bus_name;
    request_open_block.size = strlen(bus_name);

    ssize_t received = rt_task_send(&task_i2c_service, &request_data_block, &response_data_block, TM_INFINITE);
    if(received<0)
    {
        if(received==-ESRCH)
        {
            binded_task_i2c_service=false;
            return 0;
        }
        else
        {
            return received;
        }
    }

    // return session id
    return response_data_block.opcode;
}



/**
 * @brief read_i2c Считываеи блок данных с девайса i2c по указанному порту
 * @param request Данные специфицирующие запрос
 * @param data    Считанные с девайса i2c данные
 * @return  > 0 - Длина считанных данных
 *          ==0 - Недоступен сервис i2c. Следует снова открыть сессию
 *          < 0 - Ошибка. Распечатать ошибку можно при помощи функции print_task_send_error
 */
int read_i2c(data_request_i2c_t* request, char** data)
{
    request_data_block.data = (caddr_t)request;

    ssize_t received = rt_task_send(&task_i2c_service, &request_data_block, &response_data_block, TM_INFINITE);
    if(received<0)
    {
        if(received==-ESRCH)
        {
            binded_task_i2c_service=false;
            return 0;
        }
        else
        {
            return received;
        }
    }

    *data = response_data_block.data;

    return response_data_block.size;
}


void test_receiver_run (module_test_receiver_t *module)
{
    // Подготовим буфер для передачи запроса открытия сессии
    request_open_block.opcode = op_open_i2c;

    // Подготовим буфер для передачи запроса на данные
    request_data_block.size = sizeof(data_request_i2c_t);
    request_data_block.opcode = op_data_request_i2c;

    // Подготовим буфер для приема считываемых по i2c данных
    response_data_block.data = malloc(256);


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


        if(session_id<1)
        {
            session_id = open_i2c("/dev/");
            if(session_id<0)
            {
                print_task_bind_error(session_id);
            }
            continue;
        }


        char* data;
        data_request_i2c_t request;
        int res = read_i2c(&request, &data);
        if(res<0)
        {
            print_task_send_error(res);
        }



        // Скажем какие данные следует добыть из разделяемой памяти, если они не придут через трубу
        module->module_info.refresh_input_mask = in1 | in2 | in3 | in4;

        // Наглое считывание данных из разделяемой памяти
        //int res = refresh_input(module);

        cycle++;
    }
}
