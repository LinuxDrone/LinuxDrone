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

#include "cms5611.h"
#include "system/CSystemBusPool"
#include "system/CSystemBus"
#include "system/CSystem"
#include "system/Logger"

#include <native/timer.h>

#include "my_memory"

extern "C" {
CModule* moduleCreator()
{
	return new CMs5611();
}

const char* moduleName() {
	return "Ms5611";
}
}



/* MS5611 Addresses */
#define MS5611_I2C_ADDR   0x77
#define MS5611_RESET      0x1E
#define MS5611_CALIB_ADDR 0xA2
#define MS5611_ADC_READ   0x00
#define MS5611_PRES_ADDR  0x40
#define MS5611_TEMP_ADDR  0x50


CMs5611::CMs5611() :
	CModule(1024)
{
}

CMs5611::~CMs5611()
{
}

bool CMs5611::init(const mongo::BSONObj& initObject)
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
	m_bus = CBus(busType, busName, MS5611_I2C_ADDR);
	if (!m_bus.isOpened()) {
		return false;
	}
	return initMs5611();
}

bool CMs5611::start()
{
	CModule::start(this, &CMs5611::moduleTask);
	return true;
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

bool CMs5611::initMs5611()
{
	CMutexSection locker(m_bus.mutex());

	while (!setOneReg(MS5611_RESET)) {
		;
	}
    CSystem::sleep(20);

    m_oversampling = MS5611_OSR_256;
    uint8_t data[2];

    /* Calibration parameters */
    for (int i = 0; i < 6; i++) {
    	while(!getReg(MS5611_CALIB_ADDR + i * 2, data, 2)){};
        m_C[i] = (data[0] << 8) | data[1];
    }
    for (int i = 0; i < 6; i++) {
		printf("C%d=%d\n",i,m_C[i]);
	}

	return true;
}

bool CMs5611::setOneReg(uint8_t reg)
{
	uint8_t in[] = {reg};
	int size = m_bus.write(in, sizeof in);
	if (size != sizeof(in)) {
		return false;
	}
	return true;
}

bool CMs5611::setReg(uint8_t reg, uint8_t data)
{
	uint8_t in[] = {reg, data};
	int size = m_bus.write(in, sizeof in);
	if (size != sizeof(in)) {
		return false;
	}
	return true;
}

bool CMs5611::getReg(uint8_t reg, uint8_t* data, size_t size /*= 1*/)
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

void CMs5611::moduleTask()
{
	RTIME time = rt_timer_read();

	uint32_t RawTemperature;
	uint32_t RawPressure;
	int64_t Pressure;
	int64_t Temperature;
    static int64_t deltaTemp;


	/* Start the conversion */
	while (!setOneReg(MS5611_TEMP_ADDR + m_oversampling)) {
		;
	}
    CSystem::sleep(5);

	uint8_t Data[3];
    /* Read the temperature conversion */
    if (getReg(MS5611_ADC_READ, Data, 3) != true) {
        return;
    }

    RawTemperature = (Data[0] << 16) | (Data[1] << 8) | Data[2];

    deltaTemp = ((int32_t)RawTemperature) - (m_C[4] << 8);
    Temperature    = 2000l + ((deltaTemp * m_C[5]) >> 23);

	/* Start the conversion */
	while (!setOneReg(MS5611_PRES_ADDR + m_oversampling)) {
		;
	}
    CSystem::sleep(26);

    int64_t Offset;
    int64_t Sens;

    /* Apply second order */
    int64_t T2=0;
    int64_t Offset2=0;
    int64_t Sens2=0;
    if(Temperature < 2000)
    {
    	T2 = (deltaTemp*deltaTemp)>>31;
    	Offset2 = 5*((int64_t)pow((Temperature - 2000l),2))>>1;
    	Sens2 = 5*((int64_t)pow((Temperature - 2000l),2))>>2;
    	if(Temperature < -1500)
    	{
    		Offset2 = Offset2 + 7*((int64_t)pow((Temperature + 1500l),2));
    		Sens2 = Sens2 + (11*((int64_t)pow((Temperature + 1500l),2))>>1);
    	}
    }
    /* Reuse Data array */
	Data[0] = 0;
	Data[1] = 0;
	Data[2] = 0;

    /* Read the pressure conversion */
    if (getReg(MS5611_ADC_READ, Data, 3) != true) {
        return;
    }
    RawPressure = ((Data[0] << 16) | (Data[1] << 8) | Data[2]);

    Offset = (((int64_t)m_C[1]) << 16) + ((((int64_t)m_C[3]) * deltaTemp) >> 7);
    Sens = ((int64_t)m_C[0]) << 15;
    Sens = Sens + ((((int64_t)m_C[2]) * deltaTemp) >> 8);

    Temperature = Temperature - T2;
    Offset = Offset - Offset2;
    Sens = Sens - Sens2;

    Pressure = (((((int64_t)RawPressure) * Sens) >> 21) - Offset) >> 15;

    mongo::BSONObjBuilder builder;
    builder.append("name", "Ms5611");

    builder.append("temperature", Temperature/100.0f);
    builder.append("pressure", Pressure/1000.0f);

    mongo::BSONObj obj = builder.obj();
    addData(obj);

    RTIME diff = time - rt_timer_read();
    SRTIME el = rt_timer_ticks2ns(diff);
    uint64_t elapsed = abs(el) / 1000;
    //Logger() << elapsed;
    printf("Temp=%f, Pres=%f\n",Temperature/100.0,Pressure/1000.0);
    CSystem::sleep(100);
}
