#include "../include/test_sender.helper.h"

// количество типов выходных объектов
#define count_outs 2

extern t_cycle_function test_sender_run;

// Convert structure Output1 to bson
int Output12bson(Output1_t* obj, bson_t* bson)
{
	bson_append_int32 (bson, "out1", -1, obj->out1);
	bson_append_int32 (bson, "out2", -1, obj->out2);
	return 0;
}

// Convert bson to structure Output1
int bson2Output1(module_t* module, bson_t* bson)
{
    if(!module ||  !bson)
    {
        printf("Error: func bson2Output1, NULL parameter\n");
        return -1;
    }

    Output1_t* obj = (Output1_t*)module->input_data;
    bson_iter_t iter;
    if(!bson_iter_init (&iter, bson))
    {
        printf("Error: func bson2Output1, bson_iter_init\n");
        return -1;
    }

    while(bson_iter_next(&iter))
    {
        const char* key = bson_iter_key (&iter);

        if(!strncmp(key, "out1", 100))
        {
            obj->out1 = bson_iter_int32(&iter);
            module->updated_input_properties |= out1;
            continue;
        }
        if(!strncmp(key, "out2", 100))
        {
            obj->out2 = bson_iter_int32(&iter);
            module->updated_input_properties |= out2;
            continue;
        }
    }
    return 0;
}

// Helper function. Print structure Output1
void print_Output1(Output1_t* obj)
{
    printf("out1=%i\t", obj->out1);
    printf("out2=%i\t", obj->out2);
    printf("\n");
}

// Convert structure Output2 to bson
int Output22bson(Output2_t* obj, bson_t* bson)
{
	bson_append_int32 (bson, "out3", -1, obj->out3);
	return 0;
}

// Convert bson to structure Output2
int bson2Output2(module_t* module, bson_t* bson)
{
    if(!module ||  !bson)
    {
        printf("Error: func bson2Output2, NULL parameter\n");
        return -1;
    }

    Output2_t* obj = (Output2_t*)module->input_data;
    bson_iter_t iter;
    if(!bson_iter_init (&iter, bson))
    {
        printf("Error: func bson2Output2, bson_iter_init\n");
        return -1;
    }

    while(bson_iter_next(&iter))
    {
        const char* key = bson_iter_key (&iter);

        if(!strncmp(key, "out3", 100))
        {
            obj->out3 = bson_iter_int32(&iter);
            module->updated_input_properties |= out3;
            continue;
        }
    }
    return 0;
}

// Helper function. Print structure Output2
void print_Output2(Output2_t* obj)
{
    printf("out3=%i\t", obj->out3);
    printf("\n");
}

// Create module.
module_test_sender_t* test_sender_create(void *handle)
{
    module_test_sender_t* module = calloc(1, sizeof(module_test_sender_t));
    // Сохраним указатель на загруженную dll
    module->module_info.dll_handle = handle;
    module->module_info.out_objects = calloc(count_outs+1, sizeof(void *));
    module->module_info.out_objects[0]=&module->Output1;
    module->module_info.out_objects[1]=&module->Output2;
    return module;
}

// Возвращает указатель на структуру выходного объекта, по имени пина
// Используется при подготовке списка полей, для мапинга объектов (для передачи в очередь)
out_object_t* get_outobject_by_outpin(module_test_sender_t* module, char* name_out_pin, unsigned short* offset_field, unsigned short* index_port)
{
    (*offset_field) = 0;
    (*index_port) = 0;
    if(!strncmp(name_out_pin, "out1", 100))
    {
        (*offset_field) = (void*)&module->obj1_Output1.out1 - (void*)&module->obj1_Output1;
        (*index_port) = 0;
        return &module->Output1;
    }
    if(!strncmp(name_out_pin, "out2", 100))
    {
        (*offset_field) = (void*)&module->obj1_Output1.out2 - (void*)&module->obj1_Output1;
        (*index_port) = 1;
        return &module->Output1;
    }
    if(!strncmp(name_out_pin, "out3", 100))
    {
        (*offset_field) = (void*)&module->obj1_Output2.out3 - (void*)&module->obj1_Output2;
        (*index_port) = 0;
        return &module->Output2;
    }
    printf("Not found property \"%s\" among properties out objects\n", name_out_pin);
    return NULL;
}

// Stop and delete module. Free memory.
void test_sender_delete(module_test_sender_t* module)
{
    stop(module);
}

// Init module.
int test_sender_init(module_test_sender_t* module, const uint8_t* bson_data, uint32_t bson_len)
{
    // Output1
    // временное решение для указания размера выделяемой памяти под bson  объекты каждого типа
    // в реальности должны один раз создаваться тестовые bson объекты, вычисляться их размер и передаваться в функцию инициализации
    module->Output1.shmem_set.shmem_len = 300;
    module->Output1.obj1 = &module->obj1_Output1;
    module->Output1.obj2 = &module->obj2_Output1;
    module->Output1.obj2bson = (p_obj2bson)&Output12bson;
    module->Output1.bson2obj = (p_bson2obj)&bson2Output1;
    module->Output1.print_obj = (p_print_obj)&print_Output1;

    // Output2
    // временное решение для указания размера выделяемой памяти под bson  объекты каждого типа
    // в реальности должны один раз создаваться тестовые bson объекты, вычисляться их размер и передаваться в функцию инициализации
    module->Output2.shmem_set.shmem_len = 300;
    module->Output2.obj1 = &module->obj1_Output2;
    module->Output2.obj2 = &module->obj2_Output2;
    module->Output2.obj2bson = (p_obj2bson)&Output22bson;
    module->Output2.bson2obj = (p_bson2obj)&bson2Output2;
    module->Output2.print_obj = (p_print_obj)&print_Output2;

    module->module_info.get_outobj_by_outpin = (p_get_outobj_by_outpin)&get_outobject_by_outpin;
    module->module_info.input_data = NULL;

    module->module_info.func = &test_sender_run;

    int res = init(&module->module_info, bson_data, bson_len);

    // для каждого типа порождаемого объекта инициализируется соответсвующая структура
    // и указываются буферы (для обмена данными между основным и передающим потоком)
    // Output1
    init_object_set(&module->Output1, module->module_info.instance_name, "Output1");
    // Output2
    init_object_set(&module->Output2, module->module_info.instance_name, "Output2");

    return res;
}

int test_sender_start(module_test_sender_t* module)
{
    if (start(module) != 0)
        return -1;
    return 0;
}

/**
* @brief checkout4writer_Output1
* /~russian    Заполняет указатель адресом на структуру Output1_t,
*              которую можно заполнять данными для последующей передачи в разделяемую память
* @param obj
* @return
* /~russian 0 в случае успеха
*/
int checkout_Output1(module_test_sender_t* module, Output1_t** obj)
{
    return checkout4writer(module, &module->Output1, obj);
}

/**
* @brief checkin4writer_Output1
* /~ Возвращает объект системе (данные будут переданы в разделяемую память)
* @param obj
* @return
*/
int checkin_Output1(module_test_sender_t* module, Output1_t** obj)
{
    return checkin4writer(module, &module->Output1, obj);
}

/**
* @brief checkout4writer_Output2
* /~russian    Заполняет указатель адресом на структуру Output2_t,
*              которую можно заполнять данными для последующей передачи в разделяемую память
* @param obj
* @return
* /~russian 0 в случае успеха
*/
int checkout_Output2(module_test_sender_t* module, Output2_t** obj)
{
    return checkout4writer(module, &module->Output2, obj);
}

/**
* @brief checkin4writer_Output2
* /~ Возвращает объект системе (данные будут переданы в разделяемую память)
* @param obj
* @return
*/
int checkin_Output2(module_test_sender_t* module, Output2_t** obj)
{
    return checkin4writer(module, &module->Output2, obj);
}

