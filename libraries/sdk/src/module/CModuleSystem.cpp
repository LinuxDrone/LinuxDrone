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

#include "CModuleSystem.h"
#include "CModuleMetaInfo.h"
#include "CModule.h"
#include "io/CDir"
#include "io/CFile"
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

void CModuleSystem::readAllModules()
{
	pid_t pid = getpid();
	CString processPath = CString("/proc/%1/cmdline").arg(pid);
	CFile file(processPath);
	if (file.open(CIODevice::ReadOnly)) {
		CByteArray data;
		data.setSize(1024);
		int len = file.read((void*)data.data(), 1024);
		if (len == 0) {
			return;
		}
		processPath = CString(data.data(), len);
	} else {
		return;
	}
	processPath.remove(processPath.length()-15, 15);		// minus sizeof "bin/linuxdrone"
	CString pathToModules = CString("%1modules").arg(processPath);
	Logger() << "path to modules: " << pathToModules;

	CDir dir(pathToModules);
	int count = dir.count();
	for (int i = 0;i<count;i++) {
		if (!dir.isFolder(i)) {
			continue;
		}
		CString path = CString("%1/%2").arg(dir.absolutePath()).arg(dir[i]);
		Logger() << path;
		// try load module
		CDir moduleDir(path, "*.so");
		if (moduleDir.count() == 0) {
			continue;
		} else {
			CString modulePath = CString("%1/%2").arg(path).arg(moduleDir[0]);
			Logger() << modulePath;
			void* handle = dlopen(modulePath.data(), RTLD_NOW);
			if (handle == 0) {
				continue;
			}
			ptr_moduleMetainfoCreator sym = (ptr_moduleMetainfoCreator)dlsym(handle, "moduleMetainfoCreator");
			if (sym == 0) {
				continue;
			}
			CModuleMetainfo* info = sym(path);
			registerModuleMetainformation(info);
			info->release();
		}
	}
}

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
	mongo::BSONObjIterator objIt(info);
	while (objIt.more()) {
		mongo::BSONElement elem = objIt.next();
		if (elem.isNull()) {
			continue;
		}
		mongo::BSONObj obj = elem.Obj();
		createModule(obj);
	}
	return true;
}

bool CModuleSystem::createModule(const mongo::BSONObj& moduleInfo)
{
	CString name = moduleInfo["name"].String().c_str();
	CString instance = moduleInfo["instance"].String().c_str();
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
	mongo::BSONObjBuilder builder;
	mongo::BSONObjIterator objIt(moduleInfo);
	while (objIt.more()) {
		mongo::BSONElement elem = objIt.next();
		if (elem.isNull()) {
			continue;
		}
		builder.append(elem);
	}
	builder << "metaInfo" << info->metainformation();
//	Logger() << builder.obj().toString(false, true).c_str();
	if (!module->init(builder.obj())) {
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

void CModuleSystem::linkObjects(const mongo::BSONObj& linksInfo)
{
	mongo::BSONObjIterator objIt(linksInfo);
	while (objIt.more()) {
		mongo::BSONElement elem = objIt.next();
		if (elem.isNull()) {
			continue;
		}
		mongo::BSONObj link = elem.Obj();
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

/////////////////////////////////////////////////////////////////////
//                            working                              //
/////////////////////////////////////////////////////////////////////

void CModuleSystem::start()
{
	CMutexSection locker(&m_mutexModules);
	for (auto it = m_modules.begin();it!=m_modules.end();it++) {
		(*it).second->start();
	}
}

void CModuleSystem::stop()
{
	CMutexSection locker(&m_mutexModules);
	for (auto it = m_modules.begin();it!=m_modules.end();it++) {
		(*it).second->stop();
	}
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================
