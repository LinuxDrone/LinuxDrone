#include "../include/c_gy87.helper.h"

void c_gy87_run (module_c_gy87_t *module)
{
    while(1) {
        get_input_data(module);



        GyroAccelMagTemp_t* GyroAccelMagTemp;
        checkout_GyroAccelMagTemp(module, &GyroAccelMagTemp);
        GyroAccelMagTemp->accelX = 0;
        checkin_GyroAccelMagTemp(module, &GyroAccelMagTemp);
    }
}
