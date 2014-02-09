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
#include "system/Logger"

extern "C" {
CModuleMetainfo* moduleMetainfoCreator(const CString& pathToModule)
{
	Logger() << pathToModule;
	CMpu6050Meta* meta = new CMpu6050Meta(CString("%1/configure.json").arg(pathToModule));
	if (meta) {
		meta->addRef();
	}
	return meta;
}
}

CMpu6050Meta::CMpu6050Meta(const CString& pathToModule) :
		CModuleMetainfo(pathToModule)
{
	mongo::BSONObjBuilder builder;
	builder << "name" << "Mpu6050";
	builder << "task_priority" << 40;
	builder << "period" << 1000;

	setMetaObject(builder.obj());
}

CString CMpu6050Meta::moduleName() const
{
	return "Mpu6050";
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
