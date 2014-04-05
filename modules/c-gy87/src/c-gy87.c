#include "../include/generated_code.h"

void c_gy87_run (module_GY87_t *module)
{
    int cycle=0;
    while(1) {
    	ReciveResult res = get_input_data(module);

        GyroAccelMagTemp_t* objGAMT;
        checkout_GyroAccelMagTemp(&objGAMT);

        objGAMT->accelX = cycle;

        checkin_GyroAccelMagTemp(&objGAMT);

        cycle++;
    }
}


