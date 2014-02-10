
#pragma once

#include "module/CModule"
#include "system/CBus"

class CBmp180 : public CModule
{
public:
	enum oversampling {
	    BMP180_OSS_0 = 0,
	    BMP180_OSS_1 = 1,
	    BMP180_OSS_2 = 2,
	    BMP180_OSS_3 = 3,
	};

public:
	CBmp180();
	~CBmp180();

	virtual bool init(const mongo::BSONObj& initObject);
	bool start();

private:
	/* Local Types */
	typedef struct {
	    int16_t  AC1;
	    int16_t  AC2;
	    int16_t  AC3;
	    uint16_t AC4;
	    uint16_t AC5;
	    uint16_t AC6;
	    int16_t  B1;
	    int16_t  B2;
	    int16_t  MB;
	    int16_t  MC;
	    int16_t  MD;
	} BMP180CalibDataTypeDef;

	BMP180CalibDataTypeDef CalibData;

	uint8_t  m_oversampling;

	CBus m_bus;

	bool initBmp180();
	bool setOneReg(uint8_t reg);
	bool setReg(uint8_t reg, uint8_t data);
	bool getReg(uint8_t reg, uint8_t* data, size_t size = 1);
	void moduleTask();
};
