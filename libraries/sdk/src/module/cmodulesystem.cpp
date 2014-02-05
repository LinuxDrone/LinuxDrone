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

#include "cmodulesystem.h"
#include "cmodulemetainfo.h"
#include "cmodule.h"
#include "io/CDir"
#include <system/Logger>

#include <dlfcn.h>
#include <assert.h>

CModuleSystem::CModuleSystem()
{

}

CModuleSystem::~CModuleSystem()
{
	removeAllModules();
	removeAllInformation();
}

CModuleSystem* CModuleSystem::instance()
{
	static CModuleSystem system;
	return &system;
}

/////////////////////////////////////////////////////////////////////
//                       modules information                       //
/////////////////////////////////////////////////////////////////////

bool CModuleSystem::registerModuleMetainformation(CModuleMetainfo* info)
{
	if (!info) {
		return false;
	}
	CMutexSection locker(&m_mutexInfo);
	if (m_metaInfo.count(info->moduleName())) {
		return false;
	}
	info->addRef();
	m_metaInfo[info->moduleName()] = info;
	return true;
}

void CModuleSystem::removeAllInformation()
{
	CMutexSection locker(&m_mutexInfo);
	for (auto it = m_metaInfo.begin();it!=m_metaInfo.end();it++) {
		SAFE_RELEASE((*it).second);
	}
}

CModuleMetainfo* CModuleSystem::infoByName(const CString& name)
{
	CMutexSection locker(&m_mutexInfo);
	if (m_metaInfo.count(name) == 0) {
		return 0;
	}
	return m_metaInfo[name];
}

/////////////////////////////////////////////////////////////////////
//                        module instances                         //
/////////////////////////////////////////////////////////////////////

bool CModuleSystem::createModules(const mongo::BSONObj& info)
{
	if (info.isEmpty()) {
		return false;
	}
	mongo::BSONElement modulesArray = info["modules"];
	assert(modulesArray.isABSONObj());
	std::vector<mongo::BSONElement> modarr = modulesArray.Array();
	for (auto it = modarr.begin();it<modarr.end();it++) {
		mongo::BSONObj obj = (*it).Obj();
		createModule(obj);
	}
	linkObjects(info);
	return true;
}

bool CModuleSystem::createModule(const mongo::BSONObj& moduleInfo)
{
	CString name = moduleInfo["name"].String().c_str();
	CString instance = moduleInfo["name"].String().c_str();
	{
		CMutexSection locker(&m_mutexModules);
		if (m_modules.count(instance)) {
			Logger() << "detected duplicate instance name (instance =" << instance << ")";
			return false;
		}
	}
	CModuleMetainfo* info = infoByName(name);
	if (!info) {
		Logger() << "error creating module (name=" << name << "). can`t find meta information";
		return false;
	}
	CModule* module = info->createModule();
	if (!module) {
		return false;
	}
	if (!module->init(moduleInfo)) {
		SAFE_RELEASE(module);
		return false;
	}
	{
		CMutexSection locker(&m_mutexModules);
		if (m_modules.count(instance)) {
			SAFE_RELEASE(module);
			assert(false);
			return false;
		}
		m_modules[instance] = module;
	}
	return true;
}

void CModuleSystem::removeAllModules()
{
	CMutexSection locker(&m_mutexModules);
	for (auto it = m_modules.begin();it!=m_modules.end();it++) {
		SAFE_RELEASE((*it).second);
	}
	m_modules.clear();
}

CModule* CModuleSystem::moduleByName(const CString& name)
{
	CMutexSection locker(&m_mutexModules);
	if (m_modules.count(name) == 0) {
		return 0;
	}
	return m_modules[name];
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

void CModuleSystem::linkObjects(const mongo::BSONObj& linksInfo)
{
	if (linksInfo.hasElement("link") == false) {
		return;
	}
	mongo::BSONElement linksArray = linksInfo["link"];
	assert(linksArray.isABSONObj());
	std::vector<mongo::BSONElement> linkarr = linksArray.Array();
	for (auto it = linkarr.begin();it<linkarr.end();it++) {
		const mongo::BSONObj& link = (*it).Obj();
		if (link.isEmpty()) {
			continue;
		}
		// verify link
		if (!link.hasField("type") || !link.hasField("outInst") || !link.hasField("inInst") ||
			!link.hasField("outPin") || !link.hasField("inPin")) {
			Logger() << "failed link (" << link.toString(false, true).c_str() << "). Verify failed";
			continue;
		}
		CModule* module = moduleByName(link["inInst"].String().c_str());
		if (module) {
			module->link(link);
		}
		module = moduleByName(link["outInst"].String().c_str());
		if (module) {
			module->link(link);
		}
	}
}
