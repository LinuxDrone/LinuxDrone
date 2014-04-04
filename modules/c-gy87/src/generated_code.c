#include "../include/generated_code.h"

module_GY87_t module_GY87_info;

extern t_cycle_function c_gy87_run;

shmem_publisher_set_t* ar_publishers[3];

int c_gy87_init(const uint8_t* bson_data, uint32_t bson_len)
{
	ar_publishers[0]=&module_GY87_info.GyroAccelMagTemp;
	ar_publishers[1]=&module_GY87_info.Baro;
	ar_publishers[2]=NULL;

	module_GY87_info.module_info.shmem_publishers = ar_publishers;

	int res = init(&module_GY87_info.module_info, bson_data, bson_len);

	// временное решение для указания размера выделяемой памяти под bson  объекты каждого типа
	// в реальности должны один раз создаваться тестовые bson объекты, вычисляться их размер и передаваться в функцию инициализации
	module_GY87_info.GyroAccelMagTemp.shmem_len = 300;
	module_GY87_info.Baro.shmem_len = 300;

	// для каждого типа порождаемого объекта инициализируется соответсвующая структура
	init_publisher_set(&module_GY87_info.GyroAccelMagTemp, module_GY87_info.module_info.instance_name, "GyroAccelMagTemp");
	init_publisher_set(&module_GY87_info.Baro, module_GY87_info.module_info.instance_name, "Baro");

	module_GY87_info.module_info.func = &c_gy87_run;

	return res;
}

int c_gy87_start()
{
	if (start(&module_GY87_info.module_info) != 0)
		return -1;
	return 0;
}
