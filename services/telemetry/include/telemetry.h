#pragma once

#ifdef XENO
#include "../../../libraries/sdk/include/module-functions.h"
#else
#include "../../../libraries/sdk/include/apr-module-functions.h"
#endif

#include <bcon.h>

typedef struct
{
    // Имя инстанса на данные которого оформляется подписка
    char* instance_name;

    // Имя выходной группы портов инстанса
    char* out_name;

} subscription_t;


typedef struct
{
    // Длина массива подписок
    int len_subscription;
    // Массив подписок
    subscription_t** subscription;
} connection_t;
