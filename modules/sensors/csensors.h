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

class CSensors : public CModule
{
public:
	CSensors();
	~CSensors();

	bool init();
	bool start();

private:
	float m_gyroScale[3];
	float m_gyroBias[3];
	float m_accelScale[3];
	float m_accelBias[3];

	void moduleTask();
//	void recievedMpuObject(CUAVObject* object);
	void calcSensor(float* sensorValues, const float* sensorScale, const float* sensorBias, int numVals) const;

protected:

// notify
//	virtual void recievedData(CUAVObject* data);
};
