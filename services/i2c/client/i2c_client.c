#include "i2c_client.h"
#include "../service/i2c_service.h"



/**
 * @brief connect_i2c_service Устанавливает соединение с сервисом I2C
 * @param service Указатель на структуру сервиса
 * @return TRUE в случае успешного соединения
 */
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

    // Подготовим буфер для приема считываемых по i2c данных
    service->response_data_block.data = service->data_buf;

    return true;
}



/**
 * @brief disconnect_i2c_service Разраывает соединение с сервисом i2c
 * @param service
 */
void disconnect_i2c_service(i2c_service_t* service)
{
    rt_task_unbind(&service->task_i2c_service);
}



/**
 * @brief open_i2c Открывает сессию с шиной i2c
 * @param service Указатель на структуру сервиса
 * @param bus_name Имя шины. Имя файла девайса i2c
 * @return  > 0 - Идентификатор сессии
 *          ==0 - Недоступен сервис i2c. Следует пытаться снова открыть сессию
 *          < 0 - Ошибка. Распечатать ошибку можно при помощи функции print_task_send_error
 */
int open_i2c(i2c_service_t* service, const char* bus_name)
{
    service->request_open_block.data = (caddr_t)bus_name;
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
 * @brief read_i2c Считываеи блок данных с указанного девайса i2c, запрашиваемой длины,
 * начиная с указанного внутреннего порта девайса.
 * @param service Указатель на структуру сервиса
 * @param session_id Идентификатор шины (сессии)
 * @param dev_id Адрес девайса на шине i2c
 * @param dev_register Внутренний порт девайса
 * @param len_requested_data Запрашиваемая длина считываемых данных (необходимо считать)
 * @param ret_data Блок считанных данных
 * @param ret_len Длина считанных данных (фактически считано)
 * @return < 0 - Ошибка. Распечатать ошибку можно при помощи функции print_task_send_error
 */
int read_i2c(i2c_service_t* service, int session_id, char dev_id, char dev_register, int len_requested_data, char** ret_data, int* ret_len)
{
    data_request_i2c_t request;
    request.addr_and_dev_register.dev_id = dev_id;
    request.addr_and_dev_register.dev_register = dev_register;
    request.len_requested_data = len_requested_data;
    request.addr_and_dev_register.session_id = session_id;

    service->request_data_block.data = (caddr_t)&request;
    service->request_data_block.size = sizeof(data_request_i2c_t);
    service->response_data_block.size = MAX_TRANSFER_BLOCK;
    service->request_data_block.opcode = op_data_read_i2c;

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



/**
 * @brief write_i2c Записывает блок данных в указанное устройство i2с, в указанный порт
 * @param service Указатель на структуру сервиса
 * @param session_id Идентификатор сессии
 * @param dev_id Адрес девайса на шине i2c
 * @param dev_register Внутренний порт девайса
 * @param len_data Длина блока записываемых данных
 * @param data Указатель на блок записываемых данных
 * @return < 0 - Ошибка.
 */
int write_i2c(i2c_service_t* service, int session_id, char dev_id, char dev_register, int len_data, char* data)
{
    if(len_data>MAX_TRANSFER_BLOCK-sizeof(address_i2c_t))
    {
        printf("Length of transfered data > MAX_TRANSFER_BLOCK-sizeof(address_i2c_t)\n");
        return -1;
    }

    address_i2c_t* address_i2c = (address_i2c_t*)service->data_buf;
    address_i2c->session_id=session_id;
    address_i2c->dev_id = dev_id;
    address_i2c->dev_register = dev_register;

    memcpy(&address_i2c->dev_register+1, data, len_data);

    service->request_data_block.data = service->data_buf;
    service->request_data_block.size = sizeof(address_i2c_t)+len_data;
    service->response_data_block.size = MAX_TRANSFER_BLOCK;
    service->request_data_block.opcode = op_data_write_i2c;

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

    return service->response_data_block.opcode;
}



/**
 * @brief close_i2c Закрывает сессию взаимодействия с шиной i2c
 * @param service Указатель на структуру сервиса
 * @param session_id Идентификатор сессии
 * @return
 */
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



void print_i2c_error(int err)
{
    switch (err)
    {
    case res_error_write_to_i2c:
        printf("Error wtite to i2c device\n");
        break;

    case res_error_ioctl:
        printf("ioctl error:%i\n", err);
        break;

    default:
        print_task_send_error(err);
        break;
    }
}
