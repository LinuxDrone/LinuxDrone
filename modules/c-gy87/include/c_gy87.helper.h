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
	float accelX;
	float accelY;
	float accelZ;
	float gyroX;
	float gyroY;
	float gyroZ;
	float magX;
	float magY;
	float magZ;
	float temperature;
} GyroAccelMagTemp_t;


// Module Structure
typedef struct {
	module_t module_info;

	// набор данных для выхода GyroAccelMagTemp
	out_object_t  GyroAccelMagTemp;
	GyroAccelMagTemp_t obj1_GyroAccelMagTemp;
	GyroAccelMagTemp_t obj2_GyroAccelMagTemp;
} module_c_gy87_t;


// Helper functions
module_c_gy87_t* c_gy87_create();
int c_gy87_init(module_c_gy87_t* module, const uint8_t* bson_data, uint32_t bson_len);
int c_gy87_start();
void c_gy87_delete(module_c_gy87_t* module);

