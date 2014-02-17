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
	return true;
}

//===================================================================
//  p r o t e c t e d   f u n c t i o n s
//===================================================================

//-------------------------------------------------------------------
//  n o t i f y
//-------------------------------------------------------------------

void CSimpleTransform::recievedData(const mongo::BSONObj& data)
{
	float x = 0.0f, y = 0.0f, z = 0.0f;
	mongo::BSONObjIterator it(data);
	while (it.more()) {
		mongo::BSONElement elem = it.next();
		if (CString("in_x") == elem.fieldName()) {
			x = float (elem.Number());
		} else if (CString("in_y") == elem.fieldName()) {
			y = elem.Number();
		} else if (CString("in_z") == elem.fieldName()) {
			z = elem.Number();
		}
	}
	CVector4f v(x, y, z, 1.0f);
	v = v * m_matrix;
	mongo::BSONObjBuilder builder;
	builder << "out_x" << v.v[0];
	builder << "out_y" << v.v[1];
	builder << "out_z" << v.v[2];
	sendObject(builder.obj());
}
