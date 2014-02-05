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

#include "system/CSystem"
#include <stdio.h>

#include "../../modules/base/cmodulesystem.h"
#include "../../modules/mpu6050/cmpu6050meta.h"
#include "../../modules/sensors/csensorsmeta.h"

#include "../../system/csystembus.h"

#include "my_memory"

int main()
{
	mongo::BSONObjBuilder initObj;

	mongo::BSONObjBuilder initMpu6050;
	initMpu6050 << "name" << "Mpu6050";
	initMpu6050 << "instance" << "Mpu6050Inst";
	initMpu6050 << "bus_type" << CSystemBus::BusType_I2C;
	initMpu6050 << "bus_name" << "/dev/i2c-1";

	mongo::BSONObjBuilder initSensors;
	initSensors << "name" << "Sensors" << "instance" << "SensorsInst";

	mongo::BSONArrayBuilder modules;
	modules.append(initMpu6050.obj()).append(initSensors.obj());
	initObj << "modules" << modules.arr();

	mongo::BSONArrayBuilder links;
	const char* names[] = { "x", "y", "z" };
	for (int i = 0;i<3;i++) {
		CString name = CString("accel_%1").arg(names[i]);
		mongo::BSONObjBuilder link;
		link << "type" << "pipe";
		link << "outInst" << "Mpu6050Inst";
		link << "inInst" << "SensorsInst";
		link << "outPin" << name.data();
		link << "inPin" << name.data();
		links.append(link.obj());
	}
	for (int i = 0;i<3;i++) {
		CString name = CString("gyro_%1").arg(names[i]);
		mongo::BSONObjBuilder link;
		link << "type" << "pipe";
		link << "outInst" << "Mpu6050Inst";
		link << "inInst" << "SensorsInst";
		link << "outPin" << name.data();
		link << "inPin" << name.data();
		links.append(link.obj());
	}
	initObj << "links" << links.obj();

	CModuleSystem* system = CModuleSystem::instance();
	system->registerModuleMetainformation(new CMpu6050Meta());
	system->registerModuleMetainformation(new CSensorsMeta());

	system->createModules(initObj.obj());

    for (;;) {
    	CSystem::sleep(3000);
    };
    return 0;
}

//#include "system/CSystem"
//
//#include "../../../../modules/mpu6050/cmpu6050.h"
//
//#include "my_memory"
//
//int main()
//{
//	mongo::BSONObjBuilder builder;
//	builder << "name" << "Mpu6050" << "instance" << "Mpu6050Inst";
//	builder << "inputShema" << mongo::BSONObj();
//	builder << "outputs" << mongo::BSONObj();
//
//	CMpu6050* module = new CMpu6050();
//	module->init(builder.obj());
//	module->start();
//
//    for (;;) {
//    	CSystem::sleep(3000);
//    };
//    return 0;
//}
