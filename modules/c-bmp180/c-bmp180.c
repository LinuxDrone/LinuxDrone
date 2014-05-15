#include "c-bmp180.h"
#include "c-bmp180.helper.h"
#include "../../../services/i2c/client/i2c_client.h"
#include "math.h"

#define I2CBusName "/dev/i2c-1"

// Структура калибровочных коэффициентов
typedef struct {
    int16_t  AC1;
    int16_t  AC2;
    int16_t  AC3;
    uint16_t AC4;
    uint16_t AC5;
    uint16_t AC6;
    int16_t  B1;
    int16_t  B2;
    int16_t  MB;
    int16_t  MC;
    int16_t  MD;
} BMP180CalibDataTypeDef;

// Калибровочные данные
BMP180CalibDataTypeDef CalibData;

uint8_t  m_oversampling;

/**
 * @brief check_bmp180_id Проверяет, что на шине действительно bmp180
 * Считывает число из регистра 0x75 идентифицирующее чип bmp180
 * Считанное число должно ровняться 0x68
 * @return true в случае удачной проверки
 */
bool check_bmp180_id(i2c_service_t* i2c_service, int i2c_session)
{
/*  char* data;
    int ret_len;

    int res = read_i2c(i2c_service, i2c_session, BMP180_I2C_ADDR, Regs, 1, &data, &ret_len);
    if(res<0)
    {
        print_i2c_error(res);
        return false;
    }


    if(ret_len>0)
    {
        printf("mpuId = 0x%02X\n", *data);
        // Check magic number for BMP180
        if(*data == 0x68)
            return true;
    }

    return false;
*/
    return true;
}

/**
 * @brief init_bmp180 Инициализация датчика давления bmp180
 * Считывает калибровочные коэффициенты из датчика
 * И сохраняет в структуре CalibData
 * @return true в случае отсутствия ошибок
 */
bool init_bmp180(i2c_service_t* i2c_service, int i2c_session)
{
    if(!check_bmp180_id(i2c_service, i2c_session))
    {
        printf("Not found chip BMP180 on bus %s\n", I2CBusName);
        rt_task_sleep(rt_timer_ns2ticks(200000000));
        return false;
    }

    m_oversampling = (BMP180_OVERSAMPLING << 6);
    char *Data;

    int ret_len;

    /* Read all 22 bytes of calibration data in one transfer, this is a very optimized way of doing things */
    //int len_requested_data = BMP180_CALIB_LEN;
    int res = read_i2c(i2c_service, i2c_session, BMP180_I2C_ADDR, BMP180_CALIB_ADDR, BMP180_CALIB_LEN, &Data, &ret_len);
    if(res<0 | ret_len != BMP180_CALIB_LEN)
    {
        print_i2c_error(res);
        return false;
    }

    /* Parameters AC1-AC6 */
    CalibData.AC1 = (Data[0] << 8) | Data[1];
    CalibData.AC2 = (Data[2] << 8) | Data[3];
    CalibData.AC3 = (Data[4] << 8) | Data[5];
    CalibData.AC4 = (Data[6] << 8) | Data[7];
    CalibData.AC5 = (Data[8] << 8) | Data[9];
    CalibData.AC6 = (Data[10] << 8) | Data[11];

    /* Parameters B1, B2 */
    CalibData.B1  = (Data[12] << 8) | Data[13];
    CalibData.B2  = (Data[14] << 8) | Data[15];

    /* Parameters MB, MC, MD */
    CalibData.MB  = (Data[16] << 8) | Data[17];
    CalibData.MC  = (Data[18] << 8) | Data[19];
    CalibData.MD  = (Data[20] << 8) | Data[21];

    rt_task_sleep(rt_timer_ns2ticks(50000000));

    return true;
}


/**
 * @brief c_bmp180_run Функция рабочего потока модуля
 * @param module Указатель на структуру модуля
 */
void c_bmp180_run (module_c_bmp180_t *module)
{
    int i2c_session=0;
    i2c_service_t i2c_service;

    memset(&i2c_service, 0, sizeof(i2c_service_t));

    bool mpu_initialized = false;

    char* data;
    int ret_len;

    int  res = 0;
    char cmd = 0;

    long last_print_time = rt_timer_read();
    int count_reads=0;
    long print_period = rt_timer_ns2ticks(1000000000);

    //char Data[3];
    char *Data;

    /* Straight from the datasheet */
    int32_t X1, X2, X3, B3, B5, B6, P;
    uint32_t B4, B7;

    uint16_t RawTemperature;
    uint32_t RawPressure;
    uint32_t Pressure;
    uint16_t Temperature;

    while(1) {
        get_input_data(module);

        count_reads++;
        if(rt_timer_read() - last_print_time > print_period)
        {
            //printf("count_reads=%i", count_reads);

            last_print_time = rt_timer_read();
            count_reads=0;
        }

        if(!mpu_initialized)
        {
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


            if(!init_bmp180(&i2c_service, i2c_session))
            {
                printf("Error init BMP180 on bus %s\n", I2CBusName);
                rt_task_sleep(rt_timer_ns2ticks(200000000));
                continue;
            }
            mpu_initialized=true;
        }


        /* Start temperature conversion */
        cmd = BMP180_TEMP_ADDR;
        res = write_i2c(&i2c_service, i2c_session, BMP180_I2C_ADDR, BMP180_CTRL_ADDR, sizeof(char), &cmd);
        if(res<0)
        {
            printf("Error start temperature conversion:\t");
            print_i2c_error(res);
            continue;
        }
        rt_task_sleep(rt_timer_ns2ticks(5000000));

        /* Read the temperature conversion */
        res = read_i2c(&i2c_service, i2c_session, BMP180_I2C_ADDR, BMP180_ADC_MSB, 2, &Data, &ret_len);
        if(res<0 | ret_len != 2)
        {
            printf("Error read the temperature conversion:\t");
            print_i2c_error(res);
            continue;
        }

        RawTemperature = (((uint8_t)Data[0] << 8) | (uint8_t)Data[1]);

        X1 = (RawTemperature - CalibData.AC6) * CalibData.AC5 >> 15;
        X2 = ((int32_t)CalibData.MC << 11) / (X1 + CalibData.MD);
        B5 = X1 + X2;
        Temperature = (B5 + 8) >> 4;

        /* Start pressure conversion */
        cmd = BMP180_PRES_ADDR + m_oversampling;
        res = write_i2c(&i2c_service, i2c_session, BMP180_I2C_ADDR, BMP180_CTRL_ADDR, sizeof(char), &cmd);
        if(res<0)
        {
            printf("Error start pressure conversion:\t");
            print_i2c_error(res);
            continue;
        }
        rt_task_sleep(rt_timer_ns2ticks(26000000));

        /* Read the pressure conversion */
        res = read_i2c(&i2c_service, i2c_session, BMP180_I2C_ADDR, BMP180_ADC_MSB, 3, &Data, &ret_len);
        if(res<0  | ret_len != 3)
        {
            printf("Error read the pressure conversion:\t");
            print_i2c_error(res);
            continue;
        }

        RawPressure = (((uint8_t)Data[0] << 16) | ((uint8_t)Data[1] << 8) | (uint8_t)Data[2]) >> (8 - BMP180_OVERSAMPLING);

        B6 = B5 - 4000;
        X1 = (CalibData.B2 * (B6 * B6 >> 12)) >> 11;
        X2 = CalibData.AC2 * B6 >> 11;
        X3 = X1 + X2;
        B3 = ((((int32_t)CalibData.AC1 * 4 + X3) << BMP180_OVERSAMPLING) + 2) >> 2;
        X1 = CalibData.AC3 * B6 >> 13;
        X2 = (CalibData.B1 * (B6 * B6 >> 12)) >> 16;
        X3 = ((X1 + X2) + 2) >> 2;
        B4 = (CalibData.AC4 * (uint32_t)(X3 + 32768)) >> 15;
        B7 = ((uint32_t)RawPressure - B3) * (50000 >> BMP180_OVERSAMPLING);
        P  = B7 < 0x80000000 ? (B7 * 2) / B4 : (B7 / B4) * 2;

        X1 = (P >> 8) * (P >> 8);
        X1 = (X1 * 3038) >> 16;
        X2 = (-7357 * P) >> 16;
        Pressure = P + ((X1 + X2 + 3791) >> 4);

        BaroTemp_t* BaroTemp;
        checkout_BaroTemp(module, &BaroTemp);


        BaroTemp->Temperature = Temperature/10.0f;
        BaroTemp->Pressure = Pressure/1000.0f;
        BaroTemp->PressureHg = (Pressure * 760.0f) / 101325.0f;
        BaroTemp->Altitude = (44330.0f * (1.00f - powf((Pressure/101325.0f), 0.190295f)));

        checkin_BaroTemp(module, &BaroTemp);

    }
}
