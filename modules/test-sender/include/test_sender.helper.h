#pragma once

#include "../../../libraries/c-sdk/include/module-functions.h"

// Enum and Structure for port Output1
typedef enum
{
	char_out =	0b00000000000000000000000000000001,
	short_out =	0b00000000000000000000000000000010,
	int_out =	0b00000000000000000000000000000100,
	long_out =	0b00000000000000000000000000001000,
	long_long_out =	0b00000000000000000000000000010000,
	float_out =	0b00000000000000000000000000100000,
	double_out =	0b00000000000000000000000001000000,
	string_out =	0b00000000000000000000000010000000
} fields_Output1_t;

typedef struct
{
	char char_out;
	short short_out;
	int int_out;
	long long_out;
	long long long_long_out;
	float float_out;
	double double_out;
	const char* string_out;
} Output1_t;

// Enum and Structure for port Output2
typedef enum
{
	out3 =	0b00000000000000000000000000000001
} fields_Output2_t;

typedef struct
{
	int out3;
} Output2_t;


// Module Structure
typedef struct {
	module_t module_info;

	// набор данных для выхода Output1
	out_object_t  Output1;
	Output1_t obj1_Output1;
	Output1_t obj2_Output1;

	// набор данных для выхода Output2
	out_object_t  Output2;
	Output2_t obj1_Output2;
	Output2_t obj2_Output2;
} module_test_sender_t;


// Helper functions
module_test_sender_t* test_sender_create();
int test_sender_init(module_test_sender_t* module, const uint8_t* bson_data, uint32_t bson_len);
int test_sender_start();
void test_sender_delete(module_test_sender_t* module);

