#pragma once

#include <bson.h>

// TODO: Структура должна автогенерится из json определения общих для всех модулей параметров
typedef struct
{
    int Task_Priority;

    long long  Transfer_task_period;

    long long  Task_Period;

}common_params_t;


int common_params2bson(common_params_t* obj, bson_t* bson);
int bson2common_params(void* module, bson_t* bson);
void print_common_params(common_params_t* obj);
