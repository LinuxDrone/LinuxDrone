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

#include "csensors.h"

CSensors::CSensors() :
	CModule("Sensors", 20, 1024)
{
	for (int i = 0;i<3;i++) {
		m_gyroScale[i] = 1.0f;
		m_gyroBias[i] = 0.0f;
		m_accelScale[i] = 1.0f;
		m_accelBias[i] = 0.0f;
	}
}

CSensors::~CSensors()
{
}

bool CSensors::init()
{
	return true;
}

bool CSensors::start()
{
	CModule::start(this, &CSensors::moduleTask);
	return true;
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

void CSensors::moduleTask()
{
}

//void CSensors::recievedMpuObject(CUAVObject* object)
//{
//	float gyros[3];
//	float accels[3];
//	float temp;
//
//	bool existGyro = false;
//	bool existAccels = false;
//
//	CUAVObjectField* fieldTemp = object->fieldByName("temperature");
//	if (!fieldTemp) {
//		return;
//	}
//	temp = float (fieldTemp->value(0).toDouble());
//
//	CUAVObjectField* fieldGyro = object->fieldByName("gyros");
//	if (fieldGyro && fieldGyro->numElements() >= 3) {
//		for (size_t i = 0;i<3;i++) {
//			gyros[i] = float (fieldGyro->value(i).toDouble());
//		}
//		calcSensor(gyros, m_gyroScale, m_gyroBias, 3);
//		existGyro = true;
//	}
//	CUAVObjectField* fieldAccels = object->fieldByName("accels");
//	if (fieldAccels && fieldAccels->numElements() >= 3) {
//		for (size_t i = 0;i<3;i++) {
//			accels[i] = float (fieldAccels->value(i).toDouble());
//		}
//		calcSensor(accels, m_accelScale, m_accelBias, 3);
//		existAccels = true;
//	}
//
//	// make CUAVObject
//	CUAVObject* out = CUAVObject::createObject("Sensors");
//	if (existGyro) {
//	}
//	if (existAccels) {
//	}
//
//	CModule::addData(out);
//	SAFE_RELEASE(out);
//}

void CSensors::calcSensor(float* sensorValues, const float* sensorScale, const float* sensorBias, int numVals) const
{
	if (!sensorValues || !sensorScale || sensorBias) {
		return;
	}
	// TODO: it`s multiply matrix4 and vector4f!
	for (int i = 0;i<numVals;i++) {
		sensorValues[i] = sensorValues[i] * sensorScale[i] - sensorBias[i];
	}
	// TODO: rotate sensors, if needed
}

//===================================================================
//  p r o t e c t e d   f u n c t i o n s
//===================================================================

//-------------------------------------------------------------------
//  n o t i f y
//-------------------------------------------------------------------

//void CSensors::recievedData(CUAVObject* data)
//{
//	if (!data) {
//		return;
//	}
//	if (data->name() == CString("Mpu6050")) {
//		recievedMpuObject(data);
//	}
//}
