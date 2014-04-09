#include "../include/generated_code.h"

// количество типов выходных объектов
#define count_shmem_sets 2

extern t_cycle_function c_gy87_run;

//shmem_publisher_set_t* ar_publishers[3];

int GyroAccelMagTemp2bson(GyroAccelMagTemp_t* obj, bson_t* bson)
{
    bson_append_int32 (bson, "accelX", 6, obj->accelX);
    bson_append_int32 (bson, "accelY", 6, obj->accelY);
    bson_append_int32 (bson, "accelZ", 6, obj->accelZ);
    bson_append_int32 (bson, "gyroX", 5, obj->gyroX);
    bson_append_int32 (bson, "gyroY", 5, obj->gyroY);
    bson_append_int32 (bson, "gyroZ", 5, obj->gyroZ);
    bson_append_int32 (bson, "magX", 4, obj->magX);
    bson_append_int32 (bson, "magY", 4, obj->magY);
    bson_append_int32 (bson, "magZ", 4, obj->magZ);
    bson_append_int32 (bson, "temperature", 11, obj->temperature);

    return 0;
}

int bson2GyroAccelMagTemp(module_t* module, bson_t* bson)
{
    if(!module || !module->input_data || !bson)
    {
        printf("Error: func bson2GyroAccelMagTemp, NULL parameter\n");
        return -1;
    }
    GyroAccelMagTemp_t* obj = (GyroAccelMagTemp_t*)module->input_data;
    bson_iter_t iter;

    if(!bson_iter_init (&iter, bson))
    {
        printf("Error: func bson2GyroAccelMagTemp, bson_iter_init \n");
        return -1;
    }

    while(bson_iter_next(&iter))
    {
        const char* key = bson_iter_key (&iter);
        //printf("key=%s\tval=%i\n", key, bson_iter_int32(&iter));

        if(!strncmp(key, "accelX", 100))
        {
            obj->accelX = bson_iter_int32(&iter);
            module->updated_input_properties |= accelX;
            continue;
        }
        if(!strncmp(key, "accelY", 100))
        {
            obj->accelY = bson_iter_int32(&iter);
            module->updated_input_properties |= accelY;
            continue;
        }
        if(!strncmp(key, "accelZ", 100))
        {
            obj->accelZ = bson_iter_int32(&iter);
            module->updated_input_properties |= accelZ;
            continue;
        }
        if(!strncmp(key, "gyroX", 100))
        {
            obj->gyroX = bson_iter_int32(&iter);
            module->updated_input_properties |= gyroX;
            continue;
        }
        if(!strncmp(key, "gyroY", 100))
        {
            obj->gyroY = bson_iter_int32(&iter);
            module->updated_input_properties |= gyroY;
            continue;
        }
        if(!strncmp(key, "gyroZ", 100))
        {
            obj->gyroZ = bson_iter_int32(&iter);
            module->updated_input_properties |= gyroZ;
            continue;
        }
        if(!strncmp(key, "magX", 100))
        {
            obj->magX = bson_iter_int32(&iter);
            module->updated_input_properties |= magX;
            continue;
        }
        if(!strncmp(key, "magY", 100))
        {
            obj->magY = bson_iter_int32(&iter);
            module->updated_input_properties |= magY;
            continue;
        }
        if(!strncmp(key, "magZ", 100))
        {
            obj->magZ = bson_iter_int32(&iter);
            module->updated_input_properties |= magZ;
            continue;
        }
        if(!strncmp(key, "temperature", 100))
        {
            obj->temperature = bson_iter_int32(&iter);
            module->updated_input_properties |= temperature;
            continue;
        }
    }

    return 0;
}

void printGyroAccelMagTemp(GyroAccelMagTemp_t* obj)
{
    printf("accelX=%i  ", obj->accelX);
    printf("accelY=%i  ", obj->accelY);
    printf("accelZ=%i  ", obj->accelZ);
    printf("gyroX=%i  ", obj->gyroX);
    printf("gyroY=%i  ", obj->gyroY);
    printf("gyroZ=%i  ", obj->gyroZ);
    printf("magX=%i  ", obj->magX);
    printf("magY=%i  ", obj->magY);
    printf("magZ=%i  ", obj->magZ);
    printf("temperature=%i\n\n", obj->temperature);
}



module_GY87_t* c_gy87_create()
{
    module_GY87_t* module = malloc(sizeof(module_GY87_t));

    module->module_info.shmem_sets = malloc(sizeof(void *) * (count_shmem_sets+1));
    module->module_info.shmem_sets[0]=&module->GyroAccelMagTemp;
    module->module_info.shmem_sets[1]=&module->Baro;
    module->module_info.shmem_sets[2]=NULL;

    return module;
}


void c_gy87_delete(module_GY87_t* module)
{
    free(module->module_info.shmem_sets);
    free(module);
}


int c_gy87_init(module_GY87_t* module, const uint8_t* bson_data, uint32_t bson_len)
{
    int res = init(&module->module_info, bson_data, bson_len);

	// временное решение для указания размера выделяемой памяти под bson  объекты каждого типа
	// в реальности должны один раз создаваться тестовые bson объекты, вычисляться их размер и передаваться в функцию инициализации
    module->GyroAccelMagTemp.shmem_len = 300;
    module->Baro.shmem_len = 300;

	// для каждого типа порождаемого объекта инициализируется соответсвующая структура
    // и указываются буферы (для обмена данными между основным и передающим потоком)
    init_publisher_set(&module->GyroAccelMagTemp, module->module_info.instance_name, "GyroAccelMagTemp");
    module->GyroAccelMagTemp.obj1 = &module->obj1_GyroAccelMagTemp;
    module->GyroAccelMagTemp.obj2 = &module->obj2_GyroAccelMagTemp;
    module->GyroAccelMagTemp.obj2bson = (p_obj2bson)&GyroAccelMagTemp2bson;
    module->GyroAccelMagTemp.bson2obj = (p_bson2obj)&bson2GyroAccelMagTemp;
    module->GyroAccelMagTemp.print_obj = (p_print_obj)&printGyroAccelMagTemp;

    init_publisher_set(&module->Baro, module->module_info.instance_name, "Baro");
    module->Baro.obj1 = &module->obj1_Baro;
    module->Baro.obj2 = &module->obj2_Baro;

    memset(&module->input4modul, 0, sizeof(GyroAccelMagTemp_t));
    module->module_info.input_data = &module->input4modul;
    module->module_info.input_bson2obj = (p_bson2obj)&bson2GyroAccelMagTemp;

    module->module_info.func = &c_gy87_run;

    return res;
}


int c_gy87_start(module_GY87_t* module)
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
int checkout_GyroAccelMagTemp(module_GY87_t* module, GyroAccelMagTemp_t** obj)
{
    return checkout4writer(module, &module->GyroAccelMagTemp, obj);
}


/**
 * @brief checkin4writer_GyroAccelMagTemp
 * /~ Возвращает объект системе (данные будут переданы в разделяемую память)
 * @param obj
 * @return
 */
int checkin_GyroAccelMagTemp(module_GY87_t* module, GyroAccelMagTemp_t** obj)
{
    return checkin4writer(module, &module->GyroAccelMagTemp, obj);
}


