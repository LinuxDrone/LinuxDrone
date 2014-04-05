#include "../include/generated_code.h"

module_GY87_t module_GY87_info;

extern t_cycle_function c_gy87_run;

shmem_publisher_set_t* ar_publishers[3];

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


int c_gy87_init(const uint8_t* bson_data, uint32_t bson_len)
{
	ar_publishers[0]=&module_GY87_info.GyroAccelMagTemp;
	ar_publishers[1]=&module_GY87_info.Baro;
	ar_publishers[2]=NULL;

	module_GY87_info.module_info.shmem_sets = ar_publishers;

	int res = init(&module_GY87_info.module_info, bson_data, bson_len);

	// временное решение для указания размера выделяемой памяти под bson  объекты каждого типа
	// в реальности должны один раз создаваться тестовые bson объекты, вычисляться их размер и передаваться в функцию инициализации
	module_GY87_info.GyroAccelMagTemp.shmem_len = 300;
	module_GY87_info.Baro.shmem_len = 300;

	// для каждого типа порождаемого объекта инициализируется соответсвующая структура
    // и указываются буферы (для обмена данными между основным и передающим потоком)
	init_publisher_set(&module_GY87_info.GyroAccelMagTemp, module_GY87_info.module_info.instance_name, "GyroAccelMagTemp");
    module_GY87_info.GyroAccelMagTemp.obj1 = &module_GY87_info.obj1_GyroAccelMagTemp;
    module_GY87_info.GyroAccelMagTemp.obj2 = &module_GY87_info.obj2_GyroAccelMagTemp;
    module_GY87_info.GyroAccelMagTemp.obj2bson = (p_obj2bson)&GyroAccelMagTemp2bson;

	init_publisher_set(&module_GY87_info.Baro, module_GY87_info.module_info.instance_name, "Baro");
    module_GY87_info.Baro.obj1 = &module_GY87_info.obj1_Baro;
    module_GY87_info.Baro.obj2 = &module_GY87_info.obj2_Baro;


	module_GY87_info.module_info.func = &c_gy87_run;

	return res;
}

int c_gy87_start()
{
	if (start(&module_GY87_info) != 0)
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
int checkout_GyroAccelMagTemp(GyroAccelMagTemp_t** obj)
{
    return checkout4writer(&module_GY87_info, &module_GY87_info.GyroAccelMagTemp, obj);
}

/**
 * @brief checkin4writer_GyroAccelMagTemp
 * /~ Возвращает объект системе (данные будут переданы в разделяемую память)
 * @param obj
 * @return
 */
int checkin_GyroAccelMagTemp(GyroAccelMagTemp_t** obj)
{
    return checkin4writer(&module_GY87_info, &module_GY87_info.GyroAccelMagTemp, obj);
}

