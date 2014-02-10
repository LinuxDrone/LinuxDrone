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

#include "chmc5883meta.h"
#include "chmc5883.h"
#include "my_memory"
#include "system/Logger"

extern "C" {
CModuleMetainfo* moduleMetainfoCreator(const CString& pathToModule)
{
	CHmc5883Meta* meta = new CHmc5883Meta(CString("%1/configure.json").arg(pathToModule));
	if (meta) {
		meta->addRef();
	}
	return meta;
}
}

CHmc5883Meta::CHmc5883Meta(const CString& pathToModule) :
		CModuleMetainfo(pathToModule)
{
}

CString CHmc5883Meta::moduleName() const
{
	return "Hmc5883";
}

CModule* CHmc5883Meta::createModule() const
{
	CHmc5883* module = new CHmc5883();
	if (!module) {
		return 0;
	}
	module->addRef();
	return module;
}
