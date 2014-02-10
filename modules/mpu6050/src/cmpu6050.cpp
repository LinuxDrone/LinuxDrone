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

#include "cmpu6050.h"
#include "system/CSystemBusPool"
#include "system/CSystemBus"
#include "system/CSystem"
#include "system/Logger"

#include <native/timer.h>

#include "my_memory"

/* Power management and clock selection */
#define MPU6000_PWRMGMT_IMU_RST          0X80
#define MPU6000_PWRMGMT_INTERN_CLK       0X00
#define MPU6000_PWRMGMT_PLL_X_CLK        0X01
#define MPU6000_PWRMGMT_PLL_Y_CLK        0X02
#define MPU6000_PWRMGMT_PLL_Z_CLK        0X03
#define MPU6000_PWRMGMT_STOP_CLK         0X07

/* User control functionality */
#define MPU6000_USERCTL_FIFO_EN          0X40
#define MPU6000_USERCTL_I2C_MST_EN       0x20
#define MPU6000_USERCTL_DIS_I2C          0X10
#define MPU6000_USERCTL_FIFO_RST         0X04
#define MPU6000_USERCTL_SIG_COND         0X02
#define MPU6000_USERCTL_GYRO_RST         0X01

/* Interrupt Configuration */

#define MPU6000_I2C_BYPASS_EN            0x02
#define MPU6000_INT_ACTL                 0x80
#define MPU6000_INT_OPEN                 0x40
#define MPU6000_INT_LATCH_EN             0x20
#define MPU6000_INT_CLR_ANYRD            0x10
#define MPU6000_INTEN_OVERFLOW           0x10
#define MPU6000_INTEN_DATA_RDY           0x01

#define MPU6000_ACCEL_X_OUT_MSB          0x3B
#define MPU6000_ACCEL_X_OUT_LSB          0x3C
#define MPU6000_ACCEL_Y_OUT_MSB          0x3D
#define MPU6000_ACCEL_Y_OUT_LSB          0x3E
#define MPU6000_ACCEL_Z_OUT_MSB          0x3F
#define MPU6000_ACCEL_Z_OUT_LSB          0x40
#define MPU6000_TEMP_OUT_MSB             0x41
#define MPU6000_TEMP_OUT_LSB             0x42
#define MPU6000_GYRO_X_OUT_MSB           0x43
#define MPU6000_GYRO_X_OUT_LSB           0x44
#define MPU6000_GYRO_Y_OUT_MSB           0x45
#define MPU6000_GYRO_Y_OUT_LSB           0x46
#define MPU6000_GYRO_Z_OUT_MSB           0x47
#define MPU6000_GYRO_Z_OUT_LSB           0x48

CMpu6050::CMpu6050() :
	CModule("Mpu60x0", 1024)
{
}

CMpu6050::~CMpu6050()
{
}

bool CMpu6050::init(const mongo::BSONObj& initObject)
{
	if (!CModule::init(initObject)) {
		return false;
	}
	CString busName = "/dev/i2c-1";
	CSystemBus::BusType busType = CSystemBus::BusType_I2C;
	if (initObject.hasElement("params")) {
		mongo::BSONElement elemParam = initObject["params"];
		mongo::BSONObj objParam = elemParam.Obj();
		if (objParam.hasElement("bus_name")) {
			busName = objParam["bus_name"].String().c_str();
		}
		if (objParam.hasElement("bus_type")) {
			busType = (CSystemBus::BusType)objParam["bus_type"].Number();
		}
	}
	m_bus = CBus(busType, busName, 0x68);
	if (!m_bus.isOpened()) {
		return false;
	}
	return initMpu();
}

bool CMpu6050::start()
{
	CModule::start(this, &CMpu6050::moduleTask);
	return true;
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

bool CMpu6050::initMpu()
{
	CMutexSection locker(m_bus.mutex());

	uint8_t mpuId = 0;
	if (!readId(mpuId)) {
		return false;
	}
	if (mpuId != 0x68) {
		return false;
	}
    // Reset chip
    while (!setReg(MpuRegs_PwrMgmt, MPU6000_PWRMGMT_IMU_RST)) {
        ;
    }
    CSystem::sleep(50);
    // Reset chip and fifo
	while (!setReg(MpuRegs_UserCtrl,
				   MPU6000_USERCTL_GYRO_RST |
				   MPU6000_USERCTL_SIG_COND |
				   MPU6000_USERCTL_FIFO_RST)) {
		;
	}
    // Wait for reset to finish
	uint8_t data = 0;
	bool ret = false;
	while ((ret = getReg(MpuRegs_UserCtrl, &data))) {
		if (!(data & (MPU6000_USERCTL_GYRO_RST | MPU6000_USERCTL_SIG_COND | MPU6000_USERCTL_FIFO_RST))) {
			break;
		}
	}
	if (ret == false) {
		return false;
	}
	CSystem::sleep(10);
    // Power management configuration
    while (!setReg(MpuRegs_PwrMgmt, MPU6000_PWRMGMT_PLL_X_CLK)) {
        ;
    }

    configureRanges();
    // Interrupt configuration
    while (!setReg(MpuRegs_UserCtrl, 0)) {
        ;
    }

    // When I2C_BYPASS_EN is equal to 1 and I2C_MST_EN (Register 106 bit[5]) is equal to 0, the host
    // application processor will be able to directly access the auxiliary I2C bus of the MPU-60X0
    while (!setReg(MpuRegs_IntCfg, MPU6000_I2C_BYPASS_EN)) {
        ;
    }
	return true;
}

void CMpu6050::configureRanges()
{
//	MPU6000_SCALE_2000_DEG, MPU6000_ACCEL_8G, MPU6000_LOWPASS_256_HZ
    if (setReg(MpuRegs_DlpfCfg, MpuFilter_Lowpass256Hz) != true) {
        return;
    }

    // Sample rate divider, chosen upon digital filtering settings
    while (!setReg(MpuRegs_SmplRtDiv, 11)) {
    	;
    }

    // Gyro range
    while (!setReg(MpuRegs_GyroCfg, MpuRange_Scale2000Deg)) {
        ;
    }

    // Set the accel range
    while (!setReg(MpuRegs_AccellCfg, MpuAccelrange_Accel8G)) {
        ;
    }
}

bool CMpu6050::setReg(uint8_t reg, uint8_t data)
{
	uint8_t in[] = {reg, data};
	int size = m_bus.write(in, sizeof in);
	if (size != sizeof(in)) {
		return false;
	}
	return true;
}

bool CMpu6050::getReg(uint8_t reg, uint8_t* data, size_t size /*= 1*/)
{
	int len = m_bus.write(&reg, 1);
	if (len != 1) {
		return false;
	}
	len = m_bus.read(data, size);
	if (int (size) != len) {
		return false;
	}
	return true;
}

bool CMpu6050::readId(uint8_t& mpuId)
{
	if (!getReg(MpuRegs_WhoAmI, &mpuId)) {
		return false;
	}
	return true;
}

void CMpu6050::moduleTask()
{
	RTIME time = rt_timer_read();

	static const size_t size = MPU6000_GYRO_Z_OUT_LSB - MPU6000_ACCEL_X_OUT_MSB + 1;
	uint8_t data[size];
	{
		CMutexSection locker(m_bus.mutex());
		bool ret = getReg(MPU6000_ACCEL_X_OUT_MSB, data, size);
		if (!ret) {
			return;
		}
	}
	// unpack received data
	int16_t gyros[3];
	int16_t accels[3];
	int16_t temp;

    gyros[0] = data[MPU6000_GYRO_X_OUT_MSB - MPU6000_ACCEL_X_OUT_MSB] << 8 | data[MPU6000_GYRO_X_OUT_LSB - MPU6000_ACCEL_X_OUT_MSB];
    gyros[1] = data[MPU6000_GYRO_Y_OUT_MSB - MPU6000_ACCEL_X_OUT_MSB] << 8 | data[MPU6000_GYRO_Y_OUT_LSB - MPU6000_ACCEL_X_OUT_MSB];
    gyros[2] = data[MPU6000_GYRO_Z_OUT_MSB - MPU6000_ACCEL_X_OUT_MSB] << 8 | data[MPU6000_GYRO_Z_OUT_LSB - MPU6000_ACCEL_X_OUT_MSB];

    accels[0] = data[MPU6000_ACCEL_X_OUT_MSB - MPU6000_ACCEL_X_OUT_MSB] << 8 | data[MPU6000_ACCEL_X_OUT_LSB - MPU6000_ACCEL_X_OUT_MSB];
    accels[1] = data[MPU6000_ACCEL_Y_OUT_MSB - MPU6000_ACCEL_X_OUT_MSB] << 8 | data[MPU6000_ACCEL_Y_OUT_LSB - MPU6000_ACCEL_X_OUT_MSB];
    accels[2] = data[MPU6000_ACCEL_Z_OUT_MSB - MPU6000_ACCEL_X_OUT_MSB] << 8 | data[MPU6000_ACCEL_Z_OUT_LSB - MPU6000_ACCEL_X_OUT_MSB];

    temp = data[MPU6000_TEMP_OUT_MSB - MPU6000_ACCEL_X_OUT_MSB] << 8 | data[MPU6000_TEMP_OUT_LSB - MPU6000_ACCEL_X_OUT_MSB];

    mongo::BSONObjBuilder builder;
    builder.append("name", "Mpu6050");

	const char* names[] = {"x", "y", "z"};

	mongo::BSONObjBuilder gyroBuilder;
	mongo::BSONObjBuilder accelBuilder;
    for (int i = 0;i<3;i++) {
    	gyroBuilder.append(names[i], float (gyros[i]) * (1.0f / 16.4f));
    	accelBuilder.append(names[i], float (accels[i]) * (9.81f / 4096.0f));
    }
    builder.append("gyro", gyroBuilder.obj());
    builder.append("accel", accelBuilder.obj());
    builder.append("temperature", 35.0f + ((float)temp + 512.0f) / 340.0f);		// it`s magic! :)

    mongo::BSONObj obj = builder.obj();
    addData(obj);

    RTIME diff = time - rt_timer_read();
    SRTIME el = rt_timer_ticks2ns(diff);
    uint64_t elapsed = abs(el) / 1000;
    Logger() << elapsed;
    CSystem::sleep(100);
}
