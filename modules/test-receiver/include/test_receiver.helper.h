#pragma once

#include "../../../libraries/c-sdk/include/module-functions.h"

// Enum and Structure for port input
typedef enum
{
	in1 =	0b00000000000000000000000000000001,
	in2 =	0b00000000000000000000000000000010,
	in3 =	0b00000000000000000000000000000100,
	in4 =	0b00000000000000000000000000001000
} fields_input_t;

typedef struct
{
	float in1;
	float in2;
	float in3;
	float in4;
} input_t;


// Module Structure
typedef struct {
	module_t module_info;
	input_t input4modul;
} module_test_receiver_t;


// Helper functions
module_test_receiver_t* test_receiver_create();
int test_receiver_init(module_test_receiver_t* module, const uint8_t* bson_data, uint32_t bson_len);
int test_receiver_start();
void test_receiver_delete(module_test_receiver_t* module);

