#include <stdio.h>
#include "module-fuctions.h"

module_t module_info;

void task_main_body (void *cookie)
{
	int i=0;
    for (;;) {
    	Reason4callback res = get_input_data(&module_info);
    	//printf("c-gu87 %i\n", i++);
    }
}


int c_gy87_init(const uint8_t* bson_data, uint32_t bson_len) {

	module_info.func = &task_main_body;

	return init(&module_info, bson_data, bson_len);
}


int c_gy87_start()
{
	if (start(&module_info) != 0)
		return -1;
	return 0;
}
