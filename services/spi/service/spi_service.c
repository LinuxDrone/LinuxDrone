#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include "spi_service.h"
#include "../client/spi_client.h"

// Структура содержащая информацию о полученном дескрипторе на шину
// и параметрах передачи данных по шине
typedef struct
{
    // Имя шины. Имя файла устройства spi
    char* bus_name;

    // Дескриптор файла шины
    int m_file;

    // Параметры для настройки режима предачи данных по шине
    unsigned int speed_hz;
    unsigned char bits_per_word;
    unsigned short delay_usecs;

} bus_info_t;

// Список открытых шин
bus_info_t** bus_list=NULL;
// Длина списка открытых шин
int bus_list_len;


RT_TASK task_spi;
int priority_task_spi = 99;

RT_TASK_MCB request_block;
RT_TASK_MCB response_block;

char response_block_buf[MAX_TRANSFER_BLOCK];
char request_block_buf[MAX_TRANSFER_BLOCK];

/**
 * @brief open_bus Открываем шину spi
 * Если файл уже был открыт, выводим сообщение о попытке повторного открытия шины
 * @param bus_name Имя файла девайса, скорость передачи данных, число бит в слове, задержка после передачи
 * @return Индекс шины из списка открытых шин
 */
int open_bus(open_request_spi_t* request)
{
    int idx_bus = 0;
    // Найдем данные ассоциированные с инстансом подписчика
    // Если таковых не зарегистрировано, то создадим (выделим память и заполним) необходимые структуры
    bus_info_t* bus_info=NULL;
    if(bus_list!=NULL)
    {
        for(idx_bus=0;idx_bus<bus_list_len;idx_bus++)
        {
            bus_info = bus_list[idx_bus];
            if(strcmp(bus_info->bus_name, request->bus_name)==0)
                break;
            else
                bus_info = NULL;
        }
    }

    if(bus_info==NULL)
    {
        // Если инфа о шине вообще не зарегистрирована в списке, сделаем это
        bus_list_len += 1;
        bus_list = realloc(bus_list, sizeof(bus_info_t*)*bus_list_len);
        bus_info = calloc(1, sizeof(bus_info_t));

        bus_info->bus_name = malloc(strlen(request->bus_name)+1);
        strcpy(bus_info->bus_name, request->bus_name);
        bus_info->m_file = open(bus_info->bus_name, O_RDWR);
        if (bus_info->m_file < 0) {
            fprintf(stderr, "Failed to open the bus (\" %s \")", bus_info->bus_name);
            bus_info->m_file = 0;
        }
        // Сохраняем параметры для настройки режима шины
        bus_info->speed_hz      = request->cfg_transfer_spi.speed_hz;
        bus_info->bits_per_word = request->cfg_transfer_spi.bits_per_word;
        bus_info->delay_usecs   = request->cfg_transfer_spi.delay_usecs;
    
        bus_list[bus_list_len-1] = bus_info;
    }
    else
    {
        // Указанное устройство шины уже зарегистрировано
        fprintf(stderr, "The device file is already open on the bus: %s \n", bus_info->bus_name);
        //return res_error_already_open_bus;
    }

    // Идентификатор сессии
    // Возвращаем индекс шины из списка открытых шин
    return idx_bus;
}


/**
 * @brief close_bus Закрывает файл
 * @param idx_bus Индекс шины из списка открытых шин
 */
void close_bus(int idx_bus)
{
    if(bus_list==NULL || idx_bus > bus_list_len || idx_bus < 0)
        return;

    bus_info_t* bus_info = bus_list[idx_bus];

    if(bus_info==NULL)
        return;

    close(bus_info->m_file);
    free(bus_info->bus_name);
    free(bus_info);

    // Удалим из массива
    remove_element(bus_list, idx_bus, bus_list_len);  /* First shift the elements, then reallocate */
    bus_info_t** tmp = realloc(bus_list, (bus_list_len - 1) * sizeof(bus_info_t*) );
    if (tmp == NULL && bus_list_len > 1) {
       /* No memory available */
       fprintf(stderr, "Function \"close_bus\" No memory available\n");
       exit(EXIT_FAILURE);
    }
    bus_list_len--;
    bus_list = tmp;
}



void run_task_spi (void *module)
{
    data_request_spi_t* request;
    struct spi_ioc_transfer spi_transfer;

    int ioctl_err=0;

    while(1)
    {
        request_block.size=MAX_TRANSFER_BLOCK;
        int flowid = rt_task_receive(&request_block, TM_INFINITE);
        if(flowid<0)
        {
            fprintf(stderr, "Function: run_task_spi, print_task_receive_error:");
            print_task_receive_error(flowid);
            rt_task_sleep(rt_timer_ns2ticks(200000000));
            continue;
        }

        response_block.size=0;
        response_block.opcode=res_successfully;
        ioctl_err=0;
        switch (request_block.opcode)
        {
            case op_open_spi:
            {
                open_request_spi_t* request;
                request = (open_request_spi_t*)request_block.data;

                response_block.opcode = open_bus(request);
                response_block.size=0;
            }
            break;


            case op_transfer_spi:
            {
                request = (data_request_spi_t*)request_block.data;
                bus_info_t* bus_info = bus_list[request->session_id];

                spi_transfer.tx_buf         = (unsigned long)&request->transfer_data;
                spi_transfer.rx_buf         = (unsigned long)response_block.data;
                spi_transfer.len            = request->len_requested_data;
                spi_transfer.speed_hz       = bus_info->speed_hz;
                spi_transfer.bits_per_word  = bus_info->bits_per_word;
                spi_transfer.delay_usecs    = bus_info->delay_usecs;

                // Синхронно пишем и читаем данные
                ioctl_err = ioctl(bus_info->m_file, SPI_IOC_MESSAGE(1), &spi_transfer);

                if(ioctl_err<0)
                {
                    response_block.opcode=res_error_ioctl;
                }
                else
                {
                    if(response_block.opcode==res_successfully)
                    {
                        //memcpy((caddr_t)spi_transfer.rx_buf, response_block.data, spi_transfer.len);
                        response_block.size = request->len_requested_data;
                    }
                }
            }
            break;


            case op_close_spi:
                close_bus(*(int*)request_block.data);
                break;


            default:
                fprintf(stderr, "Unknown request to spi service: %i\n", request_block.opcode);
                break;
        }


        int err = rt_task_reply(flowid, &response_block);
        if(err!=0)
        {
            fprintf(stderr, "Function: run_task_spi, print_task_reply_error:");
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


    int err = rt_task_spawn(&task_spi, TASK_NAME_SPI, TASK_STKSZ, priority_task_spi, TASK_MODE, &run_task_spi, NULL);
    if (err != 0)
        fprintf(stderr, "Error start service task\n");


    fprintf(stderr, "\nPress ENTER for exit\n\n");
    getchar();

    rt_task_delete(&task_spi);

    return 0;
}


