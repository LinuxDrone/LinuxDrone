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

#include "text/CString"
#include "thread/CMutex"
#include "../telemetry/CTelemetry.h"
#include <map>
#include <mongo/bson/bson.h>

class CModule;

// main function of module 'CModuleMetainfo* moduleCreator(void)'
typedef CModule* (*ptr_moduleCreator)(void);
// return name of module
typedef const char* (*ptr_moduleName)(void);

class CModuleSystem
{
private:
	typedef struct tagModuleInfo
	{
		ptr_moduleCreator creator;
		ptr_moduleName    name;
	} MODULEINFO, *PMODULEINFO;

public:
	CModuleSystem();
	~CModuleSystem();

	static CModuleSystem* instance();

// modules information
	void readAllModules(const CString& path);
	void removeAllInformation();

// module instances
	bool createModules(const mongo::BSONObj& info);
	bool createModule(const mongo::BSONObj& moduleInfo);
	void linkObjects(const mongo::BSONObj& linksInfo);
	void removeAllModules();
	CModule* moduleByName(const CString& name);

// working
	void start();
	void stop();

private:
	std::map<CString, MODULEINFO> m_metaInfo;
	CMutex m_mutexInfo;

	std::map<CString, CModule*> m_modules;
	CMutex m_mutexModules;

	CTelemetry m_telemetry;
};
