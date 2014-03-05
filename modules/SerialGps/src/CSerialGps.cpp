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

#include "CSerialGps.h"
#include "my_memory"

extern "C" {
CModule* moduleCreator()
{
    return new CSerialGps();
}

const char* moduleName() {
    return "SerialGps";
}
}



CSerialGps::CSerialGps() :
	CModule(1024)
{
    
}

CSerialGps::~CSerialGps()
{
}

bool CSerialGps::init(const mongo::BSONObj& initObject)
{
    if (!CModule::init(initObject)) 
    {
        return false;
    }
    m_serialPort = "/dev/ttyO2";
    m_serialSpeed = 115200;
    m_gpsProtocol = "NMEA";
    //m_serialType = "Regular";
    if (initObject.hasElement("params")) 
    {
        mongo::BSONElement elemParam = initObject["params"];
        mongo::BSONObj objParam = elemParam.Obj();
        if (objParam.hasElement("Serial Port")) 
        {
            m_serialPort = objParam["Serial Port"].String().c_str();
        }
        if (objParam.hasElement("Serial Speed")) 
        {
            m_serialSpeed = (int)objParam["Serial Speed"].Number();
        }
        if (objParam.hasElement("Serial Type")) 
        {
            CString l_sertype = objParam["Serial Type"].String().c_str();
            if(l_sertype == "RTDM")
            {
                m_serialType = CSerialFactory::SerialType_RTDM;
            }
            else
            {
                m_serialType = CSerialFactory::SerialType_Regular;
            }            
        }
        if (objParam.hasElement("GPS Protocol")) 
        {
            m_gpsProtocol = objParam["GPS Protocol"].String().c_str();
        }
        if (objParam.hasElement("Port Name")) 
        {
            m_portName = objParam["Port Name"].String().c_str();
        }        
    }
    m_serialbus = csf->createSerial(m_serialType);    
    return initSerialGps();
}

bool CSerialGps::start()
{
    CModule::start(this, &CSerialGps::moduleTask);
    return true;
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

bool CSerialGps::initSerialGps()
{
    CString m_serialPort;
    
    int m_serialSpeed;
    CString m_gpsProtocol;
    
    m_serialbus->setPortFile(m_serialPort);
    m_serialbus->setPortSpeed(m_serialSpeed);
    m_serialbus->setPortName(m_portName);
    return m_serialbus->portOpen();
}

uint32_t CSerialGps::readSentence()
{
    int l_sentencesize=0;
    l_sentencesize = m_serialbus->serial_read(&m_sentence,512);
    return l_sentencesize;
}


void CSerialGps::moduleTask()
{
    RTIME time = rt_timer_read();

    int l_sensize = readSentence();

    mongo::BSONObjBuilder builder;

    sendObject(builder.obj());

    RTIME diff = time - rt_timer_read();
    SRTIME el = rt_timer_ticks2ns(diff);
    uint64_t elapsed = abs(el) / 1000;
    //printf("%5d\r",m_pwm[0]);
    //Logger() << "PwmTASK " << elapsed;
    //CSystem::sleep(10);
}
