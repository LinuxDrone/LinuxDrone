#include <stdio.h>
#include "../include/generated_code.h"

extern module_GY87_t module_GY87_info;

void c_gy87_run (void *cookie)
{
	int i=0;
    for (;;) {
    	Reason4callback res = get_input_data(&module_GY87_info.module_info);
    	//printf("c-gu87 %i\n", i++);
    }
}


