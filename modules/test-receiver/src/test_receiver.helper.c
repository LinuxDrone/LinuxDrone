#include "../include/test_receiver.helper.h"

// количество типов выходных объектов
#define count_outs 0

extern t_cycle_function test_receiver_run;

int get_inputmask_by_inputname(char* input_name)
{
    if(!strncmp(input_name, "in1", XNOBJECT_NAME_LEN))
        return in1;
    if(!strncmp(input_name, "in2", XNOBJECT_NAME_LEN))
        return in2;
    if(!strncmp(input_name, "in3", XNOBJECT_NAME_LEN))
        return in3;
    if(!strncmp(input_name, "in4", XNOBJECT_NAME_LEN))
        return in4;

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
        if(!strncmp(key, "in3", XNOBJECT_NAME_LEN))
        {
            obj->in3 = bson_iter_double(&iter);
            module->updated_input_properties |= in3;
            continue;
        }
        if(!strncmp(key, "in4", XNOBJECT_NAME_LEN))
        {
            obj->in4 = bson_iter_double(&iter);
            module->updated_input_properties |= in4;
            continue;
        }
    }
    return 0;
}

// Helper function. Print structure input
void print_test_receiver(void* obj1)
{
    input_t* obj=obj1;
    printf("in1=%lf\t", obj->in1);
    printf("in2=%lf\t", obj->in2);
    printf("in3=%lf\t", obj->in3);
    printf("in4=%lf\t", obj->in4);
    printf("\n");
}

// Create module.
module_test_receiver_t* test_receiver_create(void *handle)
{
    module_test_receiver_t* module = calloc(1, sizeof(module_test_receiver_t));
    // Сохраним указатель на загруженную dll
    module->module_info.dll_handle = handle;
    module->module_info.out_objects = calloc(count_outs+1, sizeof(void *));
    return module;
}

int get_offset_in_input_by_inpinname(module_test_receiver_t* module, char* name_inpin)
{
    if(!strncmp(name_inpin, "in1", XNOBJECT_NAME_LEN))
    {
        return (void*)&module->input4modul.in1 - (void*)&module->input4modul;
    }
    if(!strncmp(name_inpin, "in2", XNOBJECT_NAME_LEN))
    {
        return (void*)&module->input4modul.in2 - (void*)&module->input4modul;
    }
    if(!strncmp(name_inpin, "in3", XNOBJECT_NAME_LEN))
    {
        return (void*)&module->input4modul.in3 - (void*)&module->input4modul;
    }
    if(!strncmp(name_inpin, "in4", XNOBJECT_NAME_LEN))
    {
        return (void*)&module->input4modul.in4 - (void*)&module->input4modul;
    }
    printf("Not found property \"%s\" among properties in input object\n", name_inpin);
    return -1;
}

// Stop and delete module. Free memory.
void test_receiver_delete(module_test_receiver_t* module)
{
    stop(module);
}

// Init module.
int test_receiver_init(module_test_receiver_t* module, const uint8_t* bson_data, uint32_t bson_len)
{
    // Input
    memset(&module->input4modul, 0, sizeof(input_t));
    module->module_info.input_data = &module->input4modul;
    module->module_info.input_bson2obj = (p_bson2obj)&bson2input;
    module->module_info.get_inmask_by_inputname = (p_get_inputmask_by_inputname)&get_inputmask_by_inputname;
    module->module_info.get_offset_in_input_by_inpinname = (p_get_offset_in_input_by_inpinname)&get_offset_in_input_by_inpinname;

    module->module_info.func = &test_receiver_run;

    module->module_info.print_input = &print_test_receiver;

    int res = init(&module->module_info, bson_data, bson_len);


    return res;
}

int test_receiver_start(module_test_receiver_t* module)
{
    if (start(module) != 0)
        return -1;
    return 0;
}

