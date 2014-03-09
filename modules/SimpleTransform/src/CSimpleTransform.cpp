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

#include "CSimpleTransform.h"
#include "system/Logger"
#include "math/CAngle"

#include "my_memory"

extern "C" {
CModule* moduleCreator()
{
	return new CSimpleTransform();
}

const char* moduleName() {
	return "SimpleTransform";
}
}


CSimpleTransform::CSimpleTransform() :
	CModule(1024)
{
	m_matrix = CMatrix4f::identity();
}

CSimpleTransform::~CSimpleTransform()
{
}

bool CSimpleTransform::init(const mongo::BSONObj& initObject)
{
	if (!CModule::init(initObject)) {
		return false;
	}
	if (initObject.hasElement("blocksConfig")) {
		mongo::BSONElement elem = initObject["blocksConfig"];
		mongo::BSONObj blocksConfig = elem.Obj();
		if (!blocksConfig.hasElement("blocks") ||
			!blocksConfig.hasElement("links")) {
			Logger() << __PRETTY_FUNCTION__ << ": error in blocksConfig object. instance = " << this->instance();
			return false;
		}

		// enumerate all blocks
		std::map<CString, mongo::BSONObj> objects;
		elem = blocksConfig["blocks"];
		mongo::BSONObj tmp = elem.Obj();
		mongo::BSONObjIterator it(tmp);
		while (it.more()) {
			elem = it.next();
			mongo::BSONObj obj = elem.Obj();
			if (!obj.hasElement("instance")) {
				Logger() << __PRETTY_FUNCTION__ << ": error in blocksConfig object. instance = " << this->instance();
				return false;
			}
			CString instance = obj["instance"].String().c_str();
			if (objects.count(instance)) {
				Logger() << __PRETTY_FUNCTION__ << ": error in blocksConfig object. instance = " << this->instance();
				return false;
			}
			objects[instance] = obj;
		}
		// enumerate all links
		std::vector<mongo::BSONObj> links;
		std::map<CString, mongo::BSONObj> linksIn;
		std::map<CString, mongo::BSONObj> linksOut;
		elem = blocksConfig["links"];
		tmp = elem.Obj();
		it = mongo::BSONObjIterator(tmp);
		while (it.more()) {
			elem = it.next();
			mongo::BSONObj obj = elem.Obj();
			if (!obj.hasElement("inInst") ||
				!obj.hasElement("outInst")) {
				Logger() << __PRETTY_FUNCTION__ << ": error in blocksConfig object. instance = " << this->instance();
				return false;
			}
			links.push_back(obj);
			CString instance;
			instance = obj["inInst"].String().c_str();
			if (linksIn.count(instance)) {
				Logger() << __PRETTY_FUNCTION__ << ": error in blocksConfig object. instance = " << this->instance();
				return false;
			}
			linksIn[instance] = obj;
			instance = obj["outInst"].String().c_str();
			if (linksOut.count(instance)) {
				Logger() << __PRETTY_FUNCTION__ << ": error in blocksConfig object. instance = " << this->instance();
				return false;
			}
			linksOut[instance] = obj;
		}
		// find first instance
		CString startInstance;
		for (auto it:objects) {
			bool found = true;
			for (auto link:links) {
				if (it.first == link["inInst"].String().c_str()) {
					found = false;
					break;
				}
			}
			if (found) {
				if (!startInstance.isEmpty()) {
					Logger() << __PRETTY_FUNCTION__ << ": error in blocksConfig object. instance = " << this->instance();
					return false;
				}
				startInstance = it.first;
			}
		}
		if (startInstance.isEmpty()) {
			Logger() << __PRETTY_FUNCTION__ << ": error in blocksConfig object. instance = " << this->instance();
			return false;
		}
		std::vector<CString> blocks;
		blocks.push_back(startInstance);
		while (linksOut.count(startInstance)) {
			mongo::BSONObj obj = linksOut[startInstance];
			startInstance = obj["inInst"].String().c_str();
			blocks.push_back(startInstance);
		}
		for (auto it:blocks) {
			mongo::BSONObj obj = objects[it];
			Logger() << obj.toString(false, true).c_str();
			CString name = obj["name"].String().c_str();
			mongo::BSONObj params = obj["params"].Obj();
			if (name == "rotate") {
				bool x, y, z;
				x = params["x"].Bool();
				y = params["y"].Bool();
				z = params["z"].Bool();
				float a = float (params["angle"].Number());
				CAngle angle(a, CAngle::AngleUnit_Degress);
				CMatrix4f matrix = m_matrix * CMatrix4f::makeRotation(angle, float (x), float (y), float (z));
				m_matrix = matrix;
			} else if (name == "translate") {
				float x = float (params["x"].Number());
				float y = float (params["y"].Number());
				float z = float (params["z"].Number());
				CMatrix4f matrix = m_matrix * CMatrix4f::makeTranslate(x, y, z);
				m_matrix = matrix;
			} else if (name == "scale") {
				float x = float (params["x"].Number());
				float y = float (params["y"].Number());
				float z = float (params["z"].Number());
				CMatrix4f matrix = m_matrix * CMatrix4f::makeScale(x, y, z);
				m_matrix = matrix;
			}
		}
	}
	return true;
}

//===================================================================
//  p r o t e c t e d   f u n c t i o n s
//===================================================================

//-------------------------------------------------------------------
//  n o t i f y
//-------------------------------------------------------------------

void CSimpleTransform::receivedData()
{
	const char* names[] = { "in_x", "in_y", "in_z" };
	float values[4];
	for (int i = 0;i<3;i++) {
		CString name(names[i]);
		if (hasElement(name)) {
			values[i] = float (valueNumber(name));
		}
	}
	values[3] = 1.0f;
	CVector4f v(values);
	v = v * m_matrix;

	mongo::BSONObjBuilder builder;
	builder << "out_x" << v.v[0];
	builder << "out_y" << v.v[1];
	builder << "out_z" << v.v[2];
	sendObject(builder.obj());
}
