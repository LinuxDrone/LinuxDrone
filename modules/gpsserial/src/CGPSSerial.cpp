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

#include "CGPSSerial.h"
#include "my_memory"

extern "C" 
{
    CModule* moduleCreator()
    {
        return new CGpsSerial();
    }

    const char* moduleName() 
    {
        return "GpsSerial";
    }
}

CGpsSerial::CGpsSerial() :
	CModule(1024)
{
    m_serialType  = CSerialBus::SerialType_Unknown;
    m_serialSpeed = 0;
}

CGpsSerial::~CGpsSerial()
{
	if(m_serialbus->isOpened())
	{
		m_serialbus->portClose();
	}
}

bool CGpsSerial::init(const mongo::BSONObj& initObject)
{
    if (!CModule::init(initObject)) 
    {
        Logger() << "Fail - InitObject"; 
        return false;
    }
    //Logger() << "Init Method";
    m_serialPort = CString();
    m_serialSpeed = 0;
    m_gpsProtocol = CString();
    m_sentence = CString();
    m_portName = CString();

    bool l_returFcn = true;
    
    if (initObject.hasElement("params")) 
    {
        mongo::BSONElement elemParam = initObject["params"];
        mongo::BSONObj objParam = elemParam.Obj();
        if (objParam.hasElement("Serial Port")) 
        {
            this->m_serialPort = objParam["Serial Port"].String().c_str();
        }
        if (objParam.hasElement("Serial Speed")) 
        {
            this->m_serialSpeed = (int)objParam["Serial Speed"].Number();
        }
        if (objParam.hasElement("Serial Type")) 
        {
            CString l_sertype = objParam["Serial Type"].String().c_str();
            if(l_sertype == "RTDM")
            {
                m_serialType = CSerialBus::SerialType_RT;
            }
            else
            {
                m_serialType = CSerialBus::SerialType_UART;
            }
        }
        if (objParam.hasElement("GPS Protocol")) 
        {
            this->m_gpsProtocol = objParam["GPS Protocol"].String().c_str();
        }
        if (objParam.hasElement("Port Name")) 
        {
            this->m_portName = objParam["Port Name"].String().c_str();
        }        
    }
    
    m_serialbus = CSerialBus::createSerial(m_serialType);
    gpsProtocol = CGPSProtocolFactory::createGPSProtocol(m_gpsProtocol);
    gpsProtocolParser = CGPSParserFactory::createGPSParser(m_gpsProtocol);
    Logger() << "Protocol:" << m_gpsProtocol;
    m_serialbus->setPortFile(m_serialPort);
    m_serialbus->setPortSpeed(m_serialSpeed);
    m_serialbus->setPortName(m_portName);
    //Logger() << "Antes Init";
    l_returFcn = initGpsSerial();
    return l_returFcn;
}

bool CGpsSerial::start()
{
	//Logger() << "Start Method";
    CModule::start(this, &CGpsSerial::moduleTask);
    return true;
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

bool CGpsSerial::initGpsSerial()
{
    std::vector<CByteArray> l_gpsInitCommands;
    int l_retgps=0;
    int l_return;
    l_gpsInitCommands = gpsProtocol->getInitCommands();	
    l_return =m_serialbus->portOpen();
    CSystem::sleep(10);
    for(int i=0;i<l_gpsInitCommands.size();i++)
    {
        CByteArray elemento = l_gpsInitCommands[i];
        l_retgps = m_serialbus->serialWrite(elemento);
        CSystem::sleep(100);
        char str_retorno[1024];        
        int size = m_serialbus->serialRead(&str_retorno, 1024);
    }
    return (bool) l_return;
}

uint32_t CGpsSerial::readSentence()
{
    int l_sentencesize=0;
        
    CByteArray l_queryCommand = gpsProtocol->getQueryCommand();
    CString l_tmpbuf;
    int l_retgps;

    l_retgps = m_serialbus->serialWrite(l_queryCommand);
   
    CSystem::sleep(100);
    char str_retorno[512];    
    int size = m_serialbus->serialRead(&str_retorno, 512);
    CByteArray ba_return = CByteArray(str_retorno,size,false);
    if(ba_return.size() > 0)
    {    
        gpsProtocolParser->setSentence(ba_return);     
        bool l_retparser = gpsProtocolParser->parseSentence();
        if(l_retparser)
        {
            //Logger() << "Builder" << l_retparser;
            mongo::BSONObjBuilder builder;
            builder.append("name", "GpsSerial");

            CString l_tlatitude = gpsProtocolParser->getLatitude() + " " + gpsProtocolParser->getLatitudeDir();
            builder.append("latitude", l_tlatitude.data());
            CString l_tlongitude = gpsProtocolParser->getLongitude() + " " + gpsProtocolParser->getLongitudeDir();
            builder.append("longitude", l_tlongitude.data());
            builder.append("altitude", gpsProtocolParser->getAltitude().data());
            builder.append("time", gpsProtocolParser->getTime().data());
            builder.append("fix", gpsProtocolParser->getGpsFix().data());
            builder.append("satellites", gpsProtocolParser->getnumberofSatellities().data());

    //Logger() << "After Builder";

            mongo::BSONObj obj = builder.obj();
            //Logger() << obj.toString(false, true).c_str();
            addData(obj);
            return l_sentencesize;
        }
        else
        {
            //Logger() << "Invalid parser";
            //Logger() << " Sentence size:" << l_sentencesize;            
            return 0;   
        }
        
    }
    else
    {
        //Logger() << "666";
        //Logger() << "666";
    }
    return 0;
}


void CGpsSerial::moduleTask()
{
    RTIME time = rt_timer_read();

    int l_sensize = readSentence();
    RTIME diff = time - rt_timer_read();
    SRTIME el = rt_timer_ticks2ns(diff);
    uint64_t elapsed = abs(el) / 1000;
}
