#pragma once

#include <stdbool.h>
#include <native/task.h>

// Максимальный размер блока данных, передаваемый между сервисом и клиентом
#define MAX_TRANSFER_BLOCK 256

typedef struct
{
    bool connected;
    RT_TASK task_i2c_service;

    RT_TASK_MCB request_open_block;
    RT_TASK_MCB request_data_block;
    RT_TASK_MCB response_data_block;

    char response_buf[MAX_TRANSFER_BLOCK];

} i2c_service_t;

typedef enum
{
    res_successfully = 0,
    res_error_write_to_i2c = -1001,
    res_error_ioctl = -1002

} i2c_resp_status;

bool connect_i2c_service(i2c_service_t* service);
void disconnect_i2c_service(i2c_service_t* service);
int open_i2c(i2c_service_t* service, char* bus_name);
int read_i2c(i2c_service_t* service, int session_id, char dev_id, char port, int len_requested_data, char** ret_data, int* ret_len);
int close_i2c(i2c_service_t* service, int* file_d);
