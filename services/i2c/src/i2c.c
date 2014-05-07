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

#include "../include/i2c.h"


RT_TASK task_i2c;
int priority_task_i2c = 99;


void run_task_i2c (void *module)
{
    while(1)
    {
        printf("run_task_i2c\n");
        rt_task_sleep(rt_timer_ns2ticks(500000000));
    }
}


int main(int argc, char **argv)
{
    int err = rt_task_create(&task_i2c, "i2c_service_task", TASK_STKSZ, priority_task_i2c, TASK_MODE);
    if (err != 0)
    {
        fprintf(stdout, "Error create task_i2c \n");
        return err;
    }

    err = rt_task_start(&task_i2c, &run_task_i2c, NULL);
    if (err != 0)
        printf("Error start service task\n");


    printf("\nPress ENTER for exit\n\n");
    getchar();

    return 0;
}


