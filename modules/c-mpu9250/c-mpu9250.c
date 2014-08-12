//--------------------------------------------------------------------
// This file was created as a part of the LinuxDrone project:
//                http://www.linuxdrone.org
//
// This work is based on the original code from the Qiyong Mu (kylongmu@msn.com)
// See https://mbed.org/users/kylongmu/code/MPU9250_SPI/docs/tip/
//
// Distributed under the Creative Commons Attribution-ShareAlike 4.0
// International License (see accompanying License.txt file or a copy
// at http://creativecommons.org/licenses/by-sa/4.0/legalcode)
//
// The human-readable summary of (and not a substitute for) the
// license: http://creativecommons.org/licenses/by-sa/4.0/
//--------------------------------------------------------------------

#include "c-mpu9250.h"
#include "c-mpu9250.helper.h"
#include "../../../services/spi/client/spi_client.h"
#include "math.h"

// Структура хранит параметры и данные модуля
struct mpu9250Config {
    spi_service_t spi_service;
    int spi_session;
    const char *SPIBusName;

    unsigned int speed_hz;
    unsigned char bits_per_word;
    unsigned short delay_usecs;

    bool mpu_initialized;

    int sample_rate_div;
    int low_pass_filter;

    float acc_divider;
    float gyro_divider;

    int calib_data[3];
    float magnetometer_ASA[3];

    float temperature;
    float accelerometer_data[3];
    float gyroscope_data[3];
    float magnetometer_data[3];
};

bool init_mpu9250(struct mpu9250Config *cfg);
bool testConnection(struct mpu9250Config *cfg);
bool set_gyro_scale(struct mpu9250Config *cfg, char scale);
bool set_acc_scale(struct mpu9250Config *cfg, char scale);
bool calib_mag(struct mpu9250Config *cfg);
bool calib_acc(struct mpu9250Config *cfg);
bool read_all(struct mpu9250Config *cfg);
// Загрузка параметров из базы.
bool settingsLoad(struct mpu9250Config *cfg, params_c_mpu9250_t *params);



/**
 * @brief c_mpu9250_run Функция рабочего потока модуля
 * @param module Указатель на структуру модуля
 */
void c_mpu9250_run (module_c_mpu9250_t *module)
{
    struct mpu9250Config cfg_mpu9250;

    memset(&cfg_mpu9250, 0, sizeof(struct mpu9250Config));

    cfg_mpu9250.SPIBusName      = "/dev/spidev0.1";
    cfg_mpu9250.spi_session     = -1004;
    cfg_mpu9250.mpu_initialized = false;

    cfg_mpu9250.speed_hz       = 1000000;
    cfg_mpu9250.bits_per_word  = 8;
    cfg_mpu9250.delay_usecs    = 0;


    while(1) {
        get_input_data(&module->module_info);

        if(!cfg_mpu9250.mpu_initialized)
        {
            settingsLoad(&cfg_mpu9250, &module->params_c_mpu9250);

            // Открываем сессию с spi шиной
            if(cfg_mpu9250.spi_session<0)
            {
                // Устанавливаем соединение с сервисом
                if(!cfg_mpu9250.spi_service.connected)
                {
                    connect_spi_service(&cfg_mpu9250.spi_service);
                    continue;
                }

                cfg_mpu9250.spi_session = open_spi(
                            &cfg_mpu9250.spi_service,
                            cfg_mpu9250.SPIBusName,
                            cfg_mpu9250.speed_hz,
                            cfg_mpu9250.bits_per_word,
                            cfg_mpu9250.delay_usecs);
                continue;
            }

            if(!init_mpu9250(&cfg_mpu9250))
            {
                printf("Error init mpu9250 on bus %s\n", cfg_mpu9250.SPIBusName);
                rt_task_sleep(rt_timer_ns2ticks(200000000));
                continue;
            }

            cfg_mpu9250.mpu_initialized=true;
        }

        // Чтение данных с датчика
        read_all(&cfg_mpu9250);

        printf("test temperature %f\n", cfg_mpu9250.temperature);
        printf("test accelZ %f\n", cfg_mpu9250.accelerometer_data[2]);

        rt_task_sleep(rt_timer_ns2ticks(100000000));

        AGMT_t* AGMT;
        checkout_AGMT(module, &AGMT);

        AGMT->accelX        = cfg_mpu9250.accelerometer_data[0];
        AGMT->accelY        = cfg_mpu9250.accelerometer_data[1];
        AGMT->accelZ        = cfg_mpu9250.accelerometer_data[2];
        AGMT->gyroX         = cfg_mpu9250.gyroscope_data[0];
        AGMT->gyroY         = cfg_mpu9250.gyroscope_data[1];
        AGMT->gyroZ         = cfg_mpu9250.gyroscope_data[2];
        AGMT->magX          = cfg_mpu9250.magnetometer_data[0];
        AGMT->magY          = cfg_mpu9250.magnetometer_data[1];
        AGMT->magZ          = cfg_mpu9250.magnetometer_data[2];
        AGMT->temperature   = cfg_mpu9250.temperature;

        checkin_AGMT(module, &AGMT);
    }
}



/**
 * @brief Чтение параметров модуля из базы MongoDB
 * @param cfg    - Указатель на структуру с локальными параметрами и переменными модуля
 * @param params - Указатель на структуру с параметрами модуля из базы MongoDB
 * @return true  - Если успешное завершение чтения данных из базы
 */
bool settingsLoad(struct mpu9250Config *cfg, params_c_mpu9250_t *params)
{
    cfg->SPIBusName    = params->SPI_Device;
    cfg->low_pass_filter = BITS_DLPF_CFG_188HZ;

    return true;
}


/**
 * @brief Проверка связи с датчиком MPU9250 на шине spi
 * @param cfg   - Указатель на структуру с локальными параметрами и переменными модуля
 * @return true - if mpu9250 answers.
 */
bool testConnection(struct mpu9250Config *cfg)
{
    char res_data;
    int res = read_byte_spi(&cfg->spi_service, cfg->spi_session, MPUREG_WHOAMI|READ_FLAG, &res_data);
    if(res<0 | res_data != 0x71)
    {
        printf("Error connection MPU9250:\t");
        print_spi_error(res);
        return false;
    }
    else return true;
}


/**
 * @brief init_mpu9250 Инициализация 9 осевого датчика ориентации mpu9250
 * @brief call this function at startup, giving the sample rate divider (raging from 0 to 255) and
          low pass filter value; suitable values are
 * @param cfg   - Указатель на структуру с локальными параметрами и переменными модуля
 * @return true - Успешная инициализация датчика
 */
bool init_mpu9250(struct mpu9250Config *cfg)
{
    // Проверка соединения с датчиком MPU9250
    if (!testConnection(cfg)) return false;

    #define MPU_InitRegNum 17
    uint8_t i = 0;

    // Подготовка массива с данными для инициализации датчика
    uint8_t MPU_Init_Data[MPU_InitRegNum][2] = {
        {0x80, MPUREG_PWR_MGMT_1},          // Reset Device
        {0x01, MPUREG_PWR_MGMT_1},          // Clock Source
        {0x00, MPUREG_PWR_MGMT_2},          // Enable Acc & Gyro
        {cfg->low_pass_filter, MPUREG_CONFIG},   // Use DLPF set Gyroscope bandwidth 184Hz, temperature bandwidth 188Hz
        {0x18, MPUREG_GYRO_CONFIG},         // +-2000dps
        {0x08, MPUREG_ACCEL_CONFIG},        // +-4G
        {0x09, MPUREG_ACCEL_CONFIG_2},      // Set Acc Data Rates, Enable Acc LPF , Bandwidth 184Hz
        {0x30, MPUREG_INT_PIN_CFG},         //
        //{0x40, MPUREG_I2C_MST_CTRL},      // I2C Speed 348 kHz
        //{0x20, MPUREG_USER_CTRL},         // Enable AUX
        {0x20, MPUREG_USER_CTRL},           // I2C Master mode
        {0x0D, MPUREG_I2C_MST_CTRL},        // I2C configuration multi-master  IIC 400KHz

        {AK8963_I2C_ADDR, MPUREG_I2C_SLV0_ADDR},  //Set the I2C slave addres of AK8963 and set for write.
        //{0x09, MPUREG_I2C_SLV4_CTRL},
        //{0x81, MPUREG_I2C_MST_DELAY_CTRL}, //Enable I2C delay

        {AK8963_CNTL2, MPUREG_I2C_SLV0_REG}, //I2C slave 0 register address from where to begin data transfer
        {0x01, MPUREG_I2C_SLV0_DO},         // Reset AK8963
        {0x81, MPUREG_I2C_SLV0_CTRL},       //Enable I2C and set 1 byte

        {AK8963_CNTL1, MPUREG_I2C_SLV0_REG}, //I2C slave 0 register address from where to begin data transfer
        {0x12, MPUREG_I2C_SLV0_DO},         // Register value to continuous measurement in 16bit
        {0x81, MPUREG_I2C_SLV0_CTRL}        //Enable I2C and set 1 byte

    };

    for(i=0; i<MPU_InitRegNum; i++) {
        int res = write_byte_spi(&cfg->spi_service, cfg->spi_session, MPU_Init_Data[i][1], MPU_Init_Data[i][0]);
        if(res<0) {
            print_spi_error(res);
            return false;
        }
        //I2C must slow down the write speed, otherwise it won't work
        rt_task_sleep(rt_timer_ns2ticks(10000000));
    }

    set_acc_scale(cfg, BITS_FS_16G);
    set_gyro_scale(cfg, BITS_FS_2000DPS);

    calib_mag(cfg);

    return true;
}



/**
 * @brief ACCELEROMETER SCALE
 * @param cfg   - Указатель на структуру с локальными параметрами и переменными модуля
 * @param scale - call this function at startup, after initialization, to set the right range for the
                  accelerometers. Suitable ranges are:
                    BITS_FS_2G
                    BITS_FS_4G
                    BITS_FS_8G
                    BITS_FS_16G
 * @return true - Успешная передача данных в датчик
 */
bool set_acc_scale(struct mpu9250Config *cfg, char scale)
{
    int res;

    res = write_byte_spi(&cfg->spi_service, cfg->spi_session, MPUREG_ACCEL_CONFIG, scale);

    if(res<0) {
        print_spi_error(res);
        return false;
    }

    switch (scale){
        case BITS_FS_2G:
            cfg->acc_divider=16384;
        break;
        case BITS_FS_4G:
            cfg->acc_divider=8192;
        break;
        case BITS_FS_8G:
            cfg->acc_divider=4096;
        break;
        case BITS_FS_16G:
            cfg->acc_divider=2048;
        break;
    }

    return true;
}



/**
 * @brief GYROSCOPE SCALE
 * @param cfg   - Указатель на структуру с локальными параметрами и переменными модуля
 * @param scale - call this function at startup, after initialization, to set the right range for the
                  gyroscopes. Suitable ranges are:
                    BITS_FS_250DPS
                    BITS_FS_500DPS
                    BITS_FS_1000DPS
                    BITS_FS_2000DPS
 * @return true - Успешная передача данных в датчик
 */
bool set_gyro_scale(struct mpu9250Config *cfg, char scale)
{
    int res;

    res = write_byte_spi(&cfg->spi_service, cfg->spi_session, MPUREG_GYRO_CONFIG, scale);

    if(res<0) {
        print_spi_error(res);
        return false;
    }

    switch (scale){
        case BITS_FS_250DPS:
            cfg->gyro_divider=131;
        break;
        case BITS_FS_500DPS:
            cfg->gyro_divider=65.5;
        break;
        case BITS_FS_1000DPS:
            cfg->gyro_divider=32.8;
        break;
        case BITS_FS_2000DPS:
            cfg->gyro_divider=16.4;
        break;
    }

    return true;
}

/**
 * @brief Калибровка магнитометра AK8963 на шине I2C в датчике MPU9250
 * @param cfg   - Указатель на структуру с локальными параметрами и переменными модуля
 * @return true - Успешная калибровка датчика
 */
bool calib_mag(struct mpu9250Config *cfg){

    char res_data[3];
    int res;

    float data;
    int i;

    //Set the I2C slave addres of AK8963 and set for read.
    res = write_byte_spi(&cfg->spi_service, cfg->spi_session, MPUREG_I2C_SLV0_ADDR, AK8963_I2C_ADDR|READ_FLAG);
    if(res<0) {
        print_spi_error(res);
        return false;
    }
    //I2C slave 0 register address from where to begin data transfer
    res = write_byte_spi(&cfg->spi_service, cfg->spi_session, MPUREG_I2C_SLV0_REG, AK8963_ASAX);
    if(res<0) {
        print_spi_error(res);
        return false;
    }
    //Read 3 bytes from the magnetometer
    res = write_byte_spi(&cfg->spi_service, cfg->spi_session, MPUREG_I2C_SLV0_CTRL, 0x83);
    if(res<0) {
        print_spi_error(res);
        return false;
    }

    rt_task_sleep(rt_timer_ns2ticks(10000000));

    //Read I2C
    res = read_bytes_spi(&cfg->spi_service, cfg->spi_session, MPUREG_EXT_SENS_DATA_00|READ_FLAG, res_data, 3);
    if(res<0)
    {
        printf("Error connection MPU9250:\t");
        print_spi_error(res);
        return false;
    }

    //Read I2C
    for(i=0; i<3; i++) {
        data=res_data[i];
        cfg->magnetometer_ASA[i]=((data-128)/256+1)*Magnetometer_Sensitivity_Scale_Factor;
    }
}



/**
 * @brief READ ACCELEROMETER CALIBRATION
 * @param cfg   - Указатель на структуру с локальными параметрами и переменными модуля
 * @return true - Успешная калибровка датчика
 */
bool calib_acc(struct mpu9250Config *cfg)
{
    uint8_t res_data[4];
    char temp_scale;

    //READ CURRENT ACC SCALE
    int res = read_byte_spi(&cfg->spi_service, cfg->spi_session, MPUREG_ACCEL_CONFIG|READ_FLAG, &temp_scale);
    if(res<0)
    {
        print_spi_error(res);
        return false;
    }

    set_acc_scale(cfg, BITS_FS_8G);

    //ENABLE SELF TEST need modify
    //temp_scale=WriteReg(MPUREG_ACCEL_CONFIG, 0x80>>axis);

    res = read_bytes_spi(&cfg->spi_service, cfg->spi_session, MPUREG_SELF_TEST_X|READ_FLAG, res_data, 4);
    if(res<0)
    {
        print_spi_error(res);
        return false;
    }
    cfg->calib_data[0]=((res_data[0]&11100000)>>3)|((res_data[3]&00110000)>>4);
    cfg->calib_data[1]=((res_data[1]&11100000)>>3)|((res_data[3]&00001100)>>2);
    cfg->calib_data[2]=((res_data[2]&11100000)>>3)|((res_data[3]&00000011));

    set_acc_scale(cfg, temp_scale);
}




/**
 * @brief Чтение всех текущих данных по ориентации датчика в пространстве
 *          (акселерометр, гироскоп, магнитометр, температура)
 * @param cfg   - Указатель на структуру с локальными параметрами и переменными модуля
 * @return true - Успешное чтение
 */
bool read_all(struct mpu9250Config *cfg){
    uint8_t res_data[21];
    int res;
    int16_t bit_data;
    float data;
    int i;

    //Send I2C command at first
    //Set the I2C slave addres of AK8963 and set for read.
    res = write_byte_spi(&cfg->spi_service, cfg->spi_session, MPUREG_I2C_SLV0_ADDR, AK8963_I2C_ADDR|READ_FLAG);
    if(res<0) {
        print_spi_error(res);
        return false;
    }
    //I2C slave 0 register address from where to begin data transfer
    res = write_byte_spi(&cfg->spi_service, cfg->spi_session, MPUREG_I2C_SLV0_REG, AK8963_HXL);
    if(res<0) {
        print_spi_error(res);
        return false;
    }
    //Read 7 bytes from the magnetometer
    res = write_byte_spi(&cfg->spi_service, cfg->spi_session, MPUREG_I2C_SLV0_CTRL, 0x87);
    if(res<0) {
        print_spi_error(res);
        return false;
    }
    //must start your read from AK8963A register 0x03 and read seven bytes so that upon read of ST2 register 0x09 the AK8963A will unlatch the data registers for the next measurement.

    //wait(0.001);

    res = read_bytes_spi(&cfg->spi_service, cfg->spi_session, MPUREG_ACCEL_XOUT_H|READ_FLAG, res_data, 21);
    if(res<0)
    {
        printf("Error connection MPU9250:\t");
        print_spi_error(res);
        return false;
    }
    //Get accelerometer value
    for(i=0; i<3; i++) {
        bit_data=((int16_t)res_data[i*2]<<8)|res_data[i*2+1];
        data=(float)bit_data;
        cfg->accelerometer_data[i]=10.0f*data/cfg->acc_divider;
    }

    //Get temperature
    bit_data=((int16_t)res_data[i*2]<<8)|res_data[i*2+1];
    data=(float)bit_data;
    cfg->temperature=((data-21)/333.87)+21;

    //Get gyroscop value
    for(i=4; i<7; i++) {
        bit_data=((int16_t)res_data[i*2]<<8)|res_data[i*2+1];
        data=(float)bit_data;
        cfg->gyroscope_data[i-4]=data/cfg->gyro_divider;
    }

    //Get Magnetometer value
    for(i=7; i<10; i++) {
        bit_data=((int16_t)res_data[i*2+1]<<8)|res_data[i*2];
        data=(float)bit_data;
        cfg->magnetometer_data[i-7]=data*cfg->magnetometer_ASA[i-7];
    }

    return true;
}

