#pragma once

#include "../../../libraries/c-sdk/include/module-functions.h"

// Enum and Structure for port input
typedef enum
{
	pwm0 =	0b00000000000000000000000000000001,
	pwm1 =	0b00000000000000000000000000000010,
	pwm2 =	0b00000000000000000000000000000100,
	pwm3 =	0b00000000000000000000000000001000,
	pwm4 =	0b00000000000000000000000000010000,
	pwm5 =	0b00000000000000000000000000100000,
	pwm6 =	0b00000000000000000000000001000000,
	pwm7 =	0b00000000000000000000000010000000,
	pwm8 =	0b00000000000000000000000100000000,
	pwm9 =	0b00000000000000000000001000000000,
	pwm10 =	0b00000000000000000000010000000000,
	pwm11 =	0b00000000000000000000100000000000
} fields_input_t;

typedef struct
{
	int pwm0;
	int pwm1;
	int pwm2;
	int pwm3;
	int pwm4;
	int pwm5;
	int pwm6;
	int pwm7;
	int pwm8;
	int pwm9;
	int pwm10;
	int pwm11;
} input_t;

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
	input_t input4modul;

	// набор данных для выхода GyroAccelMagTemp
	shmem_publisher_set_t  GyroAccelMagTemp;
	GyroAccelMagTemp_t obj1_GyroAccelMagTemp;
	GyroAccelMagTemp_t obj2_GyroAccelMagTemp;

	// набор данных для выхода Baro
	shmem_publisher_set_t  Baro;
	Baro_t obj1_Baro;
	Baro_t obj2_Baro;
} module_c_gy87_t;


// Helper functions
module_c_gy87_t* c_gy87_create();
int c_gy87_init(module_c_gy87_t* module, const uint8_t* bson_data, uint32_t bson_len);
int c_gy87_start();
void c_gy87_delete(module_c_gy87_t* module);

