#include <stdio.h>
#include <native/queue.h>
#include <native/heap.h>
#include <native/event.h>
#include "../include/module-functions.h"


#define TASK_MODE  0  /* No flags */
#define TASK_STKSZ 0  /* Stack size (use default one) */

#define SHMEM_WRITER_MASK	0x7FFFFFFF

#define SHMEM_HEAP_SIZE		300
#define SHMEM_BLOCK1_SIZE	200

/**
 * @brief init_object_set Инициализируети структуру, представляющую выходной объект инстанса в системе.
 * Каждый тип объекта, который порождает модуль, должен иметь некоторый набор сущностей, требуемых ему для передачи данных объекта другим инстансам.
 * Данная функция создает необходимые сущности (очереди, мьютексы и т.д.)
 * @param pset Указатель на структуру, содержащую необхоимые для объекта сущности
 * @param instance_name Имя инстанса объекта
 * @param out_name Имя выходного объекта (имя группы портов, объедененных в логический объект)
 * @return Код ошибки. 0 в случае успеха.
 */
int init_object_set(out_object_t * pset, char* instance_name, char* out_name)
{
    if(strlen(instance_name) > XNOBJECT_NAME_LEN-5)
    {
        fprintf(stdout, "Function init_object_set, Instance name (\"%s\") length (%i) exceeds the maximum length allowed (%i)\n", instance_name, strlen(instance_name), XNOBJECT_NAME_LEN-5);
        return -1;
    }

    // Create shared memory
    char name_shmem[XNOBJECT_NAME_LEN] = "";
    strcat(name_shmem, instance_name);
    strcat(name_shmem, out_name);
    strcat(name_shmem, "_shmem");

    int err = rt_heap_create(&pset->shmem_set.h_shmem, name_shmem, pset->shmem_set.shmem_len,
                             H_SHARED | H_PRIO);
    if (err != 0) {
        fprintf(stdout, "Error %i create shared memory \"%s\"\n", err, name_shmem);
        return err;
    }

    // Alloc shared memory block
    err = rt_heap_alloc(&pset->shmem_set.h_shmem, 0, TM_INFINITE, &pset->shmem_set.shmem);
    if (err != 0)
        printf("Error rt_heap_alloc for block1 err=%i\n", err);
    memset(pset->shmem_set.shmem, 0, pset->shmem_set.shmem_len);

    // Create event service
    char name_eflags[XNOBJECT_NAME_LEN] = "";
    strcat(name_eflags, instance_name);
    strcat(name_eflags, out_name);
    strcat(name_eflags, "_flags");
    err = rt_event_create(&pset->shmem_set.eflags, name_eflags, ULONG_MAX, EV_PRIO);
    if (err != 0) {
        fprintf(stdout, "Error create event service \"%s\"\n", name_eflags);
        return err;
    }

    // Create mutex for read shared memory
    char name_rmutex[XNOBJECT_NAME_LEN] = "";
    strcat(name_rmutex, instance_name);
    strcat(name_rmutex, out_name);
    strcat(name_rmutex, "_rmutex");
    err = rt_mutex_create(&pset->shmem_set.mutex_read_shmem, name_rmutex);
    if (err != 0) {
        fprintf(stdout, "Error create mutex_read_shmem \"%s\"\n", name_rmutex);
        return err;
    }

    return 0;
}



/**
 * @brief Функция добавляет линк в список линков, связывающих данный модуль с удаленным модулем
 * Подготавливает структуры необходимые для работы с данными линками (мапинг свойств) в риалтайме
 * @return
 */
int register_out_link(out_object_t* out_object, const char* subscriber_instance_name, unsigned short offset_field, TypeFieldObj type_field_obj, const char* remote_field_name, remote_queue_t* remote_queue)
{
    // Найдем данные ассоциированные с инстансом подписчика
    // Если таковых не зарегистрировано, то создадим (выделим память и заполним) необходимые структуры
    out_queue_set_t* out_queue_set=NULL;
    if(out_object->out_queue_sets!=NULL)
    {
        int i;
        for(i=0;i<out_object->out_queue_sets_len;i++)
        {
            out_queue_set = out_object->out_queue_sets[i];
            if(strcmp(out_queue_set->out_queue->name_instance, subscriber_instance_name)==0)
                break;
            else
                out_queue_set = NULL;
        }
    }

    if(out_queue_set==NULL)
    {
        // Если инстанс вообще не зарегестрирован в списке потребителей нашего модуля, сделаем это
        out_object->out_queue_sets_len += 1;
        out_object->out_queue_sets = realloc(out_object->out_queue_sets, sizeof(out_queue_set_t*)*out_object->out_queue_sets_len);
        out_queue_set = calloc(1, sizeof(out_queue_set_t));
        out_queue_set->out_queue = remote_queue;
        out_object->out_queue_sets[out_object->out_queue_sets_len-1] = out_queue_set;
    }

    // Зарегистрируем линк
    out_queue_set->len += 1;
    out_queue_set->remote_obj_fields = realloc(out_queue_set->remote_obj_fields, sizeof(remote_obj_field_t*)*out_queue_set->len);
    remote_obj_field_t* remote_obj_field = calloc(1, sizeof(remote_obj_field_t));
    remote_obj_field->offset_field_obj = offset_field;
    remote_obj_field->remote_field_name = malloc(strlen(remote_field_name)+1);
    strcpy(remote_obj_field->remote_field_name, remote_field_name);
    remote_obj_field->type_field_obj = type_field_obj;
    out_queue_set->remote_obj_fields[out_queue_set->len-1] = remote_obj_field;

    return 0;
}


shmem_in_set_t* register_remote_shmem(module_t* module, const char* name_remote_instance, const char* name_remote_outgroup)
{
    if(module==NULL)
    {
        printf("Function \"register_remote_shmem\" null parameter module\n");
        return NULL;
    }

    int i=0;
    for(i=0;i<module->remote_shmems_len;i++)
    {
        shmem_in_set_t* info_remote_shmem = module->remote_shmems[i];
        if(strcmp(info_remote_shmem->name_instance, name_remote_instance)==0 && strcmp(info_remote_shmem->name_outgroup, name_remote_outgroup)==0)
        {
            // Разделяемая память уже зарегистрирована
            return info_remote_shmem;
        }
    }

    // Разделяеимая память не зарегистрирована и set следует создать и сохранить на него ссылку в массиве.
    module->remote_shmems_len +=1;
    module->remote_shmems = realloc(module->remote_shmems, sizeof(shmem_in_set_t*)*module->remote_shmems_len);
    shmem_in_set_t* new_remote_shmem = calloc(1, sizeof(shmem_in_set_t));

    new_remote_shmem->name_instance = malloc(strlen(name_remote_instance)+1);
    strcpy(new_remote_shmem->name_instance, name_remote_instance);

    new_remote_shmem->name_outgroup = malloc(strlen(name_remote_outgroup)+1);
    strcpy(new_remote_shmem->name_outgroup, name_remote_outgroup);

    module->remote_shmems[module->remote_shmems_len-1] = new_remote_shmem;

    return new_remote_shmem;
}



/**
 * @brief Фунция проверяет, зарегистрирована ли ссылка на очередь инстанса потребителя (входная очередь модуля потребителя)
 * Если таковой нет, то создается очередь и ссылка на нее сохраняется в массиве
 * @param module Массив хранящий ссылки на входные очереди модулей подписчиков
 * @param name_remote_queue Имя входной очереди модуля подписчика
 */
remote_queue_t* register_remote_queue(module_t* module, const char* name_remote_instance)
{
    if(module==NULL)
    {
        printf("Function \"register_remote_queue\" null parameter module\n");
        return NULL;
    }

    int i=0;
    for(i=0;i<module->remote_queues_len;i++)
    {
        remote_queue_t* info_remote_queue = module->remote_queues[i];
        if(strcmp(info_remote_queue->name_instance, name_remote_instance)==0)
        {
            // Очередь уже зарегистрирована
            return info_remote_queue;
        }
    }

    // Очередь не зарегистрирована и ее следует создать и сохранить на нее ссылку в массиве.
    module->remote_queues_len +=1;
    module->remote_queues = realloc(module->remote_queues, sizeof(remote_queue_t*)*module->remote_queues_len);
    remote_queue_t* new_remote_queue = calloc(1, sizeof(remote_queue_t));
    new_remote_queue->name_instance = malloc(strlen(name_remote_instance)+1);
    strcpy(new_remote_queue->name_instance, name_remote_instance);
    module->remote_queues[module->remote_queues_len-1] = new_remote_queue;

    return new_remote_queue;
}


/**
 * @brief init Инициализация инстанса модуля
 * Принимает на вход bson объект, контент которого является конфигураций инстанса
 * @param module
 * @param data Указатель на блок данных, содержащий контент bson объекта
 * @param length Длина блока данных, содержащего контент bson объекта
 * @return
 */
int init(module_t* module, const uint8_t * data, uint32_t length)
{
    bson_t bson;
    bson_init_static(&bson, data, length);

    //debug_print_bson("Function \"init\" module-functions.c", &bson);

    // Вытаскиваем из конфигурации значения обязательных настроечных параметров
    /**
     * Instance name
     */
    bson_iter_t iter;
    if (!bson_iter_init_find(&iter, &bson, "instance")) {
        printf("Not found property \"instance\"");
        return -1;
    }
    if (!BSON_ITER_HOLDS_UTF8(&iter)) {
        printf("Property \"instance\" not UTF8 type");
        return -1;
    }
    module->instance_name = bson_iter_utf8(&iter, NULL);
    // Проверяем, не превышает ли длина имени инстанса значения 32-5=27
    // Т.к. максимальная длина имен объектов ксеномая равна 32 а 5 символов могут быть использованы на суфикс,
    // при формировании имен тасков, очередй и пр., необходимых для объетов ксеномая используемых инстансом в работе
    if(strlen(module->instance_name) > XNOBJECT_NAME_LEN-5)
    {
        fprintf(stdout, "Instance name (\"%s\") length (%i) exceeds the maximum length allowed (%i)\n", module->instance_name, strlen(module->instance_name), XNOBJECT_NAME_LEN-5);
        return -1;
    }
    //fprintf(stdout, "instance name=%s\n\n", module->instance_name);

    /**
     * Task Priority
     */
    if (!bson_iter_init_find(&iter, &bson, "Task Priority")) {
        printf("Not found property \"Task Priority\"");
        return -1;
    }
    if (!BSON_ITER_HOLDS_INT32(&iter)) {
        printf("Property \"Task Priority\" not INT32 type");
        return -1;
    }
    module->task_priority = bson_iter_int32(&iter);
    //fprintf(stdout, "Task Priority=%i\n", module->task_priority);

    /**
     * Main task Period
     */
    if (!bson_iter_init_find(&iter, &bson, "Task Period")) {
        printf("Not found property \"Task Period\"");
        return -1;
    }
    if (!BSON_ITER_HOLDS_INT32(&iter)) {
        printf("Property \"Main task Period\" not INT32 type");
        return -1;
    }
    module->queue_timeout = rt_timer_ns2ticks(bson_iter_int32(&iter) * 1000);
    //fprintf(stdout, "queue_timeout=%i\n", module->queue_timeout);

    /**
     * Transfer task Period
     */
    if (!bson_iter_init_find(&iter, &bson, "Transfer task period")) {
        printf("Not found property \"Transfer task period\"");
        return -1;
    }
    if (!BSON_ITER_HOLDS_INT32(&iter)) {
        printf("Property \"Main task Period\" not INT32 type");
        return -1;
    }
    // Умножаем на тысячу потому, что время в конфиге указывается в микросекундах, а функция должна примать на вход наносекунды
    module->transmit_task_period = rt_timer_ns2ticks(bson_iter_int32(&iter) * 1000);
    //fprintf(stdout, "transmit_task_period=%i\n", module->transmit_task_period);


    // Выделяем память под структуры, представляющие связи с модулями подписчиками
    // Связи через очередь (данный модуль поставщик, другие потреьители данных)
    // Список исходящих связей, должен быть в массиве "out_links" в объекте конфигурации
    // но его может не быть, если модуль не имеет вызодов
    if (bson_iter_init_find(&iter, &bson, "out_links"))
    {
        if (!BSON_ITER_HOLDS_ARRAY(&iter)) {
            printf("Property \"out_links\" not ARRAY type");
            return -1;
        }

        const uint8_t *array_buf = NULL;
        uint32_t array_buf_len = 0;
        bson_t bson_queue_links;
        bson_iter_array(&iter, &array_buf_len, &array_buf);
        bson_init_static(&bson_queue_links, array_buf, array_buf_len);

        uint32_t count_links = bson_count_keys (&bson_queue_links);
        //printf("count_links=%i\n", count_links);

        bson_iter_t iter_links;
        if(!bson_iter_init (&iter_links, &bson_queue_links))
        {
            fprintf(stderr, "Error: error create iterator for queue links\n");
            return -1;
        }

        const uint8_t *link_buf = NULL;
        uint32_t link_buf_len = 0;
        bson_t bson_out_link;
        while(bson_iter_next(&iter_links))
        {
            if(!BSON_ITER_HOLDS_DOCUMENT(&iter_links))
            {
                fprintf(stderr, "Function: init, Error: iter_links not a out link document\n");
                continue;
            }
            bson_iter_document(&iter_links, &link_buf_len, &link_buf);
            bson_init_static(&bson_out_link, link_buf, link_buf_len);

            debug_print_bson("Function \"init\" module-functions.c inside while", &bson_out_link);

            // Получим имя инстанса модуля подписчика
            bson_iter_t iter_subscriber_instance_name;
            if (!bson_iter_init_find(&iter_subscriber_instance_name, &bson_out_link, "inInst")) {
                printf("Not found property \"inInst\" in bson_out_link");
                return -1;
            }
            if (!BSON_ITER_HOLDS_UTF8(&iter_subscriber_instance_name)) {
                printf("Property \"inInst\" in bson_out_link not UTF8 type");
                return -1;
            }
            const char* subscriber_instance_name = bson_iter_utf8(&iter_subscriber_instance_name, NULL);

            // Добавим имя инстанса подписчика и ссылку на объект его очереди (если оно не было зафиксировано раньше, то будут созданы необходимые структуры для его хранения)
            remote_queue_t* remote_queue = register_remote_queue(module, subscriber_instance_name);

            // Получим название выходного пина данного модуля
            bson_iter_t iter_outpin_name;
            if (!bson_iter_init_find(&iter_outpin_name, &bson_out_link, "outPin")) {
                printf("Not found property \"outPin\" in bson_out_link");
                return -1;
            }
            if (!BSON_ITER_HOLDS_UTF8(&iter_outpin_name)) {
                printf("Property \"outPin\" in bson_out_link not UTF8 type");
                return -1;
            }
            const char* outpin_name = bson_iter_utf8(&iter_outpin_name, NULL);


            // Получим название входного пина модуля получателя
            bson_iter_t iter_remote_inpin_name;
            if (!bson_iter_init_find(&iter_remote_inpin_name, &bson_out_link, "inPin")) {
                printf("Not found property \"inPin\" in bson_out_link");
                return -1;
            }
            if (!BSON_ITER_HOLDS_UTF8(&iter_remote_inpin_name)) {
                printf("Property \"inPin\" in bson_out_link not UTF8 type");
                return -1;
            }
            const char* remote_inpin_name = bson_iter_utf8(&iter_remote_inpin_name, NULL);


            unsigned short offset_field;
            unsigned short index_port;
            out_object_t* out_object = (*module->get_outobj_by_outpin)(module, outpin_name, &offset_field, &index_port);
            if(out_object)
            {
                //fieldInteger; //TODO: обрабатывать и другие типы
                register_out_link(out_object, subscriber_instance_name, offset_field, fieldInteger, remote_inpin_name, remote_queue);
            }
            else
            {
                printf("Not found OUT PIN \"%s\" in instance \"%s\"\n", outpin_name, module->instance_name);
            }
        }
    }
    else
    {
        //printf("Not found property \"out_links\" in configuration of instance \"%s\" which have outputs\n", module->instance_name);
        //debug_print_bson("Function \"init\" module-functions.c on error", &bson);
    }


    // Выделяем память под структуры, представляющие связи с модулями поставщиками
    // Связи через очередь (данный модуль поставщик, другие потреьители данных)
    // Список исходящих связей, должен быть в массиве "out_links" в объекте конфигурации
    // но его может не быть, если модуль не имеет вызодов
    if (bson_iter_init_find(&iter, &bson, "in_links"))
    {
        if (!BSON_ITER_HOLDS_ARRAY(&iter)) {
            printf("Property \"in_links\" not ARRAY type");
            return -1;
        }

        const uint8_t *array_buf = NULL;
        uint32_t array_buf_len = 0;
        bson_t bson_queue_links;
        bson_iter_array(&iter, &array_buf_len, &array_buf);
        bson_init_static(&bson_queue_links, array_buf, array_buf_len);

        //uint32_t count_links = bson_count_keys (&bson_queue_links);
        //printf("count_links=%i\n", count_links);

        bson_iter_t iter_links;
        if(!bson_iter_init (&iter_links, &bson_queue_links))
        {
            fprintf(stderr, "Error: error create iterator for queue links\n");
            return -1;
        }

        const uint8_t *link_buf = NULL;
        uint32_t link_buf_len = 0;
        bson_t bson_in_link;
        while(bson_iter_next(&iter_links))
        {
            if(!BSON_ITER_HOLDS_DOCUMENT(&iter_links))
            {
                fprintf(stderr, "Function: init, Error: iter_links not a out link document\n");
                continue;
            }
            bson_iter_document(&iter_links, &link_buf_len, &link_buf);
            bson_init_static(&bson_in_link, link_buf, link_buf_len);

            debug_print_bson("Function \"init\" module-functions.c inside while", &bson_in_link);

            // Получим имя инстанса модуля поставщика
            bson_iter_t iter_publisher_instance_name;
            if (!bson_iter_init_find(&iter_publisher_instance_name, &bson_in_link, "outInst")) {
                printf("Not found property \"outInst\" in bson_out_link");
                return -1;
            }
            if (!BSON_ITER_HOLDS_UTF8(&iter_publisher_instance_name)) {
                printf("Property \"outInst\" in bson_out_link not UTF8 type");
                return -1;
            }
            const char* publisher_instance_name = bson_iter_utf8(&iter_publisher_instance_name, NULL);


            // Получим имя нруппы пинов инстанча поставщика
            bson_iter_t iter_publisher_nameOutGroup;
            if (!bson_iter_init_find(&iter_publisher_nameOutGroup, &bson_in_link, "nameOutGroup")) {
                printf("Not found property \"nameOutGroup\" in bson_out_link");
                return -1;
            }
            if (!BSON_ITER_HOLDS_UTF8(&iter_publisher_nameOutGroup)) {
                printf("Property \"nameOutGroup\" in bson_out_link not UTF8 type");
                return -1;
            }
            const char* publisher_nameOutGroup = bson_iter_utf8(&iter_publisher_nameOutGroup, NULL);


            // Добавим имя инстанса подписчика и ссылку на объект его очереди (если оно не было зафиксировано раньше, то будут созданы необходимые структуры для его хранения)
            shmem_in_set_t* remote_shmem = register_remote_shmem(module, publisher_instance_name, publisher_nameOutGroup);


            // Получим название выходного пина инстанса поставщика
            bson_iter_t iter_outpin_name;
            if (!bson_iter_init_find(&iter_outpin_name, &bson_in_link, "outPin")) {
                printf("Not found property \"outPin\" in bson_out_link");
                return -1;
            }
            if (!BSON_ITER_HOLDS_UTF8(&iter_outpin_name)) {
                printf("Property \"outPin\" in bson_out_link not UTF8 type");
                return -1;
            }
            const char* remote_out_pin_name = bson_iter_utf8(&iter_outpin_name, NULL);


            // Получим название входного пина данного модуля
            bson_iter_t iter_remote_inpin_name;
            if (!bson_iter_init_find(&iter_remote_inpin_name, &bson_in_link, "inPin")) {
                printf("Not found property \"inPin\" in bson_out_link");
                return -1;
            }
            if (!BSON_ITER_HOLDS_UTF8(&iter_remote_inpin_name)) {
                printf("Property \"inPin\" in bson_out_link not UTF8 type");
                return -1;
            }
            const char* input_pin_name = bson_iter_utf8(&iter_remote_inpin_name, NULL);


            t_mask input_port_mask = (*module->get_inmask_by_inputname)(input_pin_name);
            if(input_port_mask)
            {
                remote_shmem->assigned_input_ports_mask |= input_port_mask;

                remote_shmem->remote_out_pin_name = malloc(strlen(remote_out_pin_name)+1);
                strcpy(remote_shmem->remote_out_pin_name, remote_out_pin_name);

                remote_shmem->input_pin_name = malloc(strlen(input_pin_name)+1);
                strcpy(remote_shmem->input_pin_name, input_pin_name);
            }
            else
            {
                printf("Not found INPUT PIN \"%s\" in instance \"%s\"\n", input_pin_name, module->instance_name);
            }
        }
    }
    else
    {
        //printf("Not found property \"out_links\" in configuration of instance \"%s\" which have outputs\n", module->instance_name);
        //debug_print_bson("Function \"init\" module-functions.c on error", &bson);
    }


    return 0;
}


/**
 * @brief write_shmem копирует блок данных (data) в разделяемую память, определенную в set
 * @param set структура, содержащая все необходимые данные для работы с блоком разделяемой памяти
 * @param data данные копируемые в разделяемую память
 * @param datalen длина копируемых данных
 */
void write_shmem(shmem_out_set_t* set, const char* data, unsigned short datalen)
{
    unsigned long after_mask;
    int res = rt_event_clear(&set->eflags, ~SHMEM_WRITER_MASK, &after_mask);
    if (res != 0)
    {
        printf("error write_shmem: rt_event_clear1\n");
        return;
    }
    //printf("was mask = 0x%08X\n", after_mask);

    /**
     * \~russian Подождем, пока все читающие потоки выйдут из функции чтения и обнулят счетчик читающих потоков
     */
    res = rt_event_wait(&set->eflags, SHMEM_WRITER_MASK, &after_mask,
                        EV_ALL,
                        TM_INFINITE);

    if (res != 0)
    {
        printf("error write_shmem: rt_event_wait\n");
        return;
    }

    // В первые два байта сохраняем длину блока
    *((unsigned short*) set->shmem) = datalen;

    // в буфер (со смещением в два байта) копируем блок данных
    memcpy(set->shmem + sizeof(unsigned short), data, datalen);
    //printf("datalen write_shmem: %i\n", datalen);

    res = rt_event_signal(&set->eflags, ~SHMEM_WRITER_MASK);
    if (res != 0)
    {
        printf("error write_shmem: rt_event_signal\n");
        return;
    }
}


void read_shmem(shmem_in_set_t* set, void* data, unsigned short* datalen)
{
    unsigned long after_mask;
    /**
     * \~russian Подождем, если пишущий поток выставил флаг, что он занят записью
     */
    int res = rt_event_wait(&set->remote_shmem.eflags, ~SHMEM_WRITER_MASK, &after_mask,
                            EV_ALL,
                            TM_INFINITE);
    if (res != 0) {
        printf("error read_shmem: rt_event_wait\n");
        print_event_wait_error(res);
        return;
    }

    /**
     * Залочим мьютекс
     */
    res = rt_mutex_acquire(&set->remote_shmem.mutex_read_shmem, TM_INFINITE);
    if (res != 0)
    {
        printf("error read_shmem: rt_mutex_acquire1\n");
        return;
    }

    /**
     * Считываем показания счетчика (младших битов флагов)
     */
    RT_EVENT_INFO info;
    res = rt_event_inquire(&set->remote_shmem.eflags, &info);
    if (res != 0) {
        printf("error read_shmem: rt_event_inquire1\n");
        return;
    }
    //printf("read raw mask = 0x%08X\n", info.value);

    // инкрементируем показания счетчика
    unsigned long count = (~(info.value & SHMEM_WRITER_MASK))
            & SHMEM_WRITER_MASK;
    //printf("masked raw mask = 0x%08X\n", count);
    if (count == 0)
        count = 1;
    else
        count = count << 1;

    //printf("clear mask = 0x%08X\n", count);

    // Сбросим флаги в соответствии со значением счетчика
    res = rt_event_clear(&set->remote_shmem.eflags, count, &after_mask);
    if (res != 0) {
        printf("error read_shmem: rt_event_clear\n");
        return;
    }

    res = rt_mutex_release(&set->remote_shmem.mutex_read_shmem);
    if (res != 0) {
        printf("error read_shmem:  rt_mutex_release1\n");
        return;
    }

    // из первых двух байт считываем блину последующего блока
    unsigned short buflen = *((unsigned short*) set->remote_shmem.shmem);
    //printf("buflen read_shmem: %i\n", buflen);

    if (buflen != 0) {
        // со смещением в два байта читаем следующий блок данных
        memcpy(data, set->remote_shmem.shmem + sizeof(unsigned short), buflen);
    }
    *datalen = buflen;

    /**
     * Залочим мьютекс
     */
    res = rt_mutex_acquire(&set->remote_shmem.mutex_read_shmem, TM_INFINITE);
    if (res != 0) {
        printf("error read_shmem: rt_mutex_acquire2\n");
        return;
    }

    /**
     * Считываем показания счетчика (младших битов флагов)
     */
    res = rt_event_inquire(&set->remote_shmem.eflags, &info);
    if (res != 0) {
        printf("error read_shmem: rt_event_inquire1\n");
        return;
    }
    // декрементируем показания счетчика
    count = (~(info.value & SHMEM_WRITER_MASK));

    count = count ^ (count >> 1);

    //printf("set mask = 0x%08X\n", count);

    // Установим флаги в соответствии со значением счетчика
    res = rt_event_signal(&set->remote_shmem.eflags, count);
    if (res != 0) {
        printf("error read_shmem: rt_event_signal\n");
        return;
    }

    res = rt_mutex_release(&set->remote_shmem.mutex_read_shmem);
    if (res != 0)
    {
        printf("error read_shmem:  rt_mutex_release2\n");
        return;
    }

}




/**
 * @brief send2queues Рассылает объект по входным очередям инстансов подписчиков
 * @param out_object
 * @return
 */
int send2queues(out_object_t* out_object, void* data_obj, bson_t* bson_obj)
{
    void* pval;
    int i;
    for(i=0;i<out_object->out_queue_sets_len;i++)
    {
        bson_init (bson_obj);

        out_queue_set_t* out_queue_set = out_object->out_queue_sets[i];

        int cl;
        for(cl=0;cl<out_queue_set->len;cl++)
        {
            remote_obj_field_t* remote_obj_field = out_queue_set->remote_obj_fields[cl];

            switch (remote_obj_field->type_field_obj)
            {
                case fieldInteger:
                    pval = data_obj + remote_obj_field->offset_field_obj;
                    bson_append_int32 (bson_obj, remote_obj_field->remote_field_name, -1, *((int*)pval));
                    break;

                default:
                    printf("Funcrion \"send2queues\" Unknown type remote field\n");
                    break;
            }
        }

        //debug_print_bson("send2queues", bson_obj);

        int res = rt_queue_write(&out_queue_set->out_queue->remote_queue, bson_get_data(bson_obj), bson_obj->len, Q_NORMAL);
        if(res<0)
        {
            printf("Warning: %i rt_queue_write\n", res);
            // TODO: если нет коннекта у очереди, то сбросить флаг коннекта всех очередей.
        }

        bson_destroy(bson_obj);
    }

    return 0;
}


/**
 * @brief get_input_data Функция получения данных
 * Должна вызываться из бизнес функции модуля. Доставляет данные из входной очереди и из шаред мемори инстансов поставщиков
 * @param p_module
 */
void get_input_data(void* p_module)
{
    module_t* module = p_module;

    if(module->input_data==NULL)
    {
        //здесь просто поспать потоку
        rt_task_sleep(module->queue_timeout);

        //printf("Module don't have input\n");
        return;
    }

    //TODO: Определить размер буфера где нибудь в настройках
    // и вынести в структуру
    char buf[256];

    module->updated_input_properties = 0;

    int res_read = rt_queue_read(&module->in_queue, buf, 256, module->queue_timeout);
    if (res_read > 0)
    {
        bson_t bson;
        bson_init_static(&bson, buf, res_read);
        //debug_print_bson("get_input_data", &bson);
        if ((*module->input_bson2obj)(module, &bson) != 0)
        {
            printf("Error: func get_input_data, input_bson2obj\n");
        }
        else
        {
            printf("%s: ", module->instance_name);
            (*module->print_input)(module->input_data);
        }
        bson_destroy(&bson);
    }

    // Если установлены флаги того, что юзер обязательно хочет каких то данных,
    // то постараемся их вытащить из разделяемой памяти
    // Если конечно его запрос не удовлетворен уже (возможно) полученными данными
    // 1) получено:             0010011 updated_input_properties
    // 2) мне надо:             0110001 refresh_input_mask
    // 3) часть нужного
    // мне из полученного:      0010001 получается логическим И 1&2
    // 4) осталось получить:    0100000 получается исключающим ИЛИ 2^3
    //printf("before logic oper mask=0x%08X\n", module->refresh_input_mask);
    //printf("updated_input_properties=0x%08X\n", module->updated_input_properties);
    module->refresh_input_mask ^= (module->refresh_input_mask & module->updated_input_properties);
    //printf("before refresh mask=0x%08X\n", module->refresh_input_mask);
    refresh_input(module);
}

/**
 * @brief connect_links Устанавливает исходящие соединения
 * С входными очередями инстансов подписчиков
 * @param p_module
 * @return
 */
int connect_out_links(void *p_module)
{
    module_t* module = p_module;

    //printf("len=%i\n", module->len);

    int count_connected = 0, i;
    for(i=0; i < module->remote_queues_len;i++)
    {
        remote_queue_t* info_remote_queue = module->remote_queues[i];

        if(info_remote_queue->f_queue_connected)
        {
            count_connected++;
            continue;
        }

        char name_queue[XNOBJECT_NAME_LEN] = "";
        strcat(name_queue, info_remote_queue->name_instance);
        strcat(name_queue, SUFFIX_QUEUE);

        //printf("attempt connect %s to %s\n", module->instance_name, name_queue);

        int res = rt_queue_bind	(&info_remote_queue->remote_queue, name_queue, TM_NONBLOCK);
        if(res!=0)
        {
            //printf("Error:%i rt_queue_bind instance=%s to queue %s\n", res, module->instance_name, name_queue);
            continue;
        }
        else
        {
            info_remote_queue->f_queue_connected=true;
            printf("CONNECTED: %s to queue %s\n", module->instance_name, name_queue);
        }

        count_connected++;
    }

    if(count_connected==module->remote_queues_len)
    {
        module->f_connected_out_links=true;
        printf("%s: ALL QUEUES CONNECTED\n", module->instance_name);
    }

    return 0;
}



int connect_in_links(void *p_module)
{
    module_t* module = p_module;
/*
    //printf("len=%i\n", module->len);

    int count_connected = 0, i;
    for(i=0; i < module->remote_queues_len;i++)
    {
        remote_queue_t* info_remote_queue = module->remote_queues[i];

        if(info_remote_queue->f_queue_connected)
        {
            count_connected++;
            continue;
        }

        char name_queue[XNOBJECT_NAME_LEN] = "";
        strcat(name_queue, info_remote_queue->name_instance);
        strcat(name_queue, SUFFIX_QUEUE);

        //printf("attempt connect %s to %s\n", module->instance_name, name_queue);

        int res = rt_queue_bind	(&info_remote_queue->remote_queue, name_queue, TM_NONBLOCK);
        if(res!=0)
        {
            //printf("Error:%i rt_queue_bind instance=%s to queue %s\n", res, module->instance_name, name_queue);
            continue;
        }
        else
        {
            info_remote_queue->f_queue_connected=true;
            printf("CONNECTED: %s to queue %s\n", module->instance_name, name_queue);
        }

        count_connected++;
    }

    if(count_connected==module->remote_queues_len)
    {
        module->f_connected_out_links=true;
        printf("%s: ALL QUEUES CONNECTED\n", module->instance_name);
    }
*/
    return 0;
}



/**
 * @brief task_transmit Функция выполняется в потоке задачи передачи данных подписчикам
 * @param p_module Указатель на инстанс модуль
 */
void task_transmit(void *p_module)
{
    module_t* module = p_module;
    int cycle = 0;

    bson_t bson_tr;
    void* obj;

    RTIME time_last_publish_shmem;
    RTIME time_attempt_link_modules;

    time_last_publish_shmem = rt_timer_read();
    time_attempt_link_modules = rt_timer_read();

    while (1) {
        int res = rt_mutex_acquire(&module->mutex_obj_exchange, TM_INFINITE);
        if (res != 0)
        {
            printf("error function task_transmit_body: rt_mutex_acquire\n");
            return;
        }
        // Если нет заполненных объектов, то поспим пока они не появятся
        res = rt_cond_wait(&module->obj_cond, &module->mutex_obj_exchange, module->transmit_task_period);
        if (res == 0)
        {
            int i=0;
            out_object_t* out_object = module->out_objects[i];
            while(out_object)
            {
                checkout4transmiter(module, out_object, &obj);
                if(obj!=NULL)
                {
                    // Нашли обновившийся в основном потоке объект
                    // Пуш в очереди подписчиков
                    send2queues(out_object, obj, &bson_tr);



                    // Публикация данных в разделяемую память, не чаще чем в оговоренный период
                    if(rt_timer_read() - time_last_publish_shmem > module->transmit_task_period)
                    {
                        bson_init (&bson_tr);
                        // Call user convert function
                        (*out_object->obj2bson)(obj, &bson_tr);
                        write_shmem(&out_object->shmem_set, bson_get_data(&bson_tr), bson_tr.len);
                        //printf("send %i\n", bson_tr.len);
                        bson_destroy(&bson_tr);
                        time_last_publish_shmem=rt_timer_read();
                    }

                    // Вернуть объект основному потоку на новое заполнение
                    checkin4transmiter(module, out_object, &obj);
                }
                out_object = module->out_objects[++i];
            }
        }
        else if (res!=-ETIMEDOUT)
        {
            printf("error=%i in task_transmit_body:  rt_cond_wait\n", res);
            return;
        }

        //printf("task_transmit cycle %i\n", cycle++);
        if(!module->f_connected_out_links)
        {
            // Если не все связи модуля установлены, то будем пытаться их установить
            if(rt_timer_read() - time_attempt_link_modules > 100000000)
            {
                //printf("попытка out связи\n");

                connect_out_links(module);

                time_attempt_link_modules=rt_timer_read();
            }
        }

        if(!module->f_connected_in_links)
        {
            // Если не все связи модуля установлены, то будем пытаться их установить
            if(rt_timer_read() - time_attempt_link_modules > 100000000)
            {
                printf("попытка in связи\n");

                connect_in_links(module);

                time_attempt_link_modules=rt_timer_read();
            }
        }

    }
}


int start(void* p_module)
{
    module_t* module = p_module;

    /**
     * Create required xenomai services
     */
    int err = create_xenomai_services(module);
    if (err != 0) {
        printf("Error create xenomai services\n");
        return err;
    }


    if (module == NULL) {
        printf("Function \"start\". Param \"module\" is null\n");
        return -1;
    }

    if (module->func == NULL) {
        printf("module->func for main task required\n");
        return -1;
    }

    err = rt_task_start(&module->task_main, module->func, p_module);
    if (err != 0)
        printf("Error start main task\n");

    // Если нет выходов не нужна и таска передатчика
    if(module->out_objects[0]==NULL)
        return err;

    err = rt_task_start(&module->task_transmit, &task_transmit, p_module);
    if (err != 0) {
        printf("Error start transmit task. err=%i\n", err);
        print_task_start_error(err);
    }

    return err;
}


int stop(void* p_module)
{
    module_t* module = p_module;

    // TODO: Удалить и все сервисы ксеномая
    rt_task_delete(&module->task_transmit);
    rt_task_delete(&module->task_main);

    //int res = rt_heap_free(&module->h_shmem, module->shmem);

    free(module->module_type);

    free(module->out_objects);

    free(module);

    return 0;
}


int create_xenomai_services(module_t* module)
{
    if(module->input_data)
    {
        // Create input queue
        // But only defined input buffer
        char name_queue[XNOBJECT_NAME_LEN] = "";
        strcat(name_queue, module->instance_name);
        strcat(name_queue, SUFFIX_QUEUE);
        int queue_poolsize = 200; //TODO вынести эту цифру в настройки
        int err = rt_queue_create(&module->in_queue, name_queue, queue_poolsize, 10, Q_FIFO);
        if (err != 0)
        {
            fprintf(stdout, "Error create queue \"%s\"\n", name_queue);
            return err;
        }
    }


    // Create main task
    char name_task_main[XNOBJECT_NAME_LEN] = "";
    strcat(name_task_main, module->instance_name);
    strcat(name_task_main, SUFFIX_TASK);
    int err = rt_task_create(&module->task_main, name_task_main, TASK_STKSZ, module->task_priority, TASK_MODE);
    if (err != 0)
    {
        fprintf(stdout, "Error create work task \"%s\"\n", name_task_main);
        return err;
    }

    // Если не определены выходы для модуля, то нефиг и создавать сервисы
    if(module->out_objects[0]==NULL)
        return 0;

    // Create transmit task
    char name_tr_task_main[XNOBJECT_NAME_LEN] = "";
    strcat(name_tr_task_main, module->instance_name);
    strcat(name_tr_task_main, SUFFIX_TR_TASK);
    err = rt_task_create(&module->task_transmit, name_tr_task_main, TASK_STKSZ, 99, TASK_MODE);
    if (err != 0)
    {
        fprintf(stdout, "Error create transmit task \"%s\"\n", name_tr_task_main);
        return err;
    }


    // Create mutex for exchange between main and transmit task
    char name_objmutex[XNOBJECT_NAME_LEN] = "";
    strcat(name_objmutex, module->instance_name);
    strcat(name_objmutex, SUFFIX_EXCHANGE_MUTEX);
    err = rt_mutex_create(&module->mutex_obj_exchange, name_objmutex);
    if (err != 0) {
        fprintf(stdout, "Error create mutex_obj_exchange \"%s\"\n", name_objmutex);
        return err;
    }

    // Create condition for exchange between main and transmit task
    char name_cond[XNOBJECT_NAME_LEN] = "";
    strcat(name_cond, module->instance_name);
    strcat(name_cond, SUFFIX_CONDITION);
    err = rt_cond_create(&module->obj_cond, name_cond);
    if (err != 0) {
        fprintf(stdout, "Error create obj_cond \"%s\"\n", name_cond);
        return err;
    }

    return 0;
}


/**
 * @brief checkout4writer
 * /~russian    Заполняет указатель адресом на структуру ,
 *              которую можно заполнять данными для последующей передачи в разделяемую память
 * @param obj
 * @return
 * /~russian 0 в случае успеха
 */
int checkout4writer(module_t* module, out_object_t* set, void** obj)
{
    int res = rt_mutex_acquire(&module->mutex_obj_exchange, TM_INFINITE);
    if (res != 0)
    {
        printf("error checkout4writer: rt_mutex_acquire\n");
        return res;
    }

    if(set->status_obj1 == Empty)
    {
        set->status_obj1 = Writing;
        (*obj)=set->obj1;
    }
    else if(set->status_obj2 == Empty)
    {
        set->status_obj2 = Writing;
        (*obj)=set->obj2;
    }
    else if(set->status_obj1 == Filled)
    {
        set->status_obj1 = Writing;
        (*obj)=set->obj1;
    }
    else if(set->status_obj2 == Filled)
    {
        set->status_obj2 = Writing;
        (*obj)=set->obj2;
    }
    else
    {
        printf("checkout4writer: Error in logic use function checkout4writer.\n Impossible combination statuses\n");
        print_obj_status(1, set->status_obj1);
        print_obj_status(2, set->status_obj2);
        printf("\n");
        (*obj)=NULL;
        res = -1;
    }

    int res1 = rt_mutex_release(&module->mutex_obj_exchange);
    if (res1 != 0)
    {
        printf("checkout4writer: error:  rt_mutex_release\n");
        return res1;
    }
    return res;
}


/**
 * @brief checkin4writer
 * /~ Возвращает объект системе (данные будут переданы в разделяемую память)
 * @param set
 * @param obj
 * @return
 */
int checkin4writer(module_t* module, out_object_t* set, void** obj)
{
    int res = rt_mutex_acquire(&module->mutex_obj_exchange, TM_INFINITE);
    if (res != 0)
    {
        printf("error checkin4writer: rt_mutex_acquire\n");
        return res;
    }

    if(set->status_obj1 == Writing)
    {
        set->status_obj1 = Filled;
    }
    else if(set->status_obj2 == Writing)
    {
        set->status_obj2 = Filled;
    }
    else
    {
        printf("checkin4writer: Error in logic use function checkin4writer.\n Impossible combination statuses\n");
        print_obj_status(1, set->status_obj1);
        print_obj_status(2, set->status_obj2);
        printf("\n");
        res = -1;
    }

    (*obj)=NULL;

    int res1 = rt_mutex_release(&module->mutex_obj_exchange);
    if (res1 != 0)
    {
        printf("error checkin4writer:  rt_mutex_release\n");
        return res1;
    }

    // просигнализируем (потоку передачи) что объект готов к передаче
    int res2 = rt_cond_signal(&module->obj_cond);
    if (res2 != 0)
    {
        printf("error checkin4writer:  rt_cond_signal\n");
        return res2;
    }

    return res;
}


/**
 * @brief checkout4transmiter
 * /~russian    Заполняет указатель адресом на структуру ,
 *              данные из которой можно считывать для передачи в разделяемую память
 * @param obj
 * @return
 * /~russian 0 в случае успеха
 */
int checkout4transmiter(module_t* module, out_object_t* set, void** obj)
{
    int res = rt_mutex_acquire(&module->mutex_obj_exchange, TM_INFINITE);
    if (res != 0)
    {
        printf("error checkout4transmiter: rt_mutex_acquire\n");
        return res;
    }

    if(set->status_obj1 == Filled)
    {
        set->status_obj1 = Transferring;
        (*obj)=set->obj1;
    }
    else if(set->status_obj2 == Filled)
    {
        set->status_obj2 = Transferring;
        (*obj)=set->obj2;
    }
    else
    {
        (*obj)=NULL;
    }

    int res1 = rt_mutex_release(&module->mutex_obj_exchange);
    if (res1 != 0)
    {
        printf("error checkout4transmiter:  rt_mutex_release\n");
        return res1;
    }
    return res;
}


/**
 * @brief checkin4transmiter
 * /~ Возвращает объект системе (объект будет помечен как свободный для записи основным потоком)
 * @param obj
 * @return
 */
int checkin4transmiter(module_t* module, out_object_t* set,  void** obj)
{
    int res = rt_mutex_acquire(&module->mutex_obj_exchange, TM_INFINITE);
    if (res != 0)
    {
        printf("error checkin4transmiter: rt_mutex_acquire\n");
        return res;
    }

    if(set->status_obj1 == Transferring)
    {
        set->status_obj1 = Empty;
    }
    else if(set->status_obj2 == Transferring)
    {
        set->status_obj2 = Empty;
    }
    else
    {
        printf("checkin4transmiter: Error in logic use function checkin4transmiter.\n Impossible combination statuses\n");
        print_obj_status(1, set->status_obj1);
        print_obj_status(2, set->status_obj2);
        printf("\n");
        res = -1;
    }

    (*obj)=NULL;

    int res1 = rt_mutex_release(&module->mutex_obj_exchange);
    if (res1 != 0)
    {
        printf("error checkin4transmiter:  rt_mutex_release\n");
        return res1;
    }
    return res;
}


/**
 * @brief refresh_input
 * /~russian Функция вычитывает данные из разделяемой памяти и мержит их во входной объект
 * @param p_module
 * @return
 */
int refresh_input(void* p_module)
{
    module_t* module = p_module;

    // биты не установлены, обновления данных не требуется
    if(module->refresh_input_mask == 0)
        return 0;

    //TODO: Определить размер буфера где нибудь в настройках
    // и вынести в структуру
    char buf[300];

    // Бежим по всем входящим связям типа разделяемой памяти и если какая то из них
    // ассоциирована с требуемыми для обновления входами, произведем чтение объекта из разделяемой памяти и мапинг свойств
    int i=0;
    for(i=0;i<module->remote_shmems_len;i++)
    {
        shmem_in_set_t* remote_shmem = module->remote_shmems[i];
        if(remote_shmem->assigned_input_ports_mask & module->refresh_input_mask)
        {
            unsigned short retlen;
            read_shmem(remote_shmem, buf, &retlen);
            //printf("retlen=%i\n", retlen);
            bson_t bson;
            if (retlen > 0) {
                bson_init_static(&bson, buf, retlen);
                //debug_print_bson(&bson);
                (*module->print_input)(module->input_data);
            }
        }
    }

    // перед выходом обнулить биты. они отработаны
    module->refresh_input_mask = 0;
    return 0;
}




