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
#include "system/CSystem"
#include "system/Logger"
#include "system/CSerialBus"
#include "system/CAbstractSerial"
#include "system/CSerialFactory"

#include <native/timer.h>


class CSerialGps : public CModule
{
public:
	CSerialGps();
	~CSerialGps();

	virtual bool init(const mongo::BSONObj& initObject);
	bool start();

private:
    //Methods
    bool initSerialGps();
    void moduleTask();
    uint32_t readSentence();
    
    //Variables
    CAbstractSerial *m_serialbus;    
    CString m_serialPort;
    CString m_sentence;
    CString m_portName;
    int m_serialSpeed;
    CString m_gpsProtocol;
    CSerialFactory* csf = new CSerialBus;    
    //CString m_serialType;
    CSerialFactory::SerialType m_serialType;
};
