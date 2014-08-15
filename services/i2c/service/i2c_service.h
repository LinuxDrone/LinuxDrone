#pragma once

#include "../../../libraries/sdk/include/module-functions.h"

#define TASK_NAME_I2C "i2c_service_task"


// Протокол взаимодействия сервиса и клиента
// 1. Клиент посылает запрос открытия сессии open_i2c, передавая имя шины (используется структура open_i2c_t)
// 2. Сервер отвечает числом в возвращаемом статусе (идентификатором сессии. На самом деле это дискриптор файла).
//    Если 0 то операция открытия не удалась.
// 3. Клиент посылает запрос на чтение данных из регистра data_request_i2c (используется структура data_request_i2c_t)
// 4. Сервер возвращает блок бинарных данных
// 5. Клиент уведомляет о завершении сессии close_i2c
// 6. Сервер отвечает пустыми данными

// Тип данных перегоняемый между тасками при синхронном обмене
typedef enum
{
    op_open_i2c,
    op_data_read_i2c,
    op_data_write_i2c,
    op_raw_read_i2c,
    op_raw_write_i2c,
    op_cmd_write_i2c,
    op_close_i2c

} operatation;


// Данные однозначно идентифицирующие место передачи (чтения) данных на шине i2c
// Объединяют адрес девайса на шине и внутренний порт девайса
typedef struct
{
    // Идентификатор сессии
    int session_id;

    // идентификатор девайса на шине
    char dev_id;

    char dev_register;

} address_i2c_t;



// Клиент запрашивает данные, указывая идентификатор девайса на шине, порт и длину данных, которые он желает прочитать
typedef struct
{
    address_i2c_t addr_and_dev_register;

    // длина данных, которые желает получить клиент сервиса
    int len_requested_data;

} data_request_i2c_t;
