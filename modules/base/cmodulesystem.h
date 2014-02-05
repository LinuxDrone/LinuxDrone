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

#ifndef __CMODULE_SYSTEM_H__
#define __CMODULE_SYSTEM_H__

#include "text/CString"
#include "thread/CMutex"
#include <map>
#include <mongo/bson/bson.h>

class CModuleMetainfo;
class CModule;

class CModuleSystem
{
public:
	CModuleSystem();
	~CModuleSystem();

	static CModuleSystem* instance();

// modules information
	bool registerModuleMetainformation(CModuleMetainfo* info);
	void removeAllInformation();
	CModuleMetainfo* infoByName(const CString& name);

// module instances
	bool createModules(const mongo::BSONObj& info);
	bool createModule(const mongo::BSONObj& moduleInfo);
	void removeAllModules();
	CModule* moduleByName(const CString& name);

private:
	std::map<CString, CModuleMetainfo*> m_metaInfo;
	CMutex m_mutexInfo;

	std::map<CString, CModule*> m_modules;
	CMutex m_mutexModules;

	void linkObjects(const mongo::BSONObj& linksInfo);
};

#endif // __CMODULE_SYSTEM_H__
