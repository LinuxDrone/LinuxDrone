#ifndef GENERATED_CODE_H_
#define GENERATED_CODE_H_

#include "module-fuctions.h"

typedef struct {
	int accelX;
	int accelY;
	int accelZ;
	int gyroX;
	int gyroY;
	int gyroZ;
	int magX;
	int magY;
	int magZ;
	int temperature;
} GyroAccelMagTemp_t;

typedef struct {
	int pressure;
} Baro_t;




typedef struct {
	module_t module_info;

	shmem_publisher_set_t  GyroAccelMagTemp;
	shmem_publisher_set_t  Baro;
} module_GY87_t;



#endif
