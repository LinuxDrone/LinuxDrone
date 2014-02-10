
#include "cbmp180meta.h"
#include "cbmp180.h"
#include "my_memory"

extern "C" {
CModuleMetainfo* moduleMetainfoCreator(const CString& pathToModule)
{
	CBmp180Meta* meta = new CBmp180Meta(CString("%1/configure.json").arg(pathToModule));
	if (meta) {
		meta->addRef();
	}
	return meta;
}
}

CBmp180Meta::CBmp180Meta(const CString& pathToModule) :
		CModuleMetainfo(pathToModule)
{
}

CString CBmp180Meta::moduleName() const
{
	return "Bmp180";
}

CModule* CBmp180Meta::createModule() const
{
	CBmp180* module = new CBmp180();
	if (!module) {
		return 0;
	}
	module->addRef();
	return module;
}
