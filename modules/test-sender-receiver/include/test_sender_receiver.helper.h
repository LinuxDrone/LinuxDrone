#pragma once

#include "../../../libraries/c-sdk/include/module-functions.h"

// Enum and Structure for port input
typedef enum
{
	in1 =	0b00000000000000000000000000000001,
	in2 =	0b00000000000000000000000000000010
} fields_input_t;

typedef struct
{
	float in1;
	float in2;
} input_t;

// Enum and Structure for port Output1
typedef enum
{
	out1 =	0b00000000000000000000000000000001,
	out2 =	0b00000000000000000000000000000010
} fields_Output1_t;

typedef struct
{
	float out1;
	float out2;
} Output1_t;

// Enum and Structure for port Output2
typedef enum
{
	out3 =	0b00000000000000000000000000000001
} fields_Output2_t;

typedef struct
{
	float out3;
} Output2_t;


// Module Structure
typedef struct {
	module_t module_info;
	input_t input4modul;

	// набор данных для выхода Output1
	out_object_t  Output1;
	Output1_t obj1_Output1;
	Output1_t obj2_Output1;

	// набор данных для выхода Output2
	out_object_t  Output2;
	Output2_t obj1_Output2;
	Output2_t obj2_Output2;
} module_test_sender_receiver_t;


// Helper functions
module_test_sender_receiver_t* test_sender_receiver_create();
int test_sender_receiver_init(module_test_sender_receiver_t* module, const uint8_t* bson_data, uint32_t bson_len);
int test_sender_receiver_start();
void test_sender_receiver_delete(module_test_sender_receiver_t* module);

