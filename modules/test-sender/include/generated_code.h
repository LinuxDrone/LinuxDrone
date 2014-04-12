#pragma once

#include "../../../libraries/c-sdk/include/module-functions.h"

// Enum and Structure for port GyroAccelMagTemp
typedef enum
{
	accelX =	0b00000000000000000000000000000001,
	accelY =	0b00000000000000000000000000000010,
	accelZ =	0b00000000000000000000000000000100,
	gyroX =	0b00000000000000000000000000001000,
	gyroY =	0b00000000000000000000000000010000,
	gyroZ =	0b00000000000000000000000000100000,
	magX =	0b00000000000000000000000001000000,
	magY =	0b00000000000000000000000010000000,
	magZ =	0b00000000000000000000000100000000,
	temperature =	0b00000000000000000000001000000000
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

// Enum and Structure for port Baro
typedef enum
{
	pressure =	0b00000000000000000000000000000001
} fields_Baro_t;

typedef struct
{
	int pressure;
} Baro_t;


// Module Structure
typedef struct {
	module_t module_info;

	// набор данных для выхода GyroAccelMagTemp
	shmem_publisher_set_t  GyroAccelMagTemp;
	GyroAccelMagTemp_t obj1_GyroAccelMagTemp;
	GyroAccelMagTemp_t obj2_GyroAccelMagTemp;

	// набор данных для выхода Baro
	shmem_publisher_set_t  Baro;
	Baro_t obj1_Baro;
	Baro_t obj2_Baro;
} module_test_sender_t;


// Helper functions
module_test_sender_t* test_sender_create();
int test_sender_init(module_test_sender_t* module, const uint8_t* bson_data, uint32_t bson_len);
int test_sender_start();
void test_sender_delete(module_test_sender_t* module);

