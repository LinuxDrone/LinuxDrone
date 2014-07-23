#include "c-ms5611.h"
#include "c-ms5611.helper.h"
#include "../../../services/i2c/client/i2c_client.h"
#include "math.h"

#define I2CBusName "/dev/i2c-1"

bool init_ms5611(i2c_service_t* i2c_service, int i2c_session, uint16_t *C);
void refreshTemperature(i2c_service_t* i2c_service, int i2c_session, uint8_t OSR);
void refreshPressure(i2c_service_t* i2c_service, int i2c_session, uint8_t OSR);
uint32_t readTemperature(i2c_service_t* i2c_service, int i2c_session);
uint32_t readPressure(i2c_service_t* i2c_service, int i2c_session);
void calculatePressureAndTemperature(uint32_t D1, uint32_t D2,  uint16_t *C, float *TEMP, float *PRES);


/**
 * @brief c_ms5611_run Функция рабочего потока модуля
 * @param module Указатель на структуру модуля
 */
void c_ms5611_run (module_c_ms5611_t *module)
{
    int i2c_session=0;
    i2c_service_t i2c_service;

    memset(&i2c_service, 0, sizeof(i2c_service_t));

    bool mpu_initialized = false;

    //	Калибровочные коэффициенты
	uint16_t C[6];

    uint32_t D2; // RawTemperature
    uint32_t D1; // RawPressure
    float Pressure;
    float Temperature;

    uint8_t OSR = MS5611_OVERSAMPLING;

    while(1) {
        get_input_data(&module->module_info);

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

            if(!init_ms5611(&i2c_service, i2c_session, C))
            {
                printf("Error init MS5611 on bus %s\n", I2CBusName);
                rt_task_sleep(rt_timer_ns2ticks(200000000));
                continue;
            }

            rt_task_sleep(rt_timer_ns2ticks(20000000));
            mpu_initialized=true;
        }
        /* Start pressure conversion */
        refreshPressure(&i2c_service, i2c_session, OSR);
        // Waiting for pressure data ready
        rt_task_sleep(rt_timer_ns2ticks(10000000));
        /* Read the pressure conversion */
        D1 = readPressure(&i2c_service, i2c_session);

        /* Start temperature conversion */
        refreshTemperature(&i2c_service, i2c_session, OSR);
        // Waiting for temperature data ready
        rt_task_sleep(rt_timer_ns2ticks(10000000));
        /* Read the temperature conversion */
        D2 = readTemperature(&i2c_service, i2c_session);

        calculatePressureAndTemperature(D1, D2, C, &Temperature, &Pressure);

        printf("Temperature=%f   |   Pressure=%f\n", Temperature, Pressure);

        BaroTemp_t* BaroTemp;
        checkout_BaroTemp(module, &BaroTemp);

		// Temperature в градусах цельсия
		// Pressure    в килопаскалях
		// PressureHg  в милиметрах ртутного столба
		// Altitude    в метрах

        BaroTemp->Temperature = Temperature;
        BaroTemp->Pressure = Pressure/1000.0f;
        BaroTemp->PressureHg = (Pressure * 760.0f) / 101325.0f;
        BaroTemp->Altitude = (44330.0f * (1.00f - powf((Pressure/101325.0f), 0.190295f)));

        checkin_BaroTemp(module, &BaroTemp);
    }
}



/**
 * @brief init_ms5611 Инициализация датчика давления ms5611
 * Считывает калибровочные коэффициенты из датчика
 * @return true в случае отсутствия ошибок
 */
bool init_ms5611(i2c_service_t* i2c_service, int i2c_session, uint16_t *C)
{
	/* Reset MS5611 */
    char cmd = 0;

    int res = write_cmd_i2c(i2c_service, i2c_session, MS5611_I2C_ADDR, MS5611_RESET);
	if(res<0)
	{
		printf("Error Reset MS5611:\t");
		print_i2c_error(res);
		return false;
	}
	rt_task_sleep(rt_timer_ns2ticks(10000000));

    char *Data;
    int ret_len;

    int i;
	/* Read all 12 bytes of calibration data in one transfer, this is a very optimized way of doing things */
    for (i = 0; i < MS5611_CALIB_LEN; i++) {
        res = read_i2c(i2c_service, i2c_session, MS5611_I2C_ADDR, MS5611_CALIB_ADDR + i*2, 2, &Data, &ret_len);
        if(res<0 | ret_len != 2) {
			print_i2c_error(res);
			return false;
		}

        C[i] = (Data[0] << 8) | Data[1];
        printf("C[%i]=%i\n",i,C[i]);
    }

/*    for (i = 0; i < 12; i++) {
        res = read_i2c(i2c_service, i2c_session, MS5611_I2C_ADDR, MS5611_CALIB_ADDR, 12, &Data, &ret_len);
        if(res<0 | ret_len != 12) {
			print_i2c_error(res);
			return false;
		}

        C[i] = (Data[i*2] << 8) | Data[i*2+1];
        printf("C[%i]=%i\n",i,C[i]);
    }
*/
    return true;
}

/** Initiate the process of temperature measurement
 * @param OSR value
 */
void refreshTemperature(i2c_service_t* i2c_service, int i2c_session, uint8_t OSR) {	

	int  res = 0;
    char cmd = 0;

    res = write_cmd_i2c(i2c_service, i2c_session, MS5611_I2C_ADDR, MS5611_TEMP_ADDR + OSR);
	if(res<0)
	{
		printf("Error refreshTemperature MS5611:\t");
		print_i2c_error(res);
        //return false;
	}
}

/** Read temperature value
 */
uint32_t readTemperature(i2c_service_t* i2c_service, int i2c_session) {

    char *Data;
    int ret_len;

    int res = read_i2c(i2c_service, i2c_session, MS5611_I2C_ADDR, MS5611_ADC_READ, 3, &Data, &ret_len);
    if(res<0 | ret_len != 3)
    {
        print_i2c_error(res);
        //return false;
    }
    
	return (Data[0] << 16) | (Data[1] << 8) | Data[2];
}

/** Initiate the process of pressure measurement
 * @param OSR value
 */
void refreshPressure(i2c_service_t* i2c_service, int i2c_session, uint8_t OSR) {

	int  res = 0;
    char cmd = 0;

    res = write_cmd_i2c(i2c_service, i2c_session, MS5611_I2C_ADDR, MS5611_PRES_ADDR + OSR);
	if(res<0)
	{
		printf("Error refreshPressure MS5611:\t");
		print_i2c_error(res);
        //return false;
	}
}

/** Read pressure value
 */
uint32_t readPressure(i2c_service_t* i2c_service, int i2c_session) {

    char *Data;
    int ret_len;

    int res = read_i2c(i2c_service, i2c_session, MS5611_I2C_ADDR, MS5611_ADC_READ, 3, &Data, &ret_len);
    if(res<0 | ret_len != 3)
    {
        print_i2c_error(res);
        //return false;
    }
	return (Data[0] << 16) | (Data[1] << 8) | Data[2];
}

/** Calculate temperature and pressure calculations and perform compensation
 *  More info about these calculations is available in the datasheet.
 */
void calculatePressureAndTemperature(uint32_t D1, uint32_t D2,  uint16_t *C, float *TEMP, float *PRES) {
	
    float dT = (float)D2 - (float)C[4] * pow(2, 8);
    *TEMP = (2000.0f + ((dT * (float)C[5]) / pow(2, 23)));
    float OFF = (float)C[1] * pow(2, 16) + ((float)C[3] * dT) / pow(2, 7);
    float SENS = (float)C[0] * pow(2, 15) + ((float)C[2] * dT) / pow(2, 8);

    float T2, OFF2, SENS2;

    if (*TEMP >= 2000.0f)
    {			
        T2 = 0.0f;
        OFF2 = 0.0f;
        SENS2 = 0.0f;
    }
    if (*TEMP < 2000.0f && *TEMP >= -1500.0f)
    {		
        T2 = dT * dT / pow(2, 31);
        OFF2 = 5.0f * pow(*TEMP - 2000.0f, 2) / 2.0f;
        SENS2 = OFF2 / 2;
    }
    if (*TEMP < -1500)
    {			
        OFF2 = OFF2 + 7.0f * pow(*TEMP + 1500.0f, 2);
        SENS2 = SENS2 + 11.0f * pow(*TEMP + 1500.0f, 2) / 2.0f;
    }	

    *TEMP = *TEMP - T2;
    OFF = OFF - OFF2;
    SENS = SENS - SENS2;

    *PRES = ((D1 * SENS) / pow(2, 21) - OFF) / pow(2, 15);
    *TEMP = *TEMP / 100;
}

