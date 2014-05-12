#include "../include/c_gy87.helper.h"

// количество типов выходных объектов
#define count_outs 1

extern t_cycle_function c_gy87_run;

// Convert structure GyroAccelMagTemp to bson
int GyroAccelMagTemp2bson(GyroAccelMagTemp_t* obj, bson_t* bson)
{
	bson_append_double (bson, "accelX", -1, obj->accelX);
	bson_append_double (bson, "accelY", -1, obj->accelY);
	bson_append_double (bson, "accelZ", -1, obj->accelZ);
	bson_append_double (bson, "gyroX", -1, obj->gyroX);
	bson_append_double (bson, "gyroY", -1, obj->gyroY);
	bson_append_double (bson, "gyroZ", -1, obj->gyroZ);
	bson_append_double (bson, "magX", -1, obj->magX);
	bson_append_double (bson, "magY", -1, obj->magY);
	bson_append_double (bson, "magZ", -1, obj->magZ);
	bson_append_double (bson, "temperature", -1, obj->temperature);
	return 0;
}

// Convert bson to structure GyroAccelMagTemp
int bson2GyroAccelMagTemp(module_t* module, bson_t* bson)
{
    if(!module ||  !bson)
    {
        printf("Error: func bson2GyroAccelMagTemp, NULL parameter\n");
        return -1;
    }

    GyroAccelMagTemp_t* obj = (GyroAccelMagTemp_t*)module->input_data;
    bson_iter_t iter;
    if(!bson_iter_init (&iter, bson))
    {
        printf("Error: func bson2GyroAccelMagTemp, bson_iter_init\n");
        return -1;
    }

    while(bson_iter_next(&iter))
    {
        const char* key = bson_iter_key (&iter);

        if(!strncmp(key, "accelX", XNOBJECT_NAME_LEN))
        {
            obj->accelX = bson_iter_double(&iter);
            module->updated_input_properties |= accelX;
            continue;
        }
        if(!strncmp(key, "accelY", XNOBJECT_NAME_LEN))
        {
            obj->accelY = bson_iter_double(&iter);
            module->updated_input_properties |= accelY;
            continue;
        }
        if(!strncmp(key, "accelZ", XNOBJECT_NAME_LEN))
        {
            obj->accelZ = bson_iter_double(&iter);
            module->updated_input_properties |= accelZ;
            continue;
        }
        if(!strncmp(key, "gyroX", XNOBJECT_NAME_LEN))
        {
            obj->gyroX = bson_iter_double(&iter);
            module->updated_input_properties |= gyroX;
            continue;
        }
        if(!strncmp(key, "gyroY", XNOBJECT_NAME_LEN))
        {
            obj->gyroY = bson_iter_double(&iter);
            module->updated_input_properties |= gyroY;
            continue;
        }
        if(!strncmp(key, "gyroZ", XNOBJECT_NAME_LEN))
        {
            obj->gyroZ = bson_iter_double(&iter);
            module->updated_input_properties |= gyroZ;
            continue;
        }
        if(!strncmp(key, "magX", XNOBJECT_NAME_LEN))
        {
            obj->magX = bson_iter_double(&iter);
            module->updated_input_properties |= magX;
            continue;
        }
        if(!strncmp(key, "magY", XNOBJECT_NAME_LEN))
        {
            obj->magY = bson_iter_double(&iter);
            module->updated_input_properties |= magY;
            continue;
        }
        if(!strncmp(key, "magZ", XNOBJECT_NAME_LEN))
        {
            obj->magZ = bson_iter_double(&iter);
            module->updated_input_properties |= magZ;
            continue;
        }
        if(!strncmp(key, "temperature", XNOBJECT_NAME_LEN))
        {
            obj->temperature = bson_iter_double(&iter);
            module->updated_input_properties |= temperature;
            continue;
        }
    }
    return 0;
}

// Helper function. Print structure GyroAccelMagTemp
void print_GyroAccelMagTemp(GyroAccelMagTemp_t* obj)
{
    printf("accelX=%lf\t", obj->accelX);
    printf("accelY=%lf\t", obj->accelY);
    printf("accelZ=%lf\t", obj->accelZ);
    printf("gyroX=%lf\t", obj->gyroX);
    printf("gyroY=%lf\t", obj->gyroY);
    printf("gyroZ=%lf\t", obj->gyroZ);
    printf("magX=%lf\t", obj->magX);
    printf("magY=%lf\t", obj->magY);
    printf("magZ=%lf\t", obj->magZ);
    printf("temperature=%lf\t", obj->temperature);
    printf("\n");
}

// Create module.
module_c_gy87_t* c_gy87_create(void *handle)
{
    module_c_gy87_t* module = calloc(1, sizeof(module_c_gy87_t));
    // Сохраним указатель на загруженную dll
    module->module_info.dll_handle = handle;
    module->module_info.out_objects = calloc(count_outs+1, sizeof(void *));
    module->module_info.out_objects[0]=&module->GyroAccelMagTemp;
    return module;
}

// Возвращает указатель на структуру выходного объекта, по имени пина
// Используется при подготовке списка полей, для мапинга объектов (для передачи в очередь)
out_object_t* get_outobject_by_outpin(module_c_gy87_t* module, char* name_out_pin, unsigned short* offset_field, unsigned short* index_port)
{
    (*offset_field) = 0;
    (*index_port) = 0;
    if(!strncmp(name_out_pin, "accelX", XNOBJECT_NAME_LEN))
    {
        (*offset_field) = (void*)&module->obj1_GyroAccelMagTemp.accelX - (void*)&module->obj1_GyroAccelMagTemp;
        (*index_port) = 0;
        return &module->GyroAccelMagTemp;
    }
    if(!strncmp(name_out_pin, "accelY", XNOBJECT_NAME_LEN))
    {
        (*offset_field) = (void*)&module->obj1_GyroAccelMagTemp.accelY - (void*)&module->obj1_GyroAccelMagTemp;
        (*index_port) = 1;
        return &module->GyroAccelMagTemp;
    }
    if(!strncmp(name_out_pin, "accelZ", XNOBJECT_NAME_LEN))
    {
        (*offset_field) = (void*)&module->obj1_GyroAccelMagTemp.accelZ - (void*)&module->obj1_GyroAccelMagTemp;
        (*index_port) = 2;
        return &module->GyroAccelMagTemp;
    }
    if(!strncmp(name_out_pin, "gyroX", XNOBJECT_NAME_LEN))
    {
        (*offset_field) = (void*)&module->obj1_GyroAccelMagTemp.gyroX - (void*)&module->obj1_GyroAccelMagTemp;
        (*index_port) = 3;
        return &module->GyroAccelMagTemp;
    }
    if(!strncmp(name_out_pin, "gyroY", XNOBJECT_NAME_LEN))
    {
        (*offset_field) = (void*)&module->obj1_GyroAccelMagTemp.gyroY - (void*)&module->obj1_GyroAccelMagTemp;
        (*index_port) = 4;
        return &module->GyroAccelMagTemp;
    }
    if(!strncmp(name_out_pin, "gyroZ", XNOBJECT_NAME_LEN))
    {
        (*offset_field) = (void*)&module->obj1_GyroAccelMagTemp.gyroZ - (void*)&module->obj1_GyroAccelMagTemp;
        (*index_port) = 5;
        return &module->GyroAccelMagTemp;
    }
    if(!strncmp(name_out_pin, "magX", XNOBJECT_NAME_LEN))
    {
        (*offset_field) = (void*)&module->obj1_GyroAccelMagTemp.magX - (void*)&module->obj1_GyroAccelMagTemp;
        (*index_port) = 6;
        return &module->GyroAccelMagTemp;
    }
    if(!strncmp(name_out_pin, "magY", XNOBJECT_NAME_LEN))
    {
        (*offset_field) = (void*)&module->obj1_GyroAccelMagTemp.magY - (void*)&module->obj1_GyroAccelMagTemp;
        (*index_port) = 7;
        return &module->GyroAccelMagTemp;
    }
    if(!strncmp(name_out_pin, "magZ", XNOBJECT_NAME_LEN))
    {
        (*offset_field) = (void*)&module->obj1_GyroAccelMagTemp.magZ - (void*)&module->obj1_GyroAccelMagTemp;
        (*index_port) = 8;
        return &module->GyroAccelMagTemp;
    }
    if(!strncmp(name_out_pin, "temperature", XNOBJECT_NAME_LEN))
    {
        (*offset_field) = (void*)&module->obj1_GyroAccelMagTemp.temperature - (void*)&module->obj1_GyroAccelMagTemp;
        (*index_port) = 9;
        return &module->GyroAccelMagTemp;
    }
    printf("Not found property \"%s\" among properties out objects\n", name_out_pin);
    return NULL;
}

// Stop and delete module. Free memory.
void c_gy87_delete(module_c_gy87_t* module)
{
    stop(module);
}

// Init module.
int c_gy87_init(module_c_gy87_t* module, const uint8_t* bson_data, uint32_t bson_len)
{
    // GyroAccelMagTemp
    // временное решение для указания размера выделяемой памяти под bson  объекты каждого типа
    // в реальности должны один раз создаваться тестовые bson объекты, вычисляться их размер и передаваться в функцию инициализации
    module->GyroAccelMagTemp.shmem_set.shmem_len = 300;
    module->GyroAccelMagTemp.obj1 = &module->obj1_GyroAccelMagTemp;
    module->GyroAccelMagTemp.obj2 = &module->obj2_GyroAccelMagTemp;
    module->GyroAccelMagTemp.obj2bson = (p_obj2bson)&GyroAccelMagTemp2bson;
    module->GyroAccelMagTemp.bson2obj = (p_bson2obj)&bson2GyroAccelMagTemp;
    module->GyroAccelMagTemp.print_obj = (p_print_obj)&print_GyroAccelMagTemp;

    module->module_info.get_outobj_by_outpin = (p_get_outobj_by_outpin)&get_outobject_by_outpin;
    module->module_info.input_data = NULL;

    module->module_info.func = &c_gy87_run;

    int res = init(&module->module_info, bson_data, bson_len);

    // для каждого типа порождаемого объекта инициализируется соответсвующая структура
    // и указываются буферы (для обмена данными между основным и передающим потоком)
    // GyroAccelMagTemp
    init_object_set(&module->GyroAccelMagTemp.shmem_set, module->module_info.instance_name, "GyroAccelMagTemp");

    return res;
}

int c_gy87_start(module_c_gy87_t* module)
{
    if (start(module) != 0)
        return -1;
    return 0;
}

/**
* @brief checkout4writer_GyroAccelMagTemp
* /~russian    Заполняет указатель адресом на структуру GyroAccelMagTemp_t,
*              которую можно заполнять данными для последующей передачи в разделяемую память
* @param obj
* @return
* /~russian 0 в случае успеха
*/
int checkout_GyroAccelMagTemp(module_c_gy87_t* module, GyroAccelMagTemp_t** obj)
{
    return checkout4writer(module, &module->GyroAccelMagTemp, obj);
}

/**
* @brief checkin4writer_GyroAccelMagTemp
* /~ Возвращает объект системе (данные будут переданы в разделяемую память)
* @param obj
* @return
*/
int checkin_GyroAccelMagTemp(module_c_gy87_t* module, GyroAccelMagTemp_t** obj)
{
    return checkin4writer(module, &module->GyroAccelMagTemp, obj);
}

