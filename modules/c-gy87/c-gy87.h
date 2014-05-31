#pragma once

// адрес mpu на шине i2c
#define MPU6050_I2C_ADDR 0x68


enum MpuRegs {
    MpuRegs_SmplRtDiv = 0X19,
    MpuRegs_DlpfCfg   = 0X1A,
    MpuRegs_GyroCfg   = 0X1B,
    MpuRegs_AccellCfg = 0X1C,
    MpuRegs_IntCfg    = 0x37,
    MpuRegs_UserCtrl  = 0x6a,
    MpuRegs_PwrMgmt   = 0x6b,
    MpuRegs_WhoAmI    = 0x75
};


enum MpuRange {
    MpuRange_Scale250Deg = 0x00,
    MpuRange_Scale500Deg = 0x08,
    MpuRange_Scale1000Deg = 0x10,
    MpuRange_Scale2000Deg = 0x18,
};


enum MpuFilter {
    MpuFilter_Lowpass256Hz,
    MpuFilter_Lowpass188Hz,
    MpuFilter_Lowpass98Hz,
    MpuFilter_Lowpass42Hz,
    MpuFilter_Lowpass20Hz,
    MpuFilter_Lowpass10Hz,
    MpuFilter_Lowpass5Hz
};


enum MpuAccelRange {
    MpuAccelrange_Accel2G  = 0x00,
    MpuAccelrange_Accel4G  = 0x08,
    MpuAccelrange_Accel8G  = 0x10,
    MpuAccelrange_Accel16G = 0x18
};


enum MpuOrientation { // clockwise rotation from board forward
    MpuTop0Deg   = 0x00,
    MpuTop90Deg  = 0x01,
    MpuTop180Deg = 0x02,
    MpuTop270Deg = 0x03
};


/* Power management and clock selection */
#define CGY87_PWRMGMT_IMU_RST          0X80
#define CGY87_PWRMGMT_INTERN_CLK       0X00
#define CGY87_PWRMGMT_PLL_X_CLK        0X01
#define CGY87_PWRMGMT_PLL_Y_CLK        0X02
#define CGY87_PWRMGMT_PLL_Z_CLK        0X03
#define CGY87_PWRMGMT_STOP_CLK         0X07

/* User control functionality */
#define CGY87_USERCTL_FIFO_EN          0X40
#define CGY87_USERCTL_I2C_MST_EN       0x20
#define CGY87_USERCTL_DIS_I2C          0X10
#define CGY87_USERCTL_FIFO_RST         0X04
#define CGY87_USERCTL_SIG_COND         0X02
#define CGY87_USERCTL_GYRO_RST         0X01

/* Interrupt Configuration */

#define CGY87_I2C_BYPASS_EN            0x02
#define CGY87_INT_ACTL                 0x80
#define CGY87_INT_OPEN                 0x40
#define CGY87_INT_LATCH_EN             0x20
#define CGY87_INT_CLR_ANYRD            0x10
#define CGY87_INTEN_OVERFLOW           0x10
#define CGY87_INTEN_DATA_RDY           0x01

#define CGY87_ACCEL_X_OUT_MSB          0x3B
#define CGY87_ACCEL_X_OUT_LSB          0x3C
#define CGY87_ACCEL_Y_OUT_MSB          0x3D
#define CGY87_ACCEL_Y_OUT_LSB          0x3E
#define CGY87_ACCEL_Z_OUT_MSB          0x3F
#define CGY87_ACCEL_Z_OUT_LSB          0x40
#define CGY87_TEMP_OUT_MSB             0x41
#define CGY87_TEMP_OUT_LSB             0x42
#define CGY87_GYRO_X_OUT_MSB           0x43
#define CGY87_GYRO_X_OUT_LSB           0x44
#define CGY87_GYRO_Y_OUT_MSB           0x45
#define CGY87_GYRO_Y_OUT_LSB           0x46
#define CGY87_GYRO_Z_OUT_MSB           0x47
#define CGY87_GYRO_Z_OUT_LSB           0x48
