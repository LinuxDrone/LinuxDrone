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

#include <mongo/bson/bson.h>

class CString;

namespace mongo {
class DBClientConnection;
};

class CSettings
{
public:
	CSettings();
	virtual ~CSettings();

	int Init();
	bool setValue(const CString& name, const mongo::BSONObj& value);
	mongo::BSONObj value(const CString& value) const;

	mongo::BSONObj getModules();
	mongo::BSONObj getLinks();

private:
	const char *port = "27017";
	mongo::DBClientConnection * m_dbConnection;
};
