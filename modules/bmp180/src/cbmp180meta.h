
#pragma once

#include "module/CModuleMetaInfo"

class CBmp180Meta : public CModuleMetainfo
{
public:
	CBmp180Meta(const CString& pathToConfig);

	virtual CString moduleName() const;
	virtual CModule* createModule() const;
};
