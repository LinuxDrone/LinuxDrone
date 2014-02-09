
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

#include "cpwmoutputmeta.h"
#include "cpwmoutput.h"
#include "my_memory"
#include "system/Logger"

extern "C" {
CModuleMetainfo* moduleMetainfoCreator(const CString& pathToModule)
{
	CPwmOutputMeta* meta = new CPwmOutputMeta(CString("%1/configure.json").arg(pathToModule));
	if (meta) {
		meta->addRef();
	}
	return meta;
}
}

CPwmOutputMeta::CPwmOutputMeta(const CString& pathToModule) :
		CModuleMetainfo(pathToModule)
{
}

CString CPwmOutputMeta::moduleName() const
{
	return "PwmOutput";
}

CModule* CPwmOutputMeta::createModule() const
{
	CPwmOutput* module = new CPwmOutput();
	if (!module) {
		return 0;
	}
	module->addRef();
	return module;
}
