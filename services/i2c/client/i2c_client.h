#pragma once
#include <stdbool.h>
#include <native/task.h>


// Максимальный размер блока данных, передаваемый между сервисом и клиентом
#define MAX_TRANSFER_BLOCK 256


/**
  * Структура сервиса i2c
  * Содержит служебную информацию, необходимую функциям для работы с сервисом
  */
typedef struct
{
    bool connected;
    RT_TASK task_i2c_service;

    RT_TASK_MCB request_open_block;
    RT_TASK_MCB request_data_block;
    RT_TASK_MCB response_data_block;

    // Буфер служит для приема данных от сервиса, и для временного размещения данных при передаче сервису
    char data_buf[MAX_TRANSFER_BLOCK];

} i2c_service_t;



/**
  * Коды возврата, при вызове функция сервиса
  */
typedef enum
{
    res_successfully = 0,
    res_error_write_to_i2c = -1001,
    res_error_ioctl = -1002

} i2c_resp_status;



/**
 * @brief connect_i2c_service Устанавливает соединение с сервисом I2C
 * @param service Указатель на структуру сервиса
 * @return TRUE в случае успешного соединения
 */
bool connect_i2c_service(i2c_service_t* service);



/**
 * @brief disconnect_i2c_service Разрывает соединение с сервисом i2c
 * @param service
 */
void disconnect_i2c_service(i2c_service_t* service);



/**
 * @brief open_i2c Открывает сессию с шиной i2c
 * @param service Указатель на структуру сервиса
 * @param bus_name Имя шины. Имя файла девайса i2c
 * @return  > 0 - Идентификатор сессии
 *          ==0 - Недоступен сервис i2c. Следует пытаться снова открыть сессию
 *          < 0 - Ошибка. Распечатать ошибку можно при помощи функции print_task_send_error
 */
int open_i2c(i2c_service_t* service, const char* bus_name);



/**
 * @brief read_i2c Считывает блок данных с указанного девайса i2c, запрашиваемой длины,
 * начиная с указанного внутреннего порта девайса.
 * @param service Указатель на структуру сервиса
 * @param session_id Идентификатор сессии
 * @param dev_id Адрес девайса на шине i2c
 * @param dev_register Внутренний порт девайса
 * @param len_requested_data Запрашиваемая длина считываемых данных (необходимо считать)
 * @param ret_data Блок считанных данных
 * @param ret_len Длина считанных данных (фактически считано)
 * @return < 0 - Ошибка. Распечатать ошибку можно при помощи функции print_task_send_error
 */
int read_i2c(i2c_service_t* service, int session_id, char dev_id, char dev_register, int len_requested_data, char** ret_data, int* ret_len);

/**
 * @brief read_raw_i2c Считывает блок данных с указанного устройства i2c, запрашиваемой длины,
 * без указания внутреннего порта устройства.
 * @param service Указатель на структуру сервиса
 * @param session_id Идентификатор сессии
 * @param dev_id Адрес девайса на шине i2c
 * @param len_requested_data Запрашиваемая длина считываемых данных (необходимо считать)
 * @param ret_data Блок считанных данных
 * @param ret_len Длина считанных данных (фактически считано)
 * @return < 0 - Ошибка. Распечатать ошибку можно при помощи функции print_task_send_error
 */
int read_raw_i2c(i2c_service_t* service, int session_id, char dev_id, int len_requested_data, char** ret_data, int* ret_len);


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
int write_i2c(i2c_service_t* service, int session_id, char dev_id, char dev_register, int len_data, char* data);

/**
 * @brief write_raw_i2c Записывает блок данных в указанное устройство i2с, без указания порта
 * @param service Указатель на структуру сервиса
 * @param session_id Идентификатор сессии
 * @param dev_id Адрес девайса на шине i2c
 * @param len_data Длина блока записываемых данных
 * @param data Указатель на блок записываемых данных
 * @return < 0 - Ошибка.
 */
int write_raw_i2c(i2c_service_t* service, int session_id, char dev_id, int len_data, char* data);

/**
 * @brief close_i2c Закрывает сессию взаимодействия с шиной i2c
 * @param service Указатель на структуру сервиса
 * @param session_id Идентификатор сессии
 * @return
 */
int close_i2c(i2c_service_t* service, int* session_id);

/**
 * @brief print_i2c_error Выводит в stdout ошибку возвращаемую функциями read_i2c или write_i2c
 * @param err Номер ошибки (отрицательное число)
 */
void print_i2c_error(int err);
