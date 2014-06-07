#include "../include/module-functions.h"


// Convert structure common_params_t to bson
int common_params2bson(common_params_t* obj, bson_t* bson)
{
    bson_append_int32 (bson, "Task Priority", -1, obj->Task_Priority);
    bson_append_int32 (bson, "Task Period", -1, obj->Task_Period);
    bson_append_int32 (bson, "Transfer task period", -1, obj->Transfer_task_period);
    return 0;
}


// Convert bson to structure common_params_t
int bson2common_params(module_t* module, bson_t* bson)
{
    if(!module ||  !bson)
    {
        printf("Error: func bson2params_rx_openlrs, NULL parameter\n");
        return -1;
    }

    common_params_t* obj = (common_params_t*)module->input_data;
    bson_iter_t iter;
    if(!bson_iter_init (&iter, bson))
    {
        printf("Error: func bson2params_rx_openlrs, bson_iter_init\n");
        return -1;
    }

    while(bson_iter_next(&iter))
    {
        const char* key = bson_iter_key (&iter);

        if(!strncmp(key, "Task Priority", XNOBJECT_NAME_LEN))
        {
            obj->Task_Priority = bson_iter_int32(&iter);
            continue;
        }
        if(!strncmp(key, "Task Period", XNOBJECT_NAME_LEN))
        {
            obj->Task_Period = bson_iter_int32(&iter);
            continue;
        }
        if(!strncmp(key, "Transfer task period", XNOBJECT_NAME_LEN))
        {
            obj->Transfer_task_period = bson_iter_int32(&iter);
            continue;
        }
    }
    return 0;
}


// Helper function. Print structure common_params_t
void print_common_params(common_params_t* obj)
{
    printf("Task_Priority=%i\t", obj->Task_Priority);
    printf("Task_Period=%i\t", obj->Task_Period);
    printf("Transfer_task_period=%i\t", obj->Transfer_task_period);
    printf("\n");
}
