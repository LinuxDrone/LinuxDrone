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

class CMpu6050 : public CModule
{
public:
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

public:
	CMpu6050();
	~CMpu6050();

	virtual bool init(const mongo::BSONObj& initObject);
	bool start();

private:
	CBus m_bus;

	bool initMpu();
	void configureRanges();
	bool setReg(uint8_t reg, uint8_t data);
	bool getReg(uint8_t reg, uint8_t* data, size_t size = 1);
	bool readId(uint8_t& mpuId);
	void moduleTask();
};
