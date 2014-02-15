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

#include "chmc5883.h"
#include "system/CSystemBusPool"
#include "system/CSystemBus"
#include "system/CSystem"
#include "system/Logger"

#include <native/timer.h>

#include "my_memory"

extern "C" {
CModule* moduleCreator()
{
	return new CHmc5883();
}

const char* moduleName() {
	return "Hmc5883";
}
}


/* HMC5883 Addresses */
#define HMC5883_I2C_ADDR           0x1E
#define HMC5883_I2C_READ_ADDR      0x3D
#define HMC5883_I2C_WRITE_ADDR     0x3C
#define HMC5883_CONFIG_REG_A       0x00
#define HMC5883_CONFIG_REG_B       0x01
#define HMC5883_MODE_REG           0x02
#define HMC5883_DATAOUT_XMSB_REG   0x03
#define HMC5883_DATAOUT_XLSB_REG   0x04
#define HMC5883_DATAOUT_ZMSB_REG   0x05
#define HMC5883_DATAOUT_ZLSB_REG   0x06
#define HMC5883_DATAOUT_YMSB_REG   0x07
#define HMC5883_DATAOUT_YLSB_REG   0x08
#define HMC5883_DATAOUT_STATUS_REG 0x09
#define HMC5883_DATAOUT_IDA_REG    0x0A
#define HMC5883_DATAOUT_IDB_REG    0x0B
#define HMC5883_DATAOUT_IDC_REG    0x0C

/* Output Data Rate */
#define HMC5883_ODR_0_75           0x00
#define HMC5883_ODR_1_5            0x04
#define HMC5883_ODR_3              0x08
#define HMC5883_ODR_7_5            0x0C
#define HMC5883_ODR_15             0x10
#define HMC5883_ODR_30             0x14
#define HMC5883_ODR_75             0x18

/* Measure configuration */
#define HMC5883_MEASCONF_NORMAL    0x00
#define HMC5883_MEASCONF_BIAS_POS  0x01
#define HMC5883_MEASCONF_BIAS_NEG  0x02

/* Gain settings */
#define HMC5883_GAIN_0_88          0x00
#define HMC5883_GAIN_1_3           0x20
#define HMC5883_GAIN_1_9           0x40
#define HMC5883_GAIN_2_5           0x60
#define HMC5883_GAIN_4_0           0x80
#define HMC5883_GAIN_4_7           0xA0
#define HMC5883_GAIN_5_6           0xC0
#define HMC5883_GAIN_8_1           0xE0

/* Modes */
#define HMC5883_MODE_CONTINUOUS    0x00
#define HMC5883_MODE_SINGLE        0x01
#define HMC5883_MODE_IDLE          0x02
#define HMC5883_MODE_SLEEP         0x03

/* Sensitivity Conversion Values */
#define HMC5883_Sensitivity_0_88Ga 1370 // LSB/Ga
#define HMC5883_Sensitivity_1_3Ga  1090    // LSB/Ga
#define HMC5883_Sensitivity_1_9Ga  820     // LSB/Ga
#define HMC5883_Sensitivity_2_5Ga  660     // LSB/Ga
#define HMC5883_Sensitivity_4_0Ga  440     // LSB/Ga
#define HMC5883_Sensitivity_4_7Ga  390     // LSB/Ga
#define HMC5883_Sensitivity_5_6Ga  330     // LSB/Ga
#define HMC5883_Sensitivity_8_1Ga  230     // LSB/Ga  --> NOT RECOMMENDED



CHmc5883::CHmc5883() :
	CModule(1024)
{
}

CHmc5883::~CHmc5883()
{
}

bool CHmc5883::init(const mongo::BSONObj& initObject)
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
	m_bus = CBus(busType, busName, HMC5883_I2C_ADDR);
	if (!m_bus.isOpened()) {
		return false;
	}
	return initHmc5883();
}

bool CHmc5883::start()
{
	CModule::start(this, &CHmc5883::moduleTask);
	return true;
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================


static uint8_t CTRLB = 0x00;

bool CHmc5883::initHmc5883()
{
	CMutexSection locker(m_bus.mutex());


	uint8_t ID[4]={0};
	readId(ID[0]);
	printf("HMC ID: %s",ID);

	uint8_t CTRLA = 0x00;
	uint8_t MODE  = 0x00;

	CTRLB  = 0;

	CTRLA |= (uint8_t)(HMC5883_ODR_75 | HMC5883_MEASCONF_NORMAL);
	CTRLB |= (uint8_t)(HMC5883_GAIN_1_9);
	MODE  |= (uint8_t)(HMC5883_MODE_CONTINUOUS);

	// CTRL_REGA
	while (!setReg(HMC5883_CONFIG_REG_A,CTRLA)) {
		;
	}

	// CTRL_REGB
	while (!setReg(HMC5883_CONFIG_REG_B,CTRLB)) {
		;
	}

	// Mode register
	while (!setReg(HMC5883_MODE_REG,MODE)) {
		;
	}
	Logger() << "HMC5883 init done";
	return true;
}

bool CHmc5883::setOneReg(uint8_t reg)
{
	uint8_t in[] = {reg};
	int size = m_bus.write(in, sizeof in);
	if (size != sizeof(in)) {
		return false;
	}
	return true;
}

bool CHmc5883::setReg(uint8_t reg, uint8_t data)
{
	uint8_t in[] = {reg, data};
	int size = m_bus.write(in, sizeof in);
	if (size != sizeof(in)) {
		return false;
	}
	return true;
}

bool CHmc5883::getReg(uint8_t reg, uint8_t* data, size_t size /*= 1*/)
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

int32_t CHmc5883::ReadMag(int16_t out[3])
{
    uint8_t buffer[6];
    int32_t temp;
    int32_t sensitivity;

    if (getReg(HMC5883_DATAOUT_XMSB_REG, buffer, 6) != true) {
        return -1;
    }

    switch (CTRLB & 0xE0) {
    case 0x00:
        sensitivity = HMC5883_Sensitivity_0_88Ga;
        break;
    case 0x20:
        sensitivity = HMC5883_Sensitivity_1_3Ga;
        break;
    case 0x40:
        sensitivity = HMC5883_Sensitivity_1_9Ga;
        break;
    case 0x60:
        sensitivity = HMC5883_Sensitivity_2_5Ga;
        break;
    case 0x80:
        sensitivity = HMC5883_Sensitivity_4_0Ga;
        break;
    case 0xA0:
        sensitivity = HMC5883_Sensitivity_4_7Ga;
        break;
    case 0xC0:
        sensitivity = HMC5883_Sensitivity_5_6Ga;
        break;
    case 0xE0:
        sensitivity = HMC5883_Sensitivity_8_1Ga;
        break;
    default:
    	break;
    }

    for (int i = 0; i < 3; i++) {
        temp   = ((int16_t)((uint16_t)buffer[2 * i] << 8)
                  + buffer[2 * i + 1]) * 1000 / sensitivity;
        out[i] = temp;
    }
    // Data reads out as X,Z,Y
    temp   = out[2];
    out[2] = out[1];
    out[1] = temp;

    // This should not be necessary but for some reason it is coming out of continuous conversion mode
	while (!setReg(HMC5883_MODE_REG,HMC5883_MODE_CONTINUOUS)) {
		;
	}

    return 0;
}

bool CHmc5883::readId(uint8_t& hmcId)
{
	if (!getReg(HMC5883_DATAOUT_IDA_REG, &hmcId,3)) {
		return false;
	}
	return true;
}

void CHmc5883::moduleTask()
{
	RTIME time = rt_timer_read();

	int16_t out[3];
	if(ReadMag(out) == -1)
	{
		return;
	}

    mongo::BSONObjBuilder builder;
    builder.append("name", "Hmc5883");

    builder.append("magX", out[0]);
    builder.append("magZ", out[1]);
    builder.append("magY", out[2]);

    mongo::BSONObj obj = builder.obj();
    addData(obj);

    RTIME diff = time - rt_timer_read();
    SRTIME el = rt_timer_ticks2ns(diff);
    uint64_t elapsed = abs(el) / 1000;
    //Logger() << elapsed;
    //printf("x:%5d,y:%5d,z:%5d\n",out[0],out[2],out[1]);
    CSystem::sleep(100);
}
