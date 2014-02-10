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

#pragma once

#include "module/CModule"
#include "system/CBus"

class CMs5611 : public CModule
{
public:
	enum oversampling {
	    MS5611_OSR_256  = 0,
	    MS5611_OSR_512  = 2,
	    MS5611_OSR_1024 = 4,
	    MS5611_OSR_2048 = 6,
	    MS5611_OSR_4096 = 8,
	};

public:
	CMs5611();
	~CMs5611();

	virtual bool init(const mongo::BSONObj& initObject);
	bool start();

private:
	CBus m_bus;
	uint16_t m_C[6];
	uint8_t m_oversampling;

	bool initMs5611();
	bool setOneReg(uint8_t reg);
	bool setReg(uint8_t reg, uint8_t data);
	bool getReg(uint8_t reg, uint8_t* data, size_t size = 1);
	void moduleTask();
};
