#include "../include/generated_code.h"

void c_gy87_run (module_GY87_t *module)
{
    int cycle=0;
    while(1) {
        get_input_data(module);

        // проверим, обновились ли данные
        if(module->module_info.updated_input_properties!=0)
        {
            // есть новые данные
        }
        else
        {
            // вышел таймаут
        }


        GyroAccelMagTemp_t* objGAMT;
        checkout_GyroAccelMagTemp(&objGAMT);
        objGAMT->accelX = cycle;
        objGAMT->accelY = cycle*2+1;
        checkin_GyroAccelMagTemp(&objGAMT);

        module->module_info.refresh_input_mask = accelX | accelY | accelZ;

        //int res = refresh_input(module);

        cycle++;
    }
}


