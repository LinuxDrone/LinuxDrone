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
#include "system/Logger"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <mongo/client/dbclient.h>
#pragma GCC diagnostic pop

#include "my_memory"

using namespace mongo;

CSettings::CSettings()
{
	m_dbConnection = 0;
}

CSettings::~CSettings()
{
	if (m_dbConnection) {
		delete m_dbConnection;
		m_dbConnection = 0;
	}
}

int CSettings::Init()
{
	if (m_dbConnection != 0) {
		return EXIT_FAILURE;
	}
	m_dbConnection = new mongo::DBClientConnection();
	string errmsg;
	if (!m_dbConnection->connect(string("127.0.0.1:") + port, errmsg)) {
		Logger() << "couldn't connect : " << errmsg.c_str();
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

mongo::BSONObj CSettings::getModules()
{
	mongo::BSONObj res = m_dbConnection->findOne("test.configuration", mongo::Query().sort("version", -1));
	mongo::BSONObj modules = res.getObjectField("modules");
	return modules.copy();
}

mongo::BSONObj CSettings::getLinks()
{
	mongo::BSONObj res = m_dbConnection->findOne("test.configuration", mongo::Query().sort("version", -1));
	mongo::BSONObj links = res.getObjectField("links");
	return links.copy();
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
