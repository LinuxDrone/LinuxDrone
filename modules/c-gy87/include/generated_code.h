#ifndef GENERATED_CODE_H_
#define GENERATED_CODE_H_

#include "../../../libraries/c-sdk/include/module-functions.h"

// Enum and Structure for output GyroAccelMagTemp
typedef enum
{
    accelX =        0b00000000000000000000000000000001,
    accelY =        0b00000000000000000000000000000010,
    accelZ =        0b00000000000000000000000000000100,
    gyroX =         0b00000000000000000000000000001000,
    gyroY =         0b00000000000000000000000000010000,
    gyroZ =         0b00000000000000000000000000100000,
    magX =          0b00000000000000000000000001000000,
    magY =          0b00000000000000000000000010000000,
    magZ =          0b00000000000000000000000100000000,
    temperature =   0b00000000000000000000001000000000
} fields_GyroAccelMagTemp_t;


typedef struct
{
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


// Enum and Structure for output Baro
typedef enum
{
    pressure =        0b00000000000000000000000000000001
} fields_Baro_t;


typedef struct
{
	int pressure;
} Baro_t;



typedef struct {
	module_t module_info;

    // может не быть если объект без входа
    GyroAccelMagTemp_t input4modul;

	shmem_publisher_set_t  GyroAccelMagTemp;
	shmem_publisher_set_t  Baro;

    GyroAccelMagTemp_t obj1_GyroAccelMagTemp;
    GyroAccelMagTemp_t obj2_GyroAccelMagTemp;

    Baro_t obj1_Baro;
    Baro_t obj2_Baro;
} module_GY87_t;



module_GY87_t* c_gy87_create();

int c_gy87_init(module_GY87_t* module, const uint8_t* bson_data, uint32_t bson_len);

int c_gy87_start();

void c_gy87_delete(module_GY87_t* module);

#endif
