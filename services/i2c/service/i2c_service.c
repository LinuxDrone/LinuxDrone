#include <sys/mman.h>
#include <linux/i2c-dev.h>

#include "i2c_service.h"
#include "../client/i2c_client.h"

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

RT_TASK_MCB request_block;
RT_TASK_MCB response_block;

char response_block_buf[MAX_TRANSFER_BLOCK];
char request_block_buf[MAX_TRANSFER_BLOCK];

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
    address_i2c_t* address_i2c;

    int ioctl_err=0;

    while(1)
    {
        request_block.size=MAX_TRANSFER_BLOCK;
        int flowid = rt_task_receive(&request_block, TM_INFINITE);
        if(flowid<0)
        {
            printf("Function: run_task_i2c, print_task_receive_error:");
            print_task_receive_error(flowid);
            rt_task_sleep(rt_timer_ns2ticks(200000000));
            continue;
        }

        response_block.size=0;
        response_block.opcode=res_successfully;
        ioctl_err=0;

        switch (request_block.opcode)
        {
            case op_open_i2c:
                response_block.opcode = open_bus(request_block.data);
                break;


            case op_data_read_i2c:
            {
                request = (data_request_i2c_t*)request_block.data;

                if(current_dev_id!=request->addr_and_dev_register.dev_id)
                {
                    ioctl_err = ioctl(request->addr_and_dev_register.session_id, I2C_SLAVE, request->addr_and_dev_register.dev_id);
                    current_dev_id=request->addr_and_dev_register.dev_id;
                }

                if(ioctl_err<0)
                {
                    response_block.opcode=res_error_ioctl;
                }
                else
                {
                    // Читаем с предварительным указанием порта
                    int len = write(request->addr_and_dev_register.session_id, &request->addr_and_dev_register.dev_register, sizeof(char));
                    if(len!=sizeof(request->addr_and_dev_register.dev_register))
                    {
                        response_block.opcode=res_error_write_to_i2c;
                    }

                    if(response_block.opcode==res_successfully)
                    {
                        response_block.size = read(request->addr_and_dev_register.session_id, response_block.data, request->len_requested_data);
                    }
                }
            }
            break;

            case op_raw_read_i2c:
            {
                request = (data_request_i2c_t*)request_block.data;

                if(current_dev_id!=request->addr_and_dev_register.dev_id)
                {
                    ioctl_err = ioctl(request->addr_and_dev_register.session_id, I2C_SLAVE, request->addr_and_dev_register.dev_id);
                    current_dev_id=request->addr_and_dev_register.dev_id;
                }

                if(ioctl_err<0)
                {
                    response_block.opcode=res_error_ioctl;
                }
                else
                {
                    // Читаем без указания порта
                    if(response_block.opcode==res_successfully)
                    {
                        response_block.size = read(request->addr_and_dev_register.session_id, response_block.data, request->len_requested_data);
                    }
                }
            }
            break;

            case op_data_write_i2c:
                // В принятых данных, первый байт - адрес девайса на шине
                // Второй байт - номер порта
                // Остальные данные - на передачу в порт
                address_i2c = (address_i2c_t*)request_block.data;

                if(current_dev_id!=address_i2c->dev_id)
                {
                    ioctl_err = ioctl(address_i2c->session_id, I2C_SLAVE, address_i2c->dev_id);
                    current_dev_id=address_i2c->dev_id;
                }
                if(ioctl_err<0)
                {
                    response_block.opcode=res_error_ioctl;
                }
                else
                {
                    int size_for_write = request_block.size - sizeof(address_i2c_t);

                    int writen=0;
                    // Первым записываемым байтом в поток уходит номер порта
                    size_for_write += sizeof(char); // Увеличим для этого длину передаваемых данных
                    writen = write(address_i2c->session_id, &address_i2c->dev_register, size_for_write);

                    if(writen!=size_for_write)
                    {
                        response_block.opcode=res_error_write_to_i2c;
                    }
                }
            break;

            case op_raw_write_i2c:
                // В принятых данных, первый байт - адрес устройства на шине
                // Остальные данные - на передачу в устройство без указания порта
                address_i2c = (address_i2c_t*)request_block.data;

                if(current_dev_id!=address_i2c->dev_id)
                {
                    ioctl_err = ioctl(address_i2c->session_id, I2C_SLAVE, address_i2c->dev_id);
                    current_dev_id=address_i2c->dev_id;
                }

                if(ioctl_err<0)
                {
                    response_block.opcode=res_error_ioctl;
                }
                else
                {
                    int size_for_write = request_block.size - sizeof(address_i2c_t);

                    int writen=0;
                    // Не пишем номер порта. только сырые данные
                    writen = write(address_i2c->session_id, request_block.data + sizeof(address_i2c_t), size_for_write);

                    if(writen!=size_for_write)
                    {
                        response_block.opcode=res_error_write_to_i2c;
                    }
                }
            break;

            case op_close_i2c:
                close_bus(*(int*)request_block.data);
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
    request_block.data = request_block_buf;

    // Подготовим буфер для отправки ответа
    response_block.data = response_block_buf;


    int err = rt_task_spawn(&task_i2c, TASK_NAME_I2C, TASK_STKSZ, priority_task_i2c, TASK_MODE, &run_task_i2c, NULL);
    if (err != 0)
        printf("Error start service task\n");


    printf("\nPress ENTER for exit\n\n");
    getchar();

    rt_task_delete(&task_i2c);

    return 0;
}


