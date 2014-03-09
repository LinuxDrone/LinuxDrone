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

#include "module/CModule"
#include "math/CMatrix4f"

class CSimpleTransform : public CModule
{
public:
	CSimpleTransform();
	~CSimpleTransform();

	virtual bool init(const mongo::BSONObj& initObject);

private:
	CMatrix4f m_matrix;

protected:

// notify
	virtual void receivedData() override;
};
