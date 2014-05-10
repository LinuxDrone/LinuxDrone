#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include <syslog.h>
#include <sys/time.h>
#include <unistd.h>

#include "i2c_client.h"
#include "../service/i2c_service.h"



bool connect_i2c_service(i2c_service_t* service)
{
    if(service->connected)
        return true;

    int err = rt_task_bind(&service->task_i2c_service, TASK_NAME_I2C, TM_NONBLOCK);
    if(err!=0)
    {
        return false;
    }
    service->connected=true;

    // Подготовим буфер для передачи запроса открытия сессии
    service->request_open_block.opcode = op_open_i2c;

    // Подготовим буфер для передачи запроса на данные
    service->request_data_block.size = sizeof(data_request_i2c_t);
    service->request_data_block.opcode = op_data_request_i2c;

    // Подготовим буфер для приема считываемых по i2c данных
    service->response_data_block.data = service->response_buf;

    return true;
}


void disconnect_i2c_service(i2c_service_t* service)
{
    rt_task_unbind(&service->task_i2c_service);
}


/**
 * @brief open_i2c Открывает сессию с шиной i2c
 * @param bus_name Имя шины. Имя файла девайса i2c
 * @return  > 0 - Идентификатор сессии
 *          ==0 - Недоступен сервис i2c. Следует пытаться снова открыть сессию
 *          < 0 - Ошибка. Распечатать ошибку можно при помощи функции print_task_send_error
 */
int open_i2c(i2c_service_t* service, char* bus_name)
{
    service->request_open_block.data = bus_name;
    service->request_open_block.size = strlen(bus_name);
    service->request_open_block.opcode = op_open_i2c;
    service->response_data_block.size = MAX_TRANSFER_BLOCK;
    ssize_t received = rt_task_send(&service->task_i2c_service, &service->request_open_block, &service->response_data_block, TM_INFINITE);

    if(received<0)
    {
        if(received==-ESRCH)
        {
            service->connected=false;
            return 0;
        }
        else
        {
            return received;
        }
    }

    // return session id
    return service->response_data_block.opcode;
}



/**
 * @brief read_i2c Считываеи блок данных с девайса i2c по указанному порту
 * @param request Данные специфицирующие запрос
 * @param data    Считанные с девайса i2c данные
 * @return  > 0 - Длина считанных данных
 *          ==0 - Недоступен сервис i2c. Следует снова открыть сессию
 *          < 0 - Ошибка. Распечатать ошибку можно при помощи функции print_task_send_error
 */
int read_i2c(i2c_service_t* service, int session_id, char dev_id, char port, int len_requested_data, char** ret_data, int* ret_len)
{
    data_request_i2c_t request;
    request.dev_id = dev_id;
    request.port = port;
    request.len_requested_data = len_requested_data;
    request.session_id = session_id;

    service->request_data_block.data = (caddr_t)&request;
    service->request_data_block.size = sizeof(data_request_i2c_t);
    service->response_data_block.size = MAX_TRANSFER_BLOCK;

    ssize_t received = rt_task_send(&service->task_i2c_service, &service->request_data_block, &service->response_data_block, TM_INFINITE);
    if(received<0)
    {
        if(received==-ESRCH)
        {
            service->connected=false;
            return 0;
        }
        else
        {
            return received;
        }
    }

    *ret_data = service->response_data_block.data;
    *ret_len = service->response_data_block.size;

    return service->response_data_block.opcode;
}


int close_i2c(i2c_service_t* service, int* session_id)
{
    service->request_open_block.opcode = op_close_i2c;
    service->request_open_block.data = (caddr_t)session_id;
    service->request_open_block.size = sizeof(int);
    service->response_data_block.size = MAX_TRANSFER_BLOCK;

    ssize_t received = rt_task_send(&service->task_i2c_service, &service->request_open_block, &service->response_data_block, TM_INFINITE);
    if(received<0)
    {
        if(received==-ESRCH)
        {
            service->connected=false;
            return 0;
        }
        else
        {
            return received;
        }
    }

    *session_id=0;

    return service->response_data_block.size;
}
