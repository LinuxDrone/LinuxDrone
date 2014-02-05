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

#include "CSettings.h"
#include "text/CString"

#include "my_memory"

CSettings::CSettings()
{
}

CSettings::~CSettings()
{
}

bool CSettings::setValue(const CString& name, const mongo::BSONObj& value)
{
	// C style UTF-8 string
//	const char* data = name.data();
	return false;
}

mongo::BSONObj CSettings::value(const CString& value) const
{
	return mongo::BSONObj();
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================
