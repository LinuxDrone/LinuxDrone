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

#include "CModuleMetaInfo.h"
#include "io/CFile"
#include "system/Logger"

#include <mongo/db/json.h>

CModuleMetainfo::CModuleMetainfo(const CString& pathToConfig)
{
	if (pathToConfig.isEmpty()) {
		return;
	}
	CFile file;
	file.setFileName(pathToConfig);
	if (file.open(CIODevice::ReadOnly)) {
		CByteArray data = file.readAll();
		if (!data.isEmpty()) {
			int len = data.size();
			m_metaObject = mongo::fromjson(data.data(), &len);
			Logger() << m_metaObject.toString(false, true).c_str();
		}
		file.close();
	}
}

CModuleMetainfo::~CModuleMetainfo()
{
}

CString CModuleMetainfo::moduleName() const
{
	return CString();
}

mongo::BSONObj CModuleMetainfo::metainformation() const
{
	return m_metaObject;
}

CModule* CModuleMetainfo::createModule() const
{
	return 0;
}

//===================================================================
//  p r o t e c t e d   f u n c t i o n s
//===================================================================

void CModuleMetainfo::setMetaObject(const mongo::BSONObj& metaObject)
{
	m_metaObject = metaObject.copy();
}
