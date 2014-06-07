#include "rx-openlrs.helper.h"
#include "../../../services/i2c/client/i2c_client.h"

#define I2CBusName          "/dev/i2c-2"
#define OpenLRS_I2C_ADDR    0x16       // The I2C address of the Reciever
#define COUNT_CHANNEL 16    // Количество каналов считываемых из приемника

void rx_openlrs_run (module_rx_openlrs_t *module)
{
    int i2c_session=0;
    i2c_service_t i2c_service;
    memset(&i2c_service, 0, sizeof(i2c_service_t));
    char* data;
    int len_read = COUNT_CHANNEL*2;
    int ret_len;
    uint16_t* ch_data;

    while(1) {
        get_input_data((module_t*)module);

        // Открываем сессию с i2c шиной
        if(i2c_session<1)
        {
            // Устанавливаем соединение с сервисом
            if(!i2c_service.connected)
            {
                connect_i2c_service(&i2c_service);
                continue;
            }

            i2c_session = open_i2c(&i2c_service, I2CBusName);
            continue;
        }


        int res = read_i2c(&i2c_service, i2c_session, OpenLRS_I2C_ADDR, 0, len_read, &data, &ret_len);
        if(res<0)
        {
            print_i2c_error(res);
            continue;
        }

        ch_data = (uint16_t*)data;


        RC_out_t* mRC_out_t;

        checkout_RC_out(module, &mRC_out_t);

        mRC_out_t->ch1 = ch_data[0] + 1000;
        mRC_out_t->ch2 = ch_data[1] + 1000;
        mRC_out_t->ch3 = ch_data[2] + 1000;
        mRC_out_t->ch4 = ch_data[3] + 1000;
        mRC_out_t->ch5 = ch_data[4] + 1000;
        mRC_out_t->ch6 = ch_data[5] + 1000;
        mRC_out_t->ch7 = ch_data[6] + 1000;
        mRC_out_t->ch8 = ch_data[7] + 1000;

        checkin_RC_out(module, &mRC_out_t);
    }
}
