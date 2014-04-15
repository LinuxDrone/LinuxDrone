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

void debug_print_bson(bson_t* bson) {
    char* str = bson_as_json(bson, NULL);
    fprintf(stdout, "%s\n", str);
    bson_free(str);
}

void print_task_start_error(int err) {
    switch (err)
    {
    case -EINVAL:
        printf("is returned if task is not a task descriptor.\n");
        break;

    case -EIDRM:
        printf("is returned if task is a deleted task descriptor.\n");
        break;

    case -EBUSY:
        printf("is returned if task is already started.\n");
        break;

    case -EPERM:
        printf("is returned if this service was called from an asynchronous context.\n");
        break;

    default:
        printf("Unknown task error: %i.\n", err);
    }
}

void print_event_wait_error(int err) {
    switch (err)
    {
    case -EINVAL:
        printf("is returned if event is not a event group descriptor.\n");
        break;

    case -EIDRM:
        printf("is returned if event is a deleted event group descriptor, including if the deletion occurred while the caller was sleeping on it before the request has been satisfied.\n");
        break;

    case -EWOULDBLOCK:
        printf("is returned if timeout is equal to TM_NONBLOCK and the current event mask value does not satisfy the request.\n");
        break;

    case -EINTR:
        printf("is returned if rt_task_unblock() has been called for the waiting task before the request has been satisfied.\n");
        break;

    case -ETIMEDOUT:
        printf("is returned if the request has not been satisfied within the specified amount of time.\n");
        break;

    case -EPERM:
        printf("is returned if this service should block, but was called from a context which cannot sleep (e.g. interrupt, non-realtime context).\n");
        break;

    default:
        printf("Unknown event wait error: %i.\n", err);
    }
}

void print_obj_status(int number_obj, StatusObj status) {
    switch (status)
    {
    case Empty:
        printf("Obj%i=Empty\n", number_obj);
        break;

    case Writing:
        printf("Obj%i=Writing\n", number_obj);
        break;

    case Transferring:
        printf("Obj%i=Transferring\n", number_obj);
        break;

    case Filled:
        printf("Obj%i=Filled\n", number_obj);
        break;

    default:
        printf("Unknown status of obj: %i.\n", status);
    }
}

int init_object_set(out_object_t * pset, char* instance_name, char* out_name)
{
    // Create shared memory
    char name_shmem[64] = "";
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
    char name_eflags[64] = "";
    strcat(name_eflags, instance_name);
    strcat(name_eflags, out_name);
    strcat(name_eflags, "_flags");
    err = rt_event_create(&pset->shmem_set.eflags, name_eflags, ULONG_MAX, EV_PRIO);
    if (err != 0) {
        fprintf(stdout, "Error create event service \"%s\"\n", name_eflags);
        return err;
    }

    // Create mutex for read shared memory
    char name_rmutex[64] = "";
    strcat(name_rmutex, instance_name);
    strcat(name_rmutex, out_name);
    strcat(name_rmutex, "_rmutex");
    err = rt_mutex_create(&pset->shmem_set.mutex_read_shmem, name_rmutex);
    if (err != 0) {
        fprintf(stdout, "Error create mutex_read_shmem \"%s\"\n", name_rmutex);
        return err;
    }

    pset->status_obj1 = Empty;
    pset->status_obj2 = Empty;

    return 0;
}

int init(module_t* module, const uint8_t * data, uint32_t length)
{
    bson_t bson;
    bson_init_static(&bson, data, length);
    //debug_print_bson(&bson);

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
    //fprintf(stdout, "instance name=%s\n", module->instance_name);

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
    module->transmit_task_period = rt_timer_ns2ticks(
                bson_iter_int32(&iter) * 1000);
    //fprintf(stdout, "transmit_task_period=%i\n", module->transmit_task_period);

    return 0;
}

/**
 * @brief write_shmem копирует блок данных (data) в разделяемую память, определенную в set
 * @param set структура, содержащая все необходимые данные для работы с блоком разделяемой памяти
 * @param data данные копируемые в разделяемую память
 * @param datalen длина копируемых данных
 */
void write_shmem(shmem_set_t* set, const char* data, unsigned short datalen)
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

void read_shmem(shmem_set_t* set, void* data, unsigned short* datalen)
{
    unsigned long after_mask;
    /**
     * \~russian Подождем, если пишущий поток выставил флаг, что он занят записью
     */
    int res = rt_event_wait(&set->eflags, ~SHMEM_WRITER_MASK, &after_mask,
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
    res = rt_mutex_acquire(&set->mutex_read_shmem, TM_INFINITE);
    if (res != 0)
    {
        printf("error read_shmem: rt_mutex_acquire1\n");
        return;
    }

    /**
     * Считываем показания счетчика (младших битов флагов)
     */
    RT_EVENT_INFO info;
    res = rt_event_inquire(&set->eflags, &info);
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
    res = rt_event_clear(&set->eflags, count, &after_mask);
    if (res != 0) {
        printf("error read_shmem: rt_event_clear\n");
        return;
    }

    res = rt_mutex_release(&set->mutex_read_shmem);
    if (res != 0) {
        printf("error read_shmem:  rt_mutex_release1\n");
        return;
    }

    // из первых двух байт считываем блину последующего блока
    unsigned short buflen = *((unsigned short*) set->shmem);
    //printf("buflen read_shmem: %i\n", buflen);

    if (buflen != 0) {
        // со смещением в два байта читаем следующий блок данных
        memcpy(data, set->shmem + sizeof(unsigned short), buflen);
    }
    *datalen = buflen;

    /**
     * Залочим мьютекс
     */
    res = rt_mutex_acquire(&set->mutex_read_shmem, TM_INFINITE);
    if (res != 0) {
        printf("error read_shmem: rt_mutex_acquire2\n");
        return;
    }

    /**
     * Считываем показания счетчика (младших битов флагов)
     */
    res = rt_event_inquire(&set->eflags, &info);
    if (res != 0) {
        printf("error read_shmem: rt_event_inquire1\n");
        return;
    }
    // декрементируем показания счетчика
    count = (~(info.value & SHMEM_WRITER_MASK));

    count = count ^ (count >> 1);

    //printf("set mask = 0x%08X\n", count);

    // Установим флаги в соответствии со значением счетчика
    res = rt_event_signal(&set->eflags, count);
    if (res != 0) {
        printf("error read_shmem: rt_event_signal\n");
        return;
    }

    res = rt_mutex_release(&set->mutex_read_shmem);
    if (res != 0)
    {
        printf("error read_shmem:  rt_mutex_release2\n");
        return;
    }

}





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
        printf("ofiget' ne vstat'\n");

        // распарсить полученный объект

        bson_t bson;
        bson_init_static(&bson, buf, res_read);

        if(module->input_bson2obj(module, &bson)==0)
        {
            printf("Error: func get_input_data, input_bson2obj\n");
        }

        // destroy bson
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

void task_transmit_body(void *p_module)
{
    module_t* module = p_module;
    int cycle = 0;

    bson_t bson_tr;
    void* obj;
    RTIME time_last_publish_shmem;

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
            out_object_t* set = module->out_objects[i];
            while(set)
            {
                checkout4transmiter(module, set, &obj);
                if(obj!=NULL)
                {
                    // Нашли обновившийся в основном потоке объект
                    // Пуш в очереди подписчиков



                    // Публикация данных в разделяемую память, не чаще чем в оговоренный период
                    if(rt_timer_read() - time_last_publish_shmem > module->transmit_task_period)
                    {

                        bson_init (&bson_tr);
                        // Call user convert function
                        (*set->obj2bson)(obj, &bson_tr);
                        write_shmem(&set->shmem_set, bson_get_data(&bson_tr), bson_tr.len);
                        //printf("send %i\n", bson_tr.len);
                        bson_destroy(&bson_tr);
                        time_last_publish_shmem=rt_timer_read();
                    }

                    // Вернуть объект основному потоку на новое заполнение
                    checkin4transmiter(module, set, &obj);
                }
                set = module->out_objects[++i];
            }
        }
        else if (res!=-ETIMEDOUT)
        {
            printf("error=%i in task_transmit_body:  rt_cond_wait\n", res);
            return;
        }

        //printf("task_transmit cycle %i\n", cycle++);
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

    err = rt_task_start(&module->task_transmit, &task_transmit_body, p_module);
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
    dlclose(module->dll_handle);
    free(module);

    return 0;
}


int create_xenomai_services(module_t* module)
{
    if(module->input_data)
    {
        // Create input queue
        // But only defined input buffer
        char name_queue[64] = "";
        strcat(name_queue, module->instance_name);
        strcat(name_queue, "_queue");
        int queue_poolsize = 200; //TODO вынести эту фиыру в настройки
        int err = rt_queue_create(&module->in_queue, name_queue, queue_poolsize, 10, Q_FIFO);
        if (err != 0)
        {
            fprintf(stdout, "Error create queue \"%s\"\n", name_queue);
            return err;
        }
    }


    // Create main task
    char name_task_main[64] = "";
    strcat(name_task_main, module->instance_name);
    strcat(name_task_main, "_task");

    int err = rt_task_create(&module->task_main, name_task_main,
                         TASK_STKSZ,
                         module->task_priority,
                         TASK_MODE);
    if (err != 0)
    {
        fprintf(stdout, "Error create work task \"%s\"\n", name_task_main);
        return err;
    }

    // Если не определены выходы для модуля, то нефиг и создавать сервисы
    if(module->out_objects[0]==NULL)
        return 0;

    // Create transmit task
    char name_tr_task_main[64] = "";
    strcat(name_tr_task_main, module->instance_name);
    strcat(name_tr_task_main, "_tr_task");
    err = rt_task_create(&module->task_transmit, name_tr_task_main, TASK_STKSZ, 99, TASK_MODE);
    if (err != 0)
    {
        fprintf(stdout, "Error create transmit task \"%s\"\n", name_tr_task_main);
        return err;
    }


    // Create mutex for exchange between main and transmit task
    char name_objmutex[64] = "";
    strcat(name_objmutex, module->instance_name);
    strcat(name_objmutex, "_objmutex");
    err = rt_mutex_create(&module->mutex_obj_exchange, name_objmutex);
    if (err != 0) {
        fprintf(stdout, "Error create mutex_obj_exchange \"%s\"\n", name_objmutex);
        return err;
    }

    // Create condition for exchange between main and transmit task
    char name_cond[64] = "";
    strcat(name_cond, module->instance_name);
    strcat(name_cond, "_cond");
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

    // Здесь подготовить список сетов, которые необходимо вычитать из разделяемой памяти

    //TODO: Определить размер буфера где нибудь в настройках
    // и вынести в структуру
    char buf[300];

    unsigned short retlen;
    // Данный сет является просто одним из выходных сетов этого же модуля
    // исключительно для проверки
    out_object_t* set = module->out_objects[0];
    read_shmem(&set->shmem_set, buf, &retlen);
    //printf("retlen=%i\n", retlen);

    bson_t bson;
    if (retlen > 0) {
        bson_init_static(&bson, buf, retlen);
        //debug_print_bson(&bson);

        if(set->bson2obj(module, &bson)==0)
        {
            (*set->print_obj)(module->input_data);
        }
    }

    // перед выходом обнулить биты. они отработаны
    module->refresh_input_mask = 0;
    return 0;
}

/* return a new string with every instance of ch replaced by repl */
char *replace(const char *s, char ch, const char *repl) {
    int count = 0;
    const char *t;
    for(t=s; *t; t++)
        count += (*t == ch);

    size_t rlen = strlen(repl);
    char *res = malloc(strlen(s) + (rlen-1)*count + 1);
    char *ptr = res;
    for(t=s; *t; t++) {
        if(*t == ch) {
            memcpy(ptr, repl, rlen);
            ptr += rlen;
        } else {
            *ptr++ = *t;
        }
    }
    *ptr = 0;
    return res;
}

