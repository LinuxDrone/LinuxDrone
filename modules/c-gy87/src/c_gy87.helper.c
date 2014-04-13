#include "../include/c_gy87.helper.h"

// количество типов выходных объектов
#define count_shmem_sets 2

extern t_cycle_function c_gy87_run;

// Convert bson to structure input
int bson2input(module_t* module, bson_t* bson)
{
    if(!module || !module->input_data || !bson)
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

        if(!strncmp(key, "pwm0", 100))
        {
            obj->pwm0 = bson_iter_int32(&iter);
            module->updated_input_properties |= pwm0;
            continue;
        }
        if(!strncmp(key, "pwm1", 100))
        {
            obj->pwm1 = bson_iter_int32(&iter);
            module->updated_input_properties |= pwm1;
            continue;
        }
        if(!strncmp(key, "pwm2", 100))
        {
            obj->pwm2 = bson_iter_int32(&iter);
            module->updated_input_properties |= pwm2;
            continue;
        }
        if(!strncmp(key, "pwm3", 100))
        {
            obj->pwm3 = bson_iter_int32(&iter);
            module->updated_input_properties |= pwm3;
            continue;
        }
        if(!strncmp(key, "pwm4", 100))
        {
            obj->pwm4 = bson_iter_int32(&iter);
            module->updated_input_properties |= pwm4;
            continue;
        }
        if(!strncmp(key, "pwm5", 100))
        {
            obj->pwm5 = bson_iter_int32(&iter);
            module->updated_input_properties |= pwm5;
            continue;
        }
        if(!strncmp(key, "pwm6", 100))
        {
            obj->pwm6 = bson_iter_int32(&iter);
            module->updated_input_properties |= pwm6;
            continue;
        }
        if(!strncmp(key, "pwm7", 100))
        {
            obj->pwm7 = bson_iter_int32(&iter);
            module->updated_input_properties |= pwm7;
            continue;
        }
        if(!strncmp(key, "pwm8", 100))
        {
            obj->pwm8 = bson_iter_int32(&iter);
            module->updated_input_properties |= pwm8;
            continue;
        }
        if(!strncmp(key, "pwm9", 100))
        {
            obj->pwm9 = bson_iter_int32(&iter);
            module->updated_input_properties |= pwm9;
            continue;
        }
        if(!strncmp(key, "pwm10", 100))
        {
            obj->pwm10 = bson_iter_int32(&iter);
            module->updated_input_properties |= pwm10;
            continue;
        }
        if(!strncmp(key, "pwm11", 100))
        {
            obj->pwm11 = bson_iter_int32(&iter);
            module->updated_input_properties |= pwm11;
            continue;
        }
    }
    return 0;
}

// Helper function. Print structure input
void print_input(input_t* obj)
{
    printf("pwm0=%i\n", obj->pwm0);
    printf("pwm1=%i\n", obj->pwm1);
    printf("pwm2=%i\n", obj->pwm2);
    printf("pwm3=%i\n", obj->pwm3);
    printf("pwm4=%i\n", obj->pwm4);
    printf("pwm5=%i\n", obj->pwm5);
    printf("pwm6=%i\n", obj->pwm6);
    printf("pwm7=%i\n", obj->pwm7);
    printf("pwm8=%i\n", obj->pwm8);
    printf("pwm9=%i\n", obj->pwm9);
    printf("pwm10=%i\n", obj->pwm10);
    printf("pwm11=%i\n", obj->pwm11);
}

// Convert structure GyroAccelMagTemp to bson
int GyroAccelMagTemp2bson(GyroAccelMagTemp_t* obj, bson_t* bson)
{
	bson_append_int32 (bson, "accelX", -1, obj->accelX);
	bson_append_int32 (bson, "accelY", -1, obj->accelY);
	bson_append_int32 (bson, "accelZ", -1, obj->accelZ);
	bson_append_int32 (bson, "gyroX", -1, obj->gyroX);
	bson_append_int32 (bson, "gyroY", -1, obj->gyroY);
	bson_append_int32 (bson, "gyroZ", -1, obj->gyroZ);
	bson_append_int32 (bson, "magX", -1, obj->magX);
	bson_append_int32 (bson, "magY", -1, obj->magY);
	bson_append_int32 (bson, "magZ", -1, obj->magZ);
	bson_append_int32 (bson, "temperature", -1, obj->temperature);
	return 0;
}

// Convert bson to structure GyroAccelMagTemp
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
        printf("Error: func bson2GyroAccelMagTemp, bson_iter_init\n");
        return -1;
    }

    while(bson_iter_next(&iter))
    {
        const char* key = bson_iter_key (&iter);

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

// Helper function. Print structure GyroAccelMagTemp
void print_GyroAccelMagTemp(GyroAccelMagTemp_t* obj)
{
    printf("accelX=%i\n", obj->accelX);
    printf("accelY=%i\n", obj->accelY);
    printf("accelZ=%i\n", obj->accelZ);
    printf("gyroX=%i\n", obj->gyroX);
    printf("gyroY=%i\n", obj->gyroY);
    printf("gyroZ=%i\n", obj->gyroZ);
    printf("magX=%i\n", obj->magX);
    printf("magY=%i\n", obj->magY);
    printf("magZ=%i\n", obj->magZ);
    printf("temperature=%i\n", obj->temperature);
}

// Convert structure Baro to bson
int Baro2bson(Baro_t* obj, bson_t* bson)
{
	bson_append_int32 (bson, "pressure", -1, obj->pressure);
	return 0;
}

// Convert bson to structure Baro
int bson2Baro(module_t* module, bson_t* bson)
{
    if(!module || !module->input_data || !bson)
    {
        printf("Error: func bson2Baro, NULL parameter\n");
        return -1;
    }

    Baro_t* obj = (Baro_t*)module->input_data;
    bson_iter_t iter;
    if(!bson_iter_init (&iter, bson))
    {
        printf("Error: func bson2Baro, bson_iter_init\n");
        return -1;
    }

    while(bson_iter_next(&iter))
    {
        const char* key = bson_iter_key (&iter);

        if(!strncmp(key, "pressure", 100))
        {
            obj->pressure = bson_iter_int32(&iter);
            module->updated_input_properties |= pressure;
            continue;
        }
    }
    return 0;
}

// Helper function. Print structure Baro
void print_Baro(Baro_t* obj)
{
    printf("pressure=%i\n", obj->pressure);
}

// Create module.
module_c_gy87_t* c_gy87_create(void *handle)
{
    module_c_gy87_t* module = malloc(sizeof(module_c_gy87_t));
    // Сохраним указатель на загруженную dll
    module->module_info.dll_handle = handle;
    module->module_info.shmem_sets = malloc(sizeof(void *) * (count_shmem_sets+1));
    module->module_info.shmem_sets[0]=&module->GyroAccelMagTemp;
    module->module_info.shmem_sets[0]=&module->Baro;
    module->module_info.shmem_sets[2]=NULL;
    return module;
}

// Stop and delete module. Free memory.
void c_gy87_delete(module_c_gy87_t* module)
{
    stop(module);
}

// Init module.
int c_gy87_init(module_c_gy87_t* module, const uint8_t* bson_data, uint32_t bson_len)
{
    int res = init(&module->module_info, bson_data, bson_len);

    // GyroAccelMagTemp
    // временное решение для указания размера выделяемой памяти под bson  объекты каждого типа
    // в реальности должны один раз создаваться тестовые bson объекты, вычисляться их размер и передаваться в функцию инициализации
    module->GyroAccelMagTemp.shmem_len = 300;
    // для каждого типа порождаемого объекта инициализируется соответсвующая структура
    // и указываются буферы (для обмена данными между основным и передающим потоком)
    init_publisher_set(&module->GyroAccelMagTemp, module->module_info.instance_name, "GyroAccelMagTemp");
    module->GyroAccelMagTemp.obj1 = &module->obj1_GyroAccelMagTemp;
    module->GyroAccelMagTemp.obj2 = &module->obj2_GyroAccelMagTemp;
    module->GyroAccelMagTemp.obj2bson = (p_obj2bson)&GyroAccelMagTemp2bson;
    module->GyroAccelMagTemp.bson2obj = (p_bson2obj)&bson2GyroAccelMagTemp;
    module->GyroAccelMagTemp.print_obj = (p_print_obj)&print_GyroAccelMagTemp;

    // Baro
    // временное решение для указания размера выделяемой памяти под bson  объекты каждого типа
    // в реальности должны один раз создаваться тестовые bson объекты, вычисляться их размер и передаваться в функцию инициализации
    module->Baro.shmem_len = 300;
    // для каждого типа порождаемого объекта инициализируется соответсвующая структура
    // и указываются буферы (для обмена данными между основным и передающим потоком)
    init_publisher_set(&module->Baro, module->module_info.instance_name, "Baro");
    module->Baro.obj1 = &module->obj1_Baro;
    module->Baro.obj2 = &module->obj2_Baro;
    module->Baro.obj2bson = (p_obj2bson)&Baro2bson;
    module->Baro.bson2obj = (p_bson2obj)&bson2Baro;
    module->Baro.print_obj = (p_print_obj)&print_Baro;

    // Input
    memset(&module->input4modul, 0, sizeof(input_t));
    module->module_info.input_data = &module->input4modul;
    module->module_info.input_bson2obj = (p_bson2obj)&bson2input;

    module->module_info.func = &c_gy87_run;

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

/**
* @brief checkout4writer_Baro
* /~russian    Заполняет указатель адресом на структуру Baro_t,
*              которую можно заполнять данными для последующей передачи в разделяемую память
* @param obj
* @return
* /~russian 0 в случае успеха
*/
int checkout_Baro(module_c_gy87_t* module, Baro_t** obj)
{
    return checkout4writer(module, &module->Baro, obj);
}

/**
* @brief checkin4writer_Baro
* /~ Возвращает объект системе (данные будут переданы в разделяемую память)
* @param obj
* @return
*/
int checkin_Baro(module_c_gy87_t* module, Baro_t** obj)
{
    return checkin4writer(module, &module->Baro, obj);
}

