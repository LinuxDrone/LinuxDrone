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
    std::vector<CString> l_gpsInitCommands;
    int l_retgps=0;
    int l_return;
    //Logger() << "Init";
    l_gpsInitCommands = gpsProtocol->getInitCommands();	
    l_return =m_serialbus->portOpen();
    CSystem::sleep(10);
    for(int i=0;i<l_gpsInitCommands.size();i++)
    {
        CString elemento = l_gpsInitCommands[i];
        l_retgps = m_serialbus->serial_write(elemento);
        CSystem::sleep(100);
    }
    return l_return;
}

uint32_t CGpsSerial::readSentence()
{
    int l_sentencesize=0;
    bool l_endsentence = false;
    
    CString l_queryCommand = gpsProtocol->getQueryCommand();
    CString l_tmpbuf;
    int l_retgps;

    //Logger() << "Read Sentence";

    m_sentence = CString();

    l_retgps = m_serialbus->serial_write(l_queryCommand);
//    Logger() << l_queryCommand;
 //   Logger() << "query " << l_queryCommand.mid(1,l_queryCommand.find("*",0)-1);
  //  Logger() << "CHECKSUM:" << l_cnma.getFullSentence(l_queryCommand.mid(1,l_queryCommand.find("*",0) -1));
    int l_charendsequence = gpsProtocol->getEndSentence();
    while(!l_endsentence)
    {
        l_sentencesize = m_serialbus->serial_read(l_tmpbuf,512);
        //if((l_tmpbuf == '\n') || (l_tmpbuf == '\r'))
        //Logger() << "Char: " << l_tmpbuf;
        if(l_tmpbuf ==(char )l_charendsequence)
        {
            l_endsentence = true;       
            break;
        }
        m_sentence+=l_tmpbuf;
    }

    if(m_sentence.size() > 0)
    {
        gpsProtocolParser->setSentence(m_sentence);
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

    //Logger() << "moduleTask method";
    //Logger() << "before readSentence";
    int l_sensize = readSentence();
    //Logger() << "after readSentence -> " << l_sensize;

    //Logger() << "JSON Builder";
/*    mongo::BSONObjBuilder builder;
    builder.append("name", "SerialGps");
    //Logger() << "After Builder";
    sendObject(builder.obj());
*/
    RTIME diff = time - rt_timer_read();
    SRTIME el = rt_timer_ticks2ns(diff);
    uint64_t elapsed = abs(el) / 1000;

    //Logger() << "moduleTask";

    //printf("%5d\r",m_pwm[0]);
    //Logger() << "SerialGPS Task " << elapsed;
    //CSystem::sleep(10);
}
