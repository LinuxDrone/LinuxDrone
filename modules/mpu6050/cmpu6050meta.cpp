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

#include "cmpu6050meta.h"
#include "cmpu6050.h"
#include "my_memory"

CMpu6050Meta::CMpu6050Meta()
{
}

CString CMpu6050Meta::moduleName() const
{
	return "Mpu6050";
}

mongo::BSONObj CMpu6050Meta::metainformation() const
{
	mongo::BSONObjBuilder builder;
	builder << "name" << "Mpu6050";
	builder << "task_priority" << 40;

	mongo::BSONObj obj = builder.obj();
	return obj;
}

CModule* CMpu6050Meta::createModule() const
{
	CMpu6050* module = new CMpu6050();
	if (!module) {
		return 0;
	}
	module->addRef();
	return module;
}
