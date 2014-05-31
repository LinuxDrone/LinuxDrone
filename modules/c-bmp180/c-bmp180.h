#pragma once



enum oversampling {
    BMP180_OSS_0 = 0,
    BMP180_OSS_1 = 1,
    BMP180_OSS_2 = 2,
    BMP180_OSS_3 = 3
};

/* BMP180 Addresses */
#define BMP180_I2C_ADDR     0x77
#define BMP180_CALIB_ADDR   0xAA
#define BMP180_CALIB_LEN    22
#define BMP180_CTRL_ADDR    0xF4
#define BMP180_PRES_ADDR    0x34
#define BMP180_TEMP_ADDR    0x2E
#define BMP180_ADC_MSB      0xF6
#define BMP180_P0           101325

#define BMP180_OVERSAMPLING BMP180_OSS_3
