#pragma once

#include "../../../libraries/c-sdk/include/module-functions.h"

#define TASK_NAME_I2C "i2c_service_task"

typedef struct
{
    // Длина массива подписок
    int len_subscription;
    // Массив подписок

} i2c_req_t;


typedef struct
{
    // Длина массива подписок
    int len_subscription;
    // Массив подписок

} i2c_res_t;
