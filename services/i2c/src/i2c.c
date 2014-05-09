#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

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


int m_file=0;
unsigned int dev_id=0;

void run_task_i2c (void *module)
{
    data_request_i2c_t* request;

    while(1)
    {
//printf("before receive\n");
        int flowid = rt_task_receive(&request_block, TM_INFINITE);
printf("flowid=%i\n",flowid);
        if(flowid<0)
        {
            printf("Function: run_task_i2c, print_task_receive_error:");
            print_task_receive_error(flowid);
            rt_task_sleep(rt_timer_ns2ticks(200000000));
            continue;
        }
        //response_block.flowid=flowid;

        switch (request_block.opcode)
        {
            case op_open_i2c:
                m_file = open(request_block.data, O_RDWR);
                if (m_file < 0) {
                    printf("Failed to open the bus (\" %s \")", request_block.data);
                    m_file = 0;
                }
                //response_block.data=NULL;
                response_block.size=0;
                response_block.opcode = m_file;
                break;


            case op_data_request_i2c:
                request = (data_request_i2c_t*)request_block.data;

printf("receivet data request session_id %i:\n", request->session_id);

                if(dev_id!=request->dev_id)
                {
                    printf("before ioctl dev=%i\n", request->dev_id);
                    int err = ioctl(request->session_id, I2C_SLAVE, request->dev_id);
                    if(err<0)
                    {
                        printf("ioctl error:%i\n", err);
                    }
                    dev_id=request->dev_id;
                }

                printf("before write port=0x%02X len=%i\n", request->port, sizeof(request->port));
                int len = write(request->session_id, &request->port, sizeof(request->port));
                if(len!=sizeof(request->port))
                {
                    printf("error write to i2c port=0x%02X writed=%i\n", request->port, len);
                    //response_block.data = NULL;
                    response_block.size=0;
                }
                else
                {
                    printf("read from i2c file=%i, len_requested_data=%i response_block.data=0x%08X\n", request->session_id, request->len_requested_data, response_block.data);
                    response_block.size = read(request->session_id, response_block.data, request->len_requested_data);
                    char* ddd = response_block.data;
printf("readed size=%i mpuId = 0x%02X\n", response_block.size, *ddd);
                }
                break;


            case op_close_i2c:
                close(request->session_id);
                break;


            default:
                printf("Unknown request to i2c service: %i\n", request_block.opcode);
                break;
        }


        int err = rt_task_reply(flowid, &response_block);
        if(err!=0)
        {
            printf("Function: run_task_i2c, print_task_reply_error:");
            print_task_reply_error(err);
        }
    }
}




int main(int argc, char **argv)
{
    mlockall(MCL_CURRENT|MCL_FUTURE);

    // Подготовим буфер для приема запроса
    request_block.data = malloc(256);

    // Подготовим буфер для отправки ответа
    response_block.data = malloc(256);


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


