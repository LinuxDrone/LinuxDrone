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

#include "csensorsmeta.h"
#include "csensors.h"
#include "text/CString"
#include "my_memory"

CSensorsMeta::CSensorsMeta()
{

}

CString CSensorsMeta::moduleName() const
{
	return "Sensors";
}

mongo::BSONObj CSensorsMeta::metainformation() const
{
	mongo::BSONObjBuilder builder;
	builder << "name" << "Sensors";
	builder << "task_priority" << 60;

	return builder.obj();
}

CModule* CSensorsMeta::createModule() const
{
	CSensors* sensors = new CSensors();
	if (!sensors) {
		return 0;
	}
	sensors->addRef();
	return sensors;
}
