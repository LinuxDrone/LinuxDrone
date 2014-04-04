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

#include "CDispConsole.h"
//#include "system/Logger"
//#include <iostream>
//#include <iomanip>
//#include <ncurses.h>
#include <rtdk.h>

#include "my_memory"

extern "C" {
CModule* moduleCreator()
{
	return new CDispConsole();
}

const char* moduleName() {
	return "DispConsole";
}
}


CDispConsole::CDispConsole() :
	CModule(1024)
{
}

CDispConsole::~CDispConsole()
{
}

bool CDispConsole::init(const mongo::BSONObj& initObject)
{
	if (!CModule::init(initObject)) {
		return false;
	}


	if (initObject.hasElement("params")) {
		mongo::BSONElement elemParam = initObject["params"];
		mongo::BSONObj objParam = elemParam.Obj();
		if (objParam.hasElement("Console scroll lines")) {
			scrline = objParam["Console scroll lines"].Number();
		}
	}

	for(int i=0;i<3;i++) in_xyz[i]=0;
	for(int i=0;i<8;i++) in_data[i]=0;

	rt_print_auto_init(1);

	return true;
}

bool CDispConsole::start()
{
	CModule::start(this, &CDispConsole::moduleTask);
	return true;
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

void CDispConsole::moduleTask()
{
	//initscr();
/*
	for(int i=0;i<scrline;i++) Logger() << "";

	Logger()  << "--------------------------";
	Logger() << "in_x = " << in_xyz[0];
	Logger() << "in_y = " << in_xyz[1];
	Logger() << "in_z = " << in_xyz[2];

	Logger()  << "--------------------------";

	Logger() << "in_1 = " << in_data[0];
	Logger() << "in_2 = " << in_data[1];
	Logger() << "in_3 = " << in_data[2];
	Logger() << "in_4 = " << in_data[3];
	Logger() << "in_5 = " << in_data[4];
	Logger() << "in_6 = " << in_data[5];
	Logger() << "in_7 = " << in_data[6];
	Logger() << "in_8 = " << in_data[7];
	Logger()  << "--------------------------";
*/
	for(int i=0;i<scrline;i++) rt_printf("\n");

	rt_printf("--------------------------\n");
	rt_printf("in_x = %8.2f\n", in_xyz[0]);
	rt_printf("in_y = %8.2f\n", in_xyz[1]);
	rt_printf("in_z = %8.2f\n", in_xyz[2]);

	rt_printf("--------------------------\n");

	rt_printf("in_1 = %8.2f\n", in_data[0]);
	rt_printf("in_2 = %8.2f\n", in_data[1]);
	rt_printf("in_3 = %8.2f\n", in_data[2]);
	rt_printf("in_4 = %8.2f\n", in_data[3]);
	rt_printf("in_5 = %8.2f\n", in_data[4]);
	rt_printf("in_6 = %8.2f\n", in_data[5]);
	rt_printf("in_7 = %8.2f\n", in_data[6]);
	rt_printf("in_8 = %8.2f\n", in_data[7]);
	rt_printf("--------------------------\n");

	//m_task.sleep(200);

	//printw("Test ncurses");
	//refresh();
	//for (;;){}
	//endwin();

}
//===================================================================
//  p r o t e c t e d   f u n c t i o n s
//===================================================================

//-------------------------------------------------------------------
//  n o t i f y
//-------------------------------------------------------------------

void CDispConsole::receivedData()
{
	const char* names[] = { "in_x", "in_y", "in_z" };
	for (int i = 0;i<3;i++) {
		if (hasElement(names[i])) {
			in_xyz[i] = float (valueNumber(names[i]));
		}
	}

	char str[16];
	for (int i = 0;i<8;i++) {
		sprintf(str, "in_%d", i+1);
		if (hasElement(str)) {
			in_data[i] = float (valueNumber(str));
		}
	}

	mongo::BSONObjBuilder builder;
	builder << "out_x" << in_xyz[0];
	builder << "out_y" << in_xyz[1];
	builder << "out_z" << in_xyz[2];
	sendObject(builder.obj());
}
