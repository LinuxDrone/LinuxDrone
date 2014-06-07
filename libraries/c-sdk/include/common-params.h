#pragma once

// TODO: Структура должна автогенерится из json определения общих для всех модулей параметров
typedef struct
{
    int Task_Priority;

    RTIME Transfer_task_period;

    RTIME Task_Period;

}common_params_t;

