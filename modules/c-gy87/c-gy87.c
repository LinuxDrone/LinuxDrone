#include "c-gy87.h"
#include "c-gy87.helper.h"
#include "../../../services/i2c/client/i2c_client.h"


#define I2CBusName "/dev/i2c-1"




/**
 * @brief get_mpu6050_id Проверяет, что на шине действительно чмп mpu6050
 * Считывает число из регистра 0x75 идентифицирцющее чип mpu6050
 * Считаееое число должно ровняться 0x68
 * @return true в случае удачной проверки
 */
bool check_mpu6050_id(i2c_service_t* i2c_service, int i2c_session)
{
    char* data;
    int ret_len;

    int res = read_i2c(i2c_service, i2c_session, MPU6050_I2C_ADDR, MpuRegs_WhoAmI, 1, &data, &ret_len);
    if(res<0)
    {
        print_i2c_error(res);
        return false;
    }


    if(ret_len>0)
    {
        printf("mpuId = 0x%02X\n", *data);
        // Check magic number for mpu6050
        if(*data == 0x68)
            return true;
    }

    return false;
}


bool configureRanges(i2c_service_t* i2c_service, int i2c_session)
{
    //	CGY87_SCALE_2000_DEG, CGY87_ACCEL_8G, CGY87_LOWPASS_256_HZ
    char cmd = MpuFilter_Lowpass256Hz;
    int res = write_i2c(i2c_service, i2c_session, MPU6050_I2C_ADDR, MpuRegs_DlpfCfg, sizeof(char), &cmd);
    if(res<0)
    {
        printf("Error set MpuRegs_DlpfCfg to MpuFilter_Lowpass256Hz:\t");
        print_i2c_error(res);
        return false;
    }
    rt_task_sleep(rt_timer_ns2ticks(10000000));


    // Sample rate divider, chosen upon digital filtering settings
    cmd = 11;
    res = write_i2c(i2c_service, i2c_session, MPU6050_I2C_ADDR, MpuRegs_SmplRtDiv, sizeof(char), &cmd);
    if(res<0)
    {
        printf("Error set MpuRegs_SmplRtDiv to 11:\t");
        print_i2c_error(res);
        return false;
    }
    rt_task_sleep(rt_timer_ns2ticks(10000000));


    // Gyro range
    cmd = MpuRange_Scale2000Deg;
    res = write_i2c(i2c_service, i2c_session, MPU6050_I2C_ADDR, MpuRegs_GyroCfg, sizeof(char), &cmd);
    if(res<0)
    {
        printf("Error set MpuRegs_GyroCfg to MpuRange_Scale2000Deg:\t");
        print_i2c_error(res);
        return false;
    }
    rt_task_sleep(rt_timer_ns2ticks(10000000));


    // Set the accel range
    cmd = MpuAccelrange_Accel8G;
    res = write_i2c(i2c_service, i2c_session, MPU6050_I2C_ADDR, MpuRegs_AccellCfg, sizeof(char), &cmd);
    if(res<0)
    {
        printf("Error set MpuRegs_AccellCfg to MpuAccelrange_Accel8G:\t");
        print_i2c_error(res);
        return false;
    }
    rt_task_sleep(rt_timer_ns2ticks(10000000));

    return true;
}


bool initMpu(i2c_service_t* i2c_service, int i2c_session)
{
    if(!check_mpu6050_id(i2c_service, i2c_session))
    {
        printf("Not found chip MPU6050 on bus %s\n", I2CBusName);
        rt_task_sleep(rt_timer_ns2ticks(200000000));
        return false;
    }


    // Reset chip
    char cmd = CGY87_PWRMGMT_IMU_RST;
    int res = write_i2c(i2c_service, i2c_session, MPU6050_I2C_ADDR, MpuRegs_PwrMgmt, sizeof(char), &cmd);
    if(res<0)
    {
        printf("Error reset chip CGY87_PWRMGMT_IMU_RST:\t");
        print_i2c_error(res);
        return false;
    }

    rt_task_sleep(rt_timer_ns2ticks(50000000));

    // Reset chip and fifo
    cmd = CGY87_USERCTL_GYRO_RST | CGY87_USERCTL_SIG_COND | CGY87_USERCTL_FIFO_RST;
    res = write_i2c(i2c_service, i2c_session, MPU6050_I2C_ADDR, MpuRegs_UserCtrl, sizeof(char), &cmd);
    if(res<0)
    {
        printf("Error attempt reset chip and fifo:\t");
        print_i2c_error(res);
        return false;
    }

    rt_task_sleep(rt_timer_ns2ticks(50000000));

    // Power management configuration
    cmd = CGY87_PWRMGMT_PLL_X_CLK;
    res = write_i2c(i2c_service, i2c_session, MPU6050_I2C_ADDR, MpuRegs_PwrMgmt, sizeof(char), &cmd);
    if(res<0)
    {
        printf("Error attempt Power management CGY87_PWRMGMT_PLL_X_CLK:\t");
        print_i2c_error(res);
        return false;
    }


    if(configureRanges(i2c_service, i2c_session)<0)
    {
        return false;
    }


    // Interrupt configuration
    cmd = 0;
    res = write_i2c(i2c_service, i2c_session, MPU6050_I2C_ADDR, MpuRegs_UserCtrl, sizeof(char), &cmd);
    if(res<0)
    {
        printf("Error attempt Interrupt configuration:\t");
        print_i2c_error(res);
        return false;
    }
    rt_task_sleep(rt_timer_ns2ticks(10000000));


    // When I2C_BYPASS_EN is equal to 1 and I2C_MST_EN (Register 106 bit[5]) is equal to 0, the host
    // application processor will be able to directly access the auxiliary I2C bus of the MPU-60X0
    cmd = CGY87_I2C_BYPASS_EN;
    res = write_i2c(i2c_service, i2c_session, MPU6050_I2C_ADDR, MpuRegs_IntCfg, sizeof(char), &cmd);
    if(res<0)
    {
        printf("Error attempt Interrupt configuration:\t");
        print_i2c_error(res);
        return false;
    }
    rt_task_sleep(rt_timer_ns2ticks(10000000));

    return true;
}




/**
 * @brief c_gy87_run Функция рабочего потока модуля
 * @param module Указатель на структуру модуля
 */
void c_gy87_run (module_c_gy87_t *module)
{
    int i2c_session=0;
    i2c_service_t i2c_service;
    memset(&i2c_service, 0, sizeof(i2c_service_t));
    bool mpu_initialized = false;
    static const size_t size = CGY87_GYRO_Z_OUT_LSB - CGY87_ACCEL_X_OUT_MSB + 1;
    char* data;
    int ret_len;

    long last_print_time = rt_timer_read();
    int count_reads=0;
    long print_period = rt_timer_ns2ticks(1000000000);

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


            if(!initMpu(&i2c_service, i2c_session))
            {
                printf("Error init MPU6050 on bus %s\n", I2CBusName);
                rt_task_sleep(rt_timer_ns2ticks(200000000));
                continue;
            }
            mpu_initialized=true;
        }


        int res = read_i2c(&i2c_service, i2c_session, MPU6050_I2C_ADDR, CGY87_ACCEL_X_OUT_MSB, size, &data, &ret_len);
        if(res<0)
        {
            print_i2c_error(res);
            continue;
        }

        // unpack received data
        GyroAccelMagTemp_t* GyroAccelMagTemp;
        checkout_GyroAccelMagTemp(module, &GyroAccelMagTemp);

        GyroAccelMagTemp->gyroX = data[CGY87_GYRO_X_OUT_MSB - CGY87_ACCEL_X_OUT_MSB] << 8 | data[CGY87_GYRO_X_OUT_LSB - CGY87_ACCEL_X_OUT_MSB];
        GyroAccelMagTemp->gyroY = data[CGY87_GYRO_Y_OUT_MSB - CGY87_ACCEL_X_OUT_MSB] << 8 | data[CGY87_GYRO_Y_OUT_LSB - CGY87_ACCEL_X_OUT_MSB];
        GyroAccelMagTemp->gyroZ = data[CGY87_GYRO_Z_OUT_MSB - CGY87_ACCEL_X_OUT_MSB] << 8 | data[CGY87_GYRO_Z_OUT_LSB - CGY87_ACCEL_X_OUT_MSB];
        GyroAccelMagTemp->accelX = data[CGY87_ACCEL_X_OUT_MSB - CGY87_ACCEL_X_OUT_MSB] << 8 | data[CGY87_ACCEL_X_OUT_LSB - CGY87_ACCEL_X_OUT_MSB];
        GyroAccelMagTemp->accelY = data[CGY87_ACCEL_Y_OUT_MSB - CGY87_ACCEL_X_OUT_MSB] << 8 | data[CGY87_ACCEL_Y_OUT_LSB - CGY87_ACCEL_X_OUT_MSB];
        GyroAccelMagTemp->accelZ = data[CGY87_ACCEL_Z_OUT_MSB - CGY87_ACCEL_X_OUT_MSB] << 8 | data[CGY87_ACCEL_Z_OUT_LSB - CGY87_ACCEL_X_OUT_MSB];
        GyroAccelMagTemp->temperature = data[CGY87_TEMP_OUT_MSB - CGY87_ACCEL_X_OUT_MSB] << 8 | data[CGY87_TEMP_OUT_LSB - CGY87_ACCEL_X_OUT_MSB];

        //print_GyroAccelMagTemp(GyroAccelMagTemp);

        checkin_GyroAccelMagTemp(module, &GyroAccelMagTemp);
    }
}
