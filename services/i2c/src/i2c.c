#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#include <syslog.h>
#include <sys/time.h>
#include <unistd.h>

//#include <native/task.h>

#include "../include/i2c.h"

//#define TASK_MODE  0  /* No flags */
//#define TASK_STKSZ 0  /* Stack size (use default one) */

RT_TASK task_i2c;
int priority_task_i2c = 99;

RT_TASK task_i2c_service;
bool binded_task_i2c_service = false;

RT_TASK_MCB request_block;
RT_TASK_MCB response_block;


void run_task_i2c (void *module)
{
    while(1)
    {
        int flowid = rt_task_receive(&request_block, TM_INFINITE);
        if(flowid<0)
        {
            print_task_receive_error(flowid);
            continue;
        }


printf("rt_task_receive begin\n");
rt_task_sleep(rt_timer_ns2ticks(200000000));
printf("rt_task_receive end\n");


        int err = rt_task_reply(flowid, &response_block);
        if(err!=0)
        {
            print_task_reply_error(err);
        }
    }
}




int main(int argc, char **argv)
{
    mlockall(MCL_CURRENT|MCL_FUTURE);


    // Подготовим буфер для приема запроса
    request_block.data = calloc(1, sizeof(i2c_req_t));
    request_block.size = sizeof(i2c_req_t);

    // Подготовим буфер для отправки ответа
    response_block.data = calloc(1, sizeof(i2c_res_t));
    response_block.size = sizeof(i2c_res_t);



    int err = rt_task_spawn(&task_i2c, TASK_NAME_I2C, TASK_STKSZ, priority_task_i2c, TASK_MODE, &run_task_i2c, NULL);
    if (err != 0)
        printf("Error start service task\n");



    printf("\nPress ENTER for exit\n\n");
    getchar();

    rt_task_delete(&task_i2c);

    free(request_block.data);
    free(response_block.data);

    return 0;
}


