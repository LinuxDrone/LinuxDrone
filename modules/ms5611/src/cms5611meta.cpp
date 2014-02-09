
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

#include "cms5611meta.h"
#include "cms5611.h"
#include "my_memory"
#include "system/Logger"

extern "C" {
CModuleMetainfo* moduleMetainfoCreator(const CString& pathToModule)
{
	CMs5611Meta* meta = new CMs5611Meta(CString("%1/configure.json").arg(pathToModule));
	if (meta) {
		meta->addRef();
	}
	return meta;
}
}

CMs5611Meta::CMs5611Meta(const CString& pathToModule) :
		CModuleMetainfo(pathToModule)
{
}

CString CMs5611Meta::moduleName() const
{
	return "Ms5611";
}

CModule* CMs5611Meta::createModule() const
{
	CMs5611* module = new CMs5611();
	if (!module) {
		return 0;
	}
	module->addRef();
	return module;
}
