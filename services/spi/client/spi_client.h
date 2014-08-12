#pragma once
#include <stdbool.h>
#include <native/task.h>


// Максимальный размер блока данных, передаваемый между сервисом и клиентом
#define MAX_TRANSFER_BLOCK 256

/**
  * Структура сервиса spi
  * Содержит служебную информацию, необходимую функциям для работы с сервисом
  */
typedef struct
{
    bool connected;
    RT_TASK task_spi_service;

    RT_TASK_MCB request_open_block;
    RT_TASK_MCB request_data_block;
    RT_TASK_MCB response_data_block;

    // Буфер служит для временного размещения данных при передаче сервису
    char tx_buf[MAX_TRANSFER_BLOCK];
    // Буфер служит для приема данных от сервиса
    char rx_buf[MAX_TRANSFER_BLOCK];

} spi_service_t;



/**
  * Коды возврата, при вызове функции сервиса
  */
typedef enum
{
    res_successfully = 0,
    res_error_write_to_spi = -1001,
    res_error_ioctl = -1002,
    res_error_already_open_bus = -1003,
    res_error_not_session_id = -1004

} spi_resp_status;



/**
 * @brief connect_spi_service Устанавливает соединение с сервисом SPI
 * @param service Указатель на структуру сервиса
 * @return TRUE в случае успешного соединения
 */
bool connect_spi_service(spi_service_t* service);



/**
 * @brief disconnect_spi_service Разрывает соединение с сервисом spi
 * @param service
 */
void disconnect_spi_service(spi_service_t* service);



/**
 * @brief open_spi Открывает сессию с шиной spi
 * @param service Указатель на структуру сервиса
 * @param bus_name Имя шины. Имя файла девайса spi
 * @return  > 0 - Идентификатор сессии
 *          ==0 - Недоступен сервис spi. Следует пытаться снова открыть сессию
 *          < 0 - Ошибка. Распечатать ошибку можно при помощи функции print_task_send_error
 */
int open_spi(spi_service_t* service, const char* bus_name, unsigned int speed_hz, unsigned char bits_per_word, unsigned short delay_usecs);


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
int transfer_spi(spi_service_t* service, int session_id, char* tx_buff, char* rx_buff, char len_transfer);


/**
 * @brief read_byte_spi Записывает один байт команд в порт устройства spi,
 *        и возвращает один байт с этого устройства
 * @param service Указатель на структуру сервиса
 * @param session_id Идентификатор сессии
 * @param dev_register Внутренний порт устройства на шине spi
 * @param ret_data Указатель на возвращаемые данные
 * @return < 0 - Ошибка.
 */
int read_byte_spi(spi_service_t* service, int session_id, char dev_register, char* ret_data);

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
int read_bytes_spi(spi_service_t* service, int session_id, char dev_register, char* ret_data, char read_len);


/**
 * @brief write_byte_spi Записывает один байт в указанное устройство spi, в указанный порт
 * @param service Указатель на структуру сервиса
 * @param session_id Идентификатор сессии
 * @param dev_register Внутренний порт устройства на шине spi
 * @param data Записываемый байт данных
 * @return < 0 - Ошибка.
 */
int write_byte_spi(spi_service_t* service, int session_id, char dev_register, char data);


/**
 * @brief close_spi Закрывает сессию взаимодействия с шиной spi
 * @param service Указатель на структуру сервиса
 * @param session_id Идентификатор сессии
 * @return
 */
int close_spi(spi_service_t* service, int* session_id);

/**
 * @brief print_spi_error Выводит в stdout ошибку возвращаемую функциями read_spi или write_spi
 * @param err Номер ошибки (отрицательное число)
 */
void print_spi_error(int err);
