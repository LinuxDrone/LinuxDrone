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

#pragma once

#include "module/CModuleMetaInfo"

class CMpu6050Meta : public CModuleMetainfo
{
public:
	CMpu6050Meta();

	virtual CString moduleName() const;
	virtual mongo::BSONObj metainformation() const;
	virtual CModule* createModule() const;
};
