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

#include "../include/i2c.h"


// Структура содержащая информацию о количестве подписчиков на шину
typedef struct
{
    // Имя шины. Имя файла девайса i2c
    char* bus_name;

    // Кол-во клиентов
    char count_clients;

    // Дескриптор файла шины
    int m_file;

} bus_info_t;

// Список открытых шин
bus_info_t** bus_list=NULL;
// Длина списка открытых шин
int bus_list_len;


RT_TASK task_i2c;
int priority_task_i2c = 99;

RT_TASK task_i2c_service;
bool binded_task_i2c_service = false;

RT_TASK_MCB request_block;
RT_TASK_MCB response_block;


// Девайс выбранный для общений на шине в текущий момент
unsigned int current_dev_id=0;


/**
 * @brief open_bus Открываеи шину i2c
 * Если файл уже был открыт, увеличивает счетчик его клиентов
 * @param bus_name Имя файла девайса
 * @return Дескиптор файла шины
 */
int open_bus(const char* bus_name)
{
    // Найдем данные ассоциированные с инстансом подписчика
    // Если таковых не зарегистрировано, то создадим (выделим память и заполним) необходимые структуры
    bus_info_t* bus_info=NULL;
    if(bus_list!=NULL)
    {
        int i;
        for(i=0;i<bus_list_len;i++)
        {
            bus_info = bus_list[i];
            if(strcmp(bus_info->bus_name, bus_name)==0)
                break;
            else
                bus_info = NULL;
        }
    }

    if(bus_info==NULL)
    {
        // Если инфа о шине вообще не зарегестрирована в списке, сделаем это
        bus_list_len += 1;
        bus_list = realloc(bus_list, sizeof(bus_info_t*)*bus_list_len);
        bus_info = calloc(1, sizeof(bus_info_t));

        bus_info->bus_name = malloc(strlen(bus_name)+1);
        strcpy(bus_info->bus_name, bus_name);
printf("open bus %s\n", bus_info->bus_name);
        bus_info->m_file = open(bus_info->bus_name, O_RDWR);
        if (bus_info->m_file < 0) {
            printf("Failed to open the bus (\" %s \")", bus_info->bus_name);
            bus_info->m_file = 0;
        }

        bus_list[bus_list_len-1] = bus_info;
    }

    // Зарегистрируем линк
    bus_info->count_clients++;

    return bus_info->m_file;
}


/**
 * @brief close_bus Закрывает файл, если количество подписчиков станет равнфм нулю
 * @param m_file Дескиптор файла
 */
void close_bus(int m_file)
{
    if(bus_list==NULL || m_file==0)
        return;

    bus_info_t* bus_info=NULL;

    int i;
    for(i=0;i<bus_list_len;i++)
    {
        bus_info = bus_list[i];
        if(bus_info->m_file==m_file)
            break;
        else
            bus_info = NULL;
    }


    if(bus_info==NULL)
        return;

    bus_info->count_clients--;

    if(bus_info->count_clients>0)
        return;


    close(bus_info->m_file);
    current_dev_id=0;
    free(bus_info->bus_name);
    free(bus_info);

    // Удалим из массива
    remove_element(bus_list, i, bus_list_len);  /* First shift the elements, then reallocate */
    bus_info_t** tmp = realloc(bus_list, (bus_list_len - 1) * sizeof(bus_info_t*) );
    if (tmp == NULL && bus_list_len > 1) {
       /* No memory available */
       fprintf(stderr, "Function \"close_bus\" No memory available\n");
       exit(EXIT_FAILURE);
    }
    bus_list_len--;
    bus_list = tmp;
}



void run_task_i2c (void *module)
{
    data_request_i2c_t* request;

    while(1)
    {
printf("before receive\n");
        request_block.size=256;
        int flowid = rt_task_receive(&request_block, TM_INFINITE);
printf("flowid=%i\n",flowid);
        if(flowid<0)
        {
            printf("Function: run_task_i2c, print_task_receive_error:");
            print_task_receive_error(flowid);
            rt_task_sleep(rt_timer_ns2ticks(200000000));
            continue;
        }

        switch (request_block.opcode)
        {
            case op_open_i2c:
printf("open\n");
                response_block.opcode = open_bus(request_block.data);
                response_block.size=0;
                break;


            case op_data_request_i2c:
printf("read\n");
                request = (data_request_i2c_t*)request_block.data;

//printf("receivet data request session_id %i:\n", request->session_id);

                if(current_dev_id!=request->dev_id)
                {
printf("before ioctl dev=%i\n", request->dev_id);
                    int err = ioctl(request->session_id, I2C_SLAVE, request->dev_id);
                    if(err<0)
                    {
                        printf("ioctl error:%i\n", err);
                    }
                    current_dev_id=request->dev_id;
                }

printf("before write port=0x%02X len=%i\n", request->port, sizeof(request->port));
                int len = write(request->session_id, &request->port, sizeof(request->port));
                if(len!=sizeof(request->port))
                {
printf("error write to i2c port=0x%02X writed=%i\n", request->port, len);
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
printf("close\n")            ;
                int* file_d = (int*)request_block.data;
                close_bus(*file_d);
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


