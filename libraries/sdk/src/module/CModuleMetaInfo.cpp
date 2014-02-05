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

CModuleMetainfo::CModuleMetainfo()
{
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
	return mongo::BSONObj();
}

CModule* CModuleMetainfo::createModule() const
{
	return 0;
}
