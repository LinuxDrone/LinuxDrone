
#include "cbmp180.h"
#include "system/CSystemBusPool"
#include "system/CSystemBus"
#include "system/CSystem"
#include "system/Logger"

#include <native/timer.h>

#include "my_memory"

/* BMP085 Addresses */
#define BMP180_I2C_ADDR     0x77
#define BMP180_CALIB_ADDR   0xAA
#define BMP180_CALIB_LEN    22
#define BMP180_CTRL_ADDR    0xF4
#define BMP180_PRES_ADDR    0x34
#define BMP180_TEMP_ADDR    0x2E
#define BMP180_ADC_MSB      0xF6
#define BMP180_P0           101325
#define BMP180_OVERSAMPLING BMP180_OSS_3

CBmp180::CBmp180() :
	CModule("Bmp180", 1024)
{
}

CBmp180::~CBmp180()
{
}

bool CBmp180::init(const mongo::BSONObj& initObject)
{
	if (!CModule::init(initObject)) {
		return false;
	}
	CString busName = "/dev/i2c-2";
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
	m_bus = CBus(busType, busName, BMP180_I2C_ADDR);
	if (!m_bus.isOpened()) {
		return false;
	}
	initBmp180();
	return true;
}

bool CBmp180::start()
{
	CModule::start(this, &CBmp180::moduleTask);
	return true;
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

bool CBmp180::initBmp180()
{
	CMutexSection locker(m_bus.mutex());

    CSystem::sleep(20);

    m_oversampling = (BMP180_OVERSAMPLING << 6);
    uint8_t Data[BMP180_CALIB_LEN];

    /* Read all 22 bytes of calibration data in one transfer, this is a very optimized way of doing things */
    while (!getReg(BMP180_CALIB_ADDR, Data, BMP180_CALIB_LEN)){};

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

	return true;
}

bool CBmp180::setOneReg(uint8_t reg)
{
	uint8_t in[] = {reg};
	int size = m_bus.write(in, sizeof in);
	if (size != sizeof(in)) {
		return false;
	}
	return true;
}

bool CBmp180::setReg(uint8_t reg, uint8_t data)
{
	uint8_t in[] = {reg, data};
	int size = m_bus.write(in, sizeof in);
	if (size != sizeof(in)) {
		return false;
	}
	return true;
}

bool CBmp180::getReg(uint8_t reg, uint8_t* data, size_t size /*= 1*/)
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

void CBmp180::moduleTask()
{
    uint8_t Data[3];

	RTIME time = rt_timer_read();

	/* Straight from the datasheet */
	int32_t X1, X2, X3, B3, B5, B6, P;
	uint32_t B4, B7;

	uint16_t RawTemperature;
	uint32_t RawPressure;
	uint32_t Pressure;
	uint16_t Temperature;

	/* Start temperature conversion */
	while (!setReg(BMP180_CTRL_ADDR, BMP180_TEMP_ADDR)){};
    CSystem::sleep(5);

    /* Read the temperature conversion */
    while (!getReg(BMP180_ADC_MSB, Data, 2)){};

    RawTemperature = ((Data[0] << 8) | Data[1]);

    X1 = (RawTemperature - CalibData.AC6) * CalibData.AC5 >> 15;
    X2 = ((int32_t)CalibData.MC << 11) / (X1 + CalibData.MD);
    B5 = X1 + X2;
    Temperature = (B5 + 8) >> 4;


	/* Start pressure conversion */
	while (!setReg(BMP180_CTRL_ADDR, (BMP180_PRES_ADDR + m_oversampling))){};
    CSystem::sleep(26);

    /* Read the temperature conversion */
    while(!getReg(BMP180_ADC_MSB, Data, 3)){};

    RawPressure = ((Data[0] << 16) | (Data[1] << 8) | Data[2]) >> (8 - BMP180_OVERSAMPLING);

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


    mongo::BSONObjBuilder builder;
    builder.append("name", "Bmp180");

    builder.append("temperature", Temperature/10.0f);
    builder.append("pressure", Pressure/1000.0f);

    mongo::BSONObj obj = builder.obj();
    addData(obj);

    RTIME diff = time - rt_timer_read();
    SRTIME el = rt_timer_ticks2ns(diff);
    uint64_t elapsed = abs(el) / 1000;
    Logger() << elapsed;

    float PressureRs = (Pressure * 760.0f) / 101325.0f;
    float Altitude = 44330.0f * (1 - pow((Pressure/101325.0f), 0.190295f));

    printf("Temp=%f, Pres=%f, PressureRs=%f, Altitude=%f\n", Temperature/10.0,Pressure/1000.0, PressureRs, Altitude);
    CSystem::sleep(100);
}
