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

class CHmc5883 : public CModule
{
public:
	CHmc5883();
	~CHmc5883();

	virtual bool init(const mongo::BSONObj& initObject);
	bool start();

private:
	CBus m_bus;

	bool initHmc5883();
	bool readId(uint8_t& hmcId);
	int32_t ReadMag(int16_t out[3]);
	bool setOneReg(uint8_t reg);
	bool setReg(uint8_t reg, uint8_t data);
	bool getReg(uint8_t reg, uint8_t* data, size_t size = 1);
	void moduleTask();
};
