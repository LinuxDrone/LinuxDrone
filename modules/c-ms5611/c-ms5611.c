//--------------------------------------------------------------------
// This file was created as a part of the LinuxDrone project:
//                http://www.linuxdrone.org
//
// Distributed under the Creative Commons Attribution-ShareAlike 4.0
// International License (see accompanying License.txt file or a copy
// at http://creativecommons.org/licenses/by-sa/4.0/legalcode)
//
// The human-readable summary of (and not a substitute for) the
// license: http://creativecommons.org/licenses/by-sa/4.0/
//--------------------------------------------------------------------

#include "c-ms5611.h"
#include "c-ms5611.helper.h"
#include "../../../services/i2c/client/i2c_client.h"
#include "math.h"

// Структура хранит параметры модуля
struct ms5611Config {
    i2c_service_t i2c_service;
    int i2c_session;
	const char *I2CBusName;
	char i2c_address;

    bool baro_initialized;

	//	Калибровочные коэффициенты
	uint16_t C[6];
	uint8_t OSR;
	int     convTime;
	float   alfaFilt;
    bool calcPress0;
    bool initPress0;
    float startAltitude;
};

bool init_ms5611(struct ms5611Config *cfg);
void refreshTemperature(struct ms5611Config *cfg);
void refreshPressure(struct ms5611Config *cfg);
uint32_t readTemperature(struct ms5611Config *cfg);
uint32_t readPressure(struct ms5611Config *cfg);
void calculatePressureAndTemperature(struct ms5611Config *cfg, uint32_t D1, uint32_t D2, float *TEMP, float *PRES);
// Загрузка параметров из базы.
void settingsLoad(struct ms5611Config *cfg, params_c_ms5611_t *params);


/**
 * @brief c_ms5611_run Функция рабочего потока модуля
 * @param module Указатель на структуру модуля
 */
void c_ms5611_run (module_c_ms5611_t *module)
{
    // Структура хранит параметры модуля
    struct ms5611Config cfg_ms5611, *pcfg_ms5611;

    memset(&cfg_ms5611, 0, sizeof(struct ms5611Config));

    cfg_ms5611.OSR             = MS5611_OVERSAMPLING;
    cfg_ms5611.i2c_address     = MS5611_I2C_ADDR;
    cfg_ms5611.I2CBusName      = "/dev/i2c-1";
    cfg_ms5611.baro_initialized = false;
    cfg_ms5611.convTime        = MS5611_CONV_TIME_OSR_4096;
    cfg_ms5611.alfaFilt        = 0.0f;
    cfg_ms5611.initPress0      = false;
    cfg_ms5611.startAltitude   = 0.0f;

    uint32_t D2; // RawTemperature
    uint32_t D1; // RawPressure
    float Pressure, oldPressure;
    float Temperature;

    int countCalib = 0;

    while(1) {
        get_input_data(&module->module_info);

        if(!cfg_ms5611.baro_initialized)
        {
            settingsLoad(&cfg_ms5611, &module->params_c_ms5611);

            // Открываем сессию с i2c шиной
            if(cfg_ms5611.i2c_session<1)
            {
                // Устанавливаем соединение с сервисом
                if(!cfg_ms5611.i2c_service.connected)
                {
                    connect_i2c_service(&cfg_ms5611.i2c_service);
                    continue;
                }

                cfg_ms5611.i2c_session = open_i2c(&cfg_ms5611.i2c_service, cfg_ms5611.I2CBusName);
                continue;
            }

            if(!init_ms5611(&cfg_ms5611))
            {
                printf("Error init MS5611 on bus %s\n", cfg_ms5611.I2CBusName);
                rt_task_sleep(rt_timer_ns2ticks(200000000));
                continue;
            }

            rt_task_sleep(rt_timer_ns2ticks(700000000));
            cfg_ms5611.baro_initialized=true;
        }

        /* Start pressure conversion */
        refreshPressure(&cfg_ms5611);
        /* Read the pressure conversion */
        D1 = readPressure(&cfg_ms5611);

        /* Start temperature conversion */
        refreshTemperature(&cfg_ms5611);
        /* Read the temperature conversion */
        D2 = readTemperature(&cfg_ms5611);

        calculatePressureAndTemperature(&cfg_ms5611, D1, D2, &Temperature, &Pressure);

        if(cfg_ms5611.alfaFilt > 0.1f)
        {
            Pressure = oldPressure = Pressure * (1 - cfg_ms5611.alfaFilt) + oldPressure * cfg_ms5611.alfaFilt;
        }

        //printf("Temperature=%f | Pressure=%f\n", Temperature, Pressure);

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

        if(cfg_ms5611.calcPress0)
        {
            if(!cfg_ms5611.initPress0 & countCalib++ >= 100)
            {
                cfg_ms5611.initPress0 = true;
                cfg_ms5611.startAltitude = BaroTemp->Altitude;
            }

            BaroTemp->Altitude -= cfg_ms5611.startAltitude;
        }

        checkin_BaroTemp(module, &BaroTemp);
    }
}



/**
 * @brief init_ms5611 Инициализация датчика давления ms5611
 * Считывает калибровочные коэффициенты из датчика
 * @return true в случае отсутствия ошибок
 */
bool init_ms5611(struct ms5611Config *cfg)
{
	/* Reset MS5611 */
    int res = write_cmd_i2c(&cfg->i2c_service, cfg->i2c_session, cfg->i2c_address, MS5611_RESET);
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
        res = read_i2c(&cfg->i2c_service, cfg->i2c_session, cfg->i2c_address, MS5611_CALIB_ADDR + i*2, 2, &Data, &ret_len);
        if(res<0 | ret_len != 2) {
			print_i2c_error(res);
			return false;
		}

        cfg->C[i] = (Data[0] << 8) | Data[1];
        printf("C[%i]=%i\n",i,cfg->C[i]);
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
void refreshTemperature(struct ms5611Config *cfg) {

    int res = write_cmd_i2c(&cfg->i2c_service, cfg->i2c_session, cfg->i2c_address, MS5611_TEMP_ADDR + cfg->OSR);
	if(res<0)
	{
		printf("Error refreshTemperature MS5611:\t");
		print_i2c_error(res);
        //return false;
	}
    // Waiting for pressure data ready
    rt_task_sleep(rt_timer_ns2ticks(cfg->convTime));
}

/** Read temperature value
 */
uint32_t readTemperature(struct ms5611Config *cfg) {

    char *Data;
    int ret_len;

    int res = read_i2c(&cfg->i2c_service, cfg->i2c_session, cfg->i2c_address, MS5611_ADC_READ, 3, &Data, &ret_len);
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
void refreshPressure(struct ms5611Config *cfg) {

    int res = write_cmd_i2c(&cfg->i2c_service, cfg->i2c_session, cfg->i2c_address, MS5611_PRES_ADDR + cfg->OSR);
	if(res<0)
	{
		printf("Error refreshPressure MS5611:\t");
		print_i2c_error(res);
        //return false;
    }
    // Waiting for temperature data ready
    rt_task_sleep(rt_timer_ns2ticks(cfg->convTime));
}

/** Read pressure value
 */
uint32_t readPressure(struct ms5611Config *cfg) {

    char *Data;
    int ret_len;

    int res = read_i2c(&cfg->i2c_service, cfg->i2c_session, cfg->i2c_address, MS5611_ADC_READ, 3, &Data, &ret_len);
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
void calculatePressureAndTemperature(struct ms5611Config *cfg, uint32_t D1, uint32_t D2, float *TEMP, float *PRES) {
	
    uint16_t *C =cfg->C;

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

/**
 * @brief Читает данные из базы MongoDB
 */
void settingsLoad(struct ms5611Config *cfg, params_c_ms5611_t *params)
{
    cfg->I2CBusName    = params->I2C_Device;
    cfg->i2c_address   = params->I2C_Address;

    switch (params->OverSampling)
    {
        case 256:
            cfg->OSR        = MS5611_OSR_256;
            cfg->convTime   = MS5611_CONV_TIME_OSR_256;
            break;

        case 512:
            cfg->OSR        = MS5611_OSR_512;
            cfg->convTime   = MS5611_CONV_TIME_OSR_512;
            break;

        case 1024:
            cfg->OSR        = MS5611_OSR_1024;
            cfg->convTime   = MS5611_CONV_TIME_OSR_1024;
            break;

        case 2048:
            cfg->OSR        = MS5611_OSR_2048;
            cfg->convTime   = MS5611_CONV_TIME_OSR_2048;
            break;

        case 4096:
            cfg->OSR        = MS5611_OSR_4096;
            cfg->convTime   = MS5611_CONV_TIME_OSR_4096;
            break;
    }

    cfg->alfaFilt = params->alfaFiltered;
    cfg->calcPress0 = params->calcPress0;
}
