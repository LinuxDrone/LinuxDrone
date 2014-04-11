#include "../include/test-sender.h"

void test_sender_run (module_GY87_t *module)
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
        checkout_GyroAccelMagTemp(module, &objGAMT);
        objGAMT->accelX = cycle;
        objGAMT->accelY = cycle*2+1;
        objGAMT->accelZ = 0;
        objGAMT->gyroX = 0;
        objGAMT->gyroY = 0;
        objGAMT->gyroZ = 0;
        objGAMT->magX = 0;
        objGAMT->magY = 0;
        objGAMT->magZ = 0;
        objGAMT->temperature = 0;
        checkin_GyroAccelMagTemp(module, &objGAMT);

        // Скажем какие данные следует добыть из разделяемой памяти, если они не придут через трубу
        module->module_info.refresh_input_mask = accelX | accelY | accelZ;

        // Наглое считывание данных из разделяемой памяти
        //int res = refresh_input(module);

        cycle++;
    }
}


