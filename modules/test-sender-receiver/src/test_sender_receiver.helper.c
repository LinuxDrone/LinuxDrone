#include "../include/test_sender_receiver.helper.h"

// количество типов выходных объектов
#define count_outs 2

extern t_cycle_function test_sender_receiver_run;

int get_inputmask_by_inputname(char* input_name)
{
    if(!strncmp(input_name, "in1", XNOBJECT_NAME_LEN))
        return in1;
    if(!strncmp(input_name, "in2", XNOBJECT_NAME_LEN))
        return in2;

    return 0;
}

// Convert bson to structure input
int bson2input(module_t* module, bson_t* bson)
{
    if(!module ||  !module->input_data ||  !bson)
    {
        printf("Error: func bson2input, NULL parameter\n");
        return -1;
    }

    input_t* obj = (input_t*)module->input_data;
    bson_iter_t iter;
    if(!bson_iter_init (&iter, bson))
    {
        printf("Error: func bson2input, bson_iter_init\n");
        return -1;
    }

    while(bson_iter_next(&iter))
    {
        const char* key = bson_iter_key (&iter);

        if(!strncmp(key, "in1", XNOBJECT_NAME_LEN))
        {
            obj->in1 = bson_iter_double(&iter);
            module->updated_input_properties |= in1;
            continue;
        }
        if(!strncmp(key, "in2", XNOBJECT_NAME_LEN))
        {
            obj->in2 = bson_iter_double(&iter);
            module->updated_input_properties |= in2;
            continue;
        }
    }
    return 0;
}

// Helper function. Print structure input
void print_test_sender_receiver(void* obj1)
{
    input_t* obj=obj1;
    printf("in1=%lf\t", obj->in1);
    printf("in2=%lf\t", obj->in2);
    printf("\n");
}

// Convert structure Output1 to bson
int Output12bson(Output1_t* obj, bson_t* bson)
{
	bson_append_double (bson, "out1", -1, obj->out1);
	bson_append_double (bson, "out2", -1, obj->out2);
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

        if(!strncmp(key, "out1", XNOBJECT_NAME_LEN))
        {
            obj->out1 = bson_iter_double(&iter);
            module->updated_input_properties |= out1;
            continue;
        }
        if(!strncmp(key, "out2", XNOBJECT_NAME_LEN))
        {
            obj->out2 = bson_iter_double(&iter);
            module->updated_input_properties |= out2;
            continue;
        }
    }
    return 0;
}

// Helper function. Print structure Output1
void print_Output1(Output1_t* obj)
{
    printf("out1=%lf\t", obj->out1);
    printf("out2=%lf\t", obj->out2);
    printf("\n");
}

// Convert structure Output2 to bson
int Output22bson(Output2_t* obj, bson_t* bson)
{
	bson_append_double (bson, "out3", -1, obj->out3);
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

        if(!strncmp(key, "out3", XNOBJECT_NAME_LEN))
        {
            obj->out3 = bson_iter_double(&iter);
            module->updated_input_properties |= out3;
            continue;
        }
    }
    return 0;
}

// Helper function. Print structure Output2
void print_Output2(Output2_t* obj)
{
    printf("out3=%lf\t", obj->out3);
    printf("\n");
}

// Create module.
module_test_sender_receiver_t* test_sender_receiver_create(void *handle)
{
    module_test_sender_receiver_t* module = calloc(1, sizeof(module_test_sender_receiver_t));
    // Сохраним указатель на загруженную dll
    module->module_info.dll_handle = handle;
    module->module_info.out_objects = calloc(count_outs+1, sizeof(void *));
    module->module_info.out_objects[0]=&module->Output1;
    module->module_info.out_objects[1]=&module->Output2;
    return module;
}

// Возвращает указатель на структуру выходного объекта, по имени пина
// Используется при подготовке списка полей, для мапинга объектов (для передачи в очередь)
out_object_t* get_outobject_by_outpin(module_test_sender_receiver_t* module, char* name_out_pin, unsigned short* offset_field, unsigned short* index_port)
{
    (*offset_field) = 0;
    (*index_port) = 0;
    if(!strncmp(name_out_pin, "out1", XNOBJECT_NAME_LEN))
    {
        (*offset_field) = (void*)&module->obj1_Output1.out1 - (void*)&module->obj1_Output1;
        (*index_port) = 0;
        return &module->Output1;
    }
    if(!strncmp(name_out_pin, "out2", XNOBJECT_NAME_LEN))
    {
        (*offset_field) = (void*)&module->obj1_Output1.out2 - (void*)&module->obj1_Output1;
        (*index_port) = 1;
        return &module->Output1;
    }
    if(!strncmp(name_out_pin, "out3", XNOBJECT_NAME_LEN))
    {
        (*offset_field) = (void*)&module->obj1_Output2.out3 - (void*)&module->obj1_Output2;
        (*index_port) = 0;
        return &module->Output2;
    }
    printf("Not found property \"%s\" among properties out objects\n", name_out_pin);
    return NULL;
}

int get_offset_in_input_by_inpinname(module_test_sender_receiver_t* module, char* name_inpin)
{
    if(!strncmp(name_inpin, "in1", XNOBJECT_NAME_LEN))
    {
        return (void*)&module->input4modul.in1 - (void*)&module->input4modul;
    }
    if(!strncmp(name_inpin, "in2", XNOBJECT_NAME_LEN))
    {
        return (void*)&module->input4modul.in2 - (void*)&module->input4modul;
    }
    printf("Not found property \"%s\" among properties in input object\n", name_inpin);
    return -1;
}

// Stop and delete module. Free memory.
void test_sender_receiver_delete(module_test_sender_receiver_t* module)
{
    stop(module);
}

// Init module.
int test_sender_receiver_init(module_test_sender_receiver_t* module, const uint8_t* bson_data, uint32_t bson_len)
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
    // Input
    memset(&module->input4modul, 0, sizeof(input_t));
    module->module_info.input_data = &module->input4modul;
    module->module_info.input_bson2obj = (p_bson2obj)&bson2input;
    module->module_info.get_inmask_by_inputname = (p_get_inputmask_by_inputname)&get_inputmask_by_inputname;
    module->module_info.get_offset_in_input_by_inpinname = (p_get_offset_in_input_by_inpinname)&get_offset_in_input_by_inpinname;

    module->module_info.func = &test_sender_receiver_run;

    module->module_info.print_input = &print_test_sender_receiver;

    int res = init(&module->module_info, bson_data, bson_len);

    // для каждого типа порождаемого объекта инициализируется соответсвующая структура
    // и указываются буферы (для обмена данными между основным и передающим потоком)
    // Output1
    init_object_set(&module->Output1.shmem_set, module->module_info.instance_name, "Output1");
    // Output2
    init_object_set(&module->Output2.shmem_set, module->module_info.instance_name, "Output2");

    return res;
}

int test_sender_receiver_start(module_test_sender_receiver_t* module)
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
int checkout_Output1(module_test_sender_receiver_t* module, Output1_t** obj)
{
    return checkout4writer(module, &module->Output1, obj);
}

/**
* @brief checkin4writer_Output1
* /~ Возвращает объект системе (данные будут переданы в разделяемую память)
* @param obj
* @return
*/
int checkin_Output1(module_test_sender_receiver_t* module, Output1_t** obj)
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
int checkout_Output2(module_test_sender_receiver_t* module, Output2_t** obj)
{
    return checkout4writer(module, &module->Output2, obj);
}

/**
* @brief checkin4writer_Output2
* /~ Возвращает объект системе (данные будут переданы в разделяемую память)
* @param obj
* @return
*/
int checkin_Output2(module_test_sender_receiver_t* module, Output2_t** obj)
{
    return checkin4writer(module, &module->Output2, obj);
}

