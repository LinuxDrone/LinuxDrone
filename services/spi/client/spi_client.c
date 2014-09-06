#include "spi_client.h"
#include "../service/spi_service.h"



/**
 * @brief connect_spi_service Устанавливает соединение с сервисом SPI
 * @param service Указатель на структуру сервиса
 * @return TRUE в случае успешного соединения
 */
bool connect_spi_service(spi_service_t* service)
{
    if(service->connected)
        return true;

    int err = rt_task_bind(&service->task_spi_service, TASK_NAME_SPI, TM_NONBLOCK);
    if(err!=0)
    {
        return false;
    }
    service->connected=true;

    // Подготовим буфер для передачи запроса открытия сессии
    service->request_open_block.opcode = op_open_spi;

    // Подготовим буфер для приема считываемых по spi данных
    service->response_data_block.data = service->rx_buf;

    return true;
}



/**
 * @brief disconnect_spi_service Разрывает соединение с сервисом spi
 * @param service
 */
void disconnect_spi_service(spi_service_t* service)
{
    rt_task_unbind(&service->task_spi_service);
}



/**
 * @brief open_spi Открывает сессию с шиной spi
 * @param service Указатель на структуру сервиса
 * @param bus_name Имя шины. Имя файла девайса spi
 * @param speed_hz Имя шины. Тактовая частота передачи данных по шине spi
 * @param bits_per_word Число бит в передаваемом слове по шине spi
 * @param delay_usecs Добавочная задержка после транзакции на шине spi
 * @return  >= 0 - Идентификатор сессии
 *          <  0 - Ошибка. Распечатать ошибку можно при помощи функции print_task_send_error
 *          == res_error_not_session_id  - Недоступен сервис spi. Следует пытаться снова открыть сессию
 */
int open_spi(spi_service_t* service, const char* bus_name, unsigned int speed_hz, unsigned char bits_per_word, unsigned short delay_usecs)
{
    if(strlen(bus_name) > MAX_SIZE_BUS_NAME)
    {
        fprintf(stderr, "Error bus name > MAX_SIZE_BUS_NAME\n");
        return -1;
    }

    open_request_spi_t request;

    strcpy(request.bus_name, (caddr_t)bus_name);

    request.cfg_transfer_spi.speed_hz       = speed_hz;
    request.cfg_transfer_spi.bits_per_word  = bits_per_word;
    request.cfg_transfer_spi.delay_usecs    = delay_usecs;

    service->request_open_block.data = (caddr_t)&request;
    service->request_open_block.size = sizeof(open_request_spi_t);
    service->request_open_block.opcode = op_open_spi;
    service->response_data_block.size = MAX_TRANSFER_BLOCK;

    ssize_t received = rt_task_send(&service->task_spi_service, &service->request_open_block, &service->response_data_block, TM_INFINITE);

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
 * @brief transfer_spi Синхронно записывает и считывает данные с устройства на шине spi,
 *                      данные в массивах размещать согласно протоколу обмена с устройством
 * @param service Указатель на структуру сервиса
 * @param session_id Идентификатор сессии
 * @param tx_buff Указатель на массив отправляемых данных
 * @param rx_buff Указатель на массив принятых данных
 * @param transfer_len Число отправляемых - переданных байт в передаче
 * @return < 0 - Ошибка.
 */
int transfer_spi(spi_service_t* service, int session_id, char* tx_buff, char* rx_buff, char len_transfer)
{
    data_request_spi_t* request = (data_request_spi_t*)service->tx_buf;
    request->session_id=session_id;
    request->len_requested_data = len_transfer;

    memcpy(&request->transfer_data, tx_buff, len_transfer);

    service->request_data_block.data = service->tx_buf;
    service->request_data_block.size = sizeof(data_request_spi_t) + len_transfer - sizeof(char);
    service->request_data_block.opcode = op_transfer_spi;

    service->response_data_block.size = MAX_TRANSFER_BLOCK;

    ssize_t received = rt_task_send(&service->task_spi_service, &service->request_data_block, &service->response_data_block, TM_INFINITE);

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

    memcpy(rx_buff, service->response_data_block.data, len_transfer);

    return service->response_data_block.opcode;
}



/**
 * @brief read_byte_spi Записывает один байт команды в порт устройства spi,
 *        и возвращает один байт с этого устройства
 * @param service Указатель на структуру сервиса
 * @param session_id Идентификатор сессии
 * @param dev_register Внутренний порт устройства на шине spi
 * @param ret_data Указатель на возвращаемые данные
 * @return < 0 - Ошибка.
 */
int read_byte_spi(spi_service_t* service, int session_id, char dev_register, char* ret_data)
{
    data_request_spi_t* request = (data_request_spi_t*)service->tx_buf;
    request->session_id=session_id;
    request->transfer_data = dev_register;
    request->len_requested_data = 2;

    service->request_data_block.data = service->tx_buf;
    service->request_data_block.size = sizeof(data_request_spi_t);
    service->request_data_block.opcode = op_transfer_spi;

    service->response_data_block.size = MAX_TRANSFER_BLOCK;

    ssize_t received = rt_task_send(&service->task_spi_service, &service->request_data_block, &service->response_data_block, TM_INFINITE);

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

    *ret_data = service->response_data_block.data[1];

    return service->response_data_block.opcode;
}


/**
 * @brief read_bytes_spi Записывает один байт команды в порт устройства spi,
 *        и возвращает указатель на буфер с прочитанными данными с этого устройства
 * @param service Указатель на структуру сервиса
 * @param session_id Идентификатор сессии
 * @param dev_register Внутренний порт устройства на шине spi
 * @param ret_data Указатель на массив со считанными данными
 * @param read_len Число байт для чтения
 * @return < 0 - Ошибка.
 */
int read_bytes_spi(spi_service_t* service, int session_id, char dev_register, char* ret_data, char read_len)
{
    data_request_spi_t* request = (data_request_spi_t*)service->tx_buf;
    request->session_id=session_id;
    request->transfer_data = dev_register;
    request->len_requested_data = read_len + 1;

    service->request_data_block.data = service->tx_buf;
    service->request_data_block.size = sizeof(data_request_spi_t);
    service->request_data_block.opcode = op_transfer_spi;

    service->response_data_block.size = MAX_TRANSFER_BLOCK;

    ssize_t received = rt_task_send(&service->task_spi_service, &service->request_data_block, &service->response_data_block, TM_INFINITE);

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

    memcpy(ret_data, service->response_data_block.data + 1, read_len);

    return service->response_data_block.opcode;
}



/**
 * @brief write_byte_spi Записывает один байт в указанное устройство spi, в указанный порт
 * @param service Указатель на структуру сервиса
 * @param session_id Идентификатор сессии
 * @param dev_register Внутренний порт устройства на шине spi
 * @param data Записываемый байт данных
 * @return < 0 - Ошибка.
 */
int write_byte_spi(spi_service_t* service, int session_id, char dev_register, char data)
{
    data_request_spi_t* request = (data_request_spi_t*)service->tx_buf;
    request->session_id=session_id;
    request->transfer_data = dev_register;
    request->len_requested_data = 2;

    memcpy(&request->transfer_data + 1, &data, 1);

    service->request_data_block.data = service->tx_buf;
    service->request_data_block.size = sizeof(data_request_spi_t) + sizeof(char);
    service->request_data_block.opcode = op_transfer_spi;

    service->response_data_block.size = MAX_TRANSFER_BLOCK;

    ssize_t received = rt_task_send(&service->task_spi_service, &service->request_data_block, &service->response_data_block, TM_INFINITE);
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
 * @brief close_spi Закрывает сессию взаимодействия с шиной spi
 * @param service Указатель на структуру сервиса
 * @param session_id Идентификатор сессии
 * @return
 */
int close_spi(spi_service_t* service, int* session_id)
{
    service->request_open_block.opcode = op_close_spi;
    service->request_open_block.data = (caddr_t)session_id;
    service->request_open_block.size = sizeof(int);
    service->response_data_block.size = MAX_TRANSFER_BLOCK;

    ssize_t received = rt_task_send(&service->task_spi_service, &service->request_open_block, &service->response_data_block, TM_INFINITE);
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



void print_spi_error(int err)
{
    switch (err)
    {
    case res_error_write_to_spi:
        fprintf(stderr, "Error write to spi device\n");
        break;

    case res_error_ioctl:
        fprintf(stderr, "ioctl error:%i\n", err);
        break;

    default:
        print_task_send_error(err);
        break;
    }
}
