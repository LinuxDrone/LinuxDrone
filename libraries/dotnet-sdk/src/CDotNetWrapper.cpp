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

//#include "CDotNetWrapper.h"
#include "system/CSystemBusPool"
#include "system/CSystemBus"
#include "system/CSystemPru"
#include "system/CSystem"
#include "system/Logger"

#include <native/task.h>
#define TASK_PRIO 99 /* Highest RT priority */
#define TASK_MODE T_FPU /* Uses FPU, bound to CPU #0 */
#define TASK_STKSZ 4096 /* Stack size (in bytes) */
RT_TASK task_desc;

#include <native/timer.h>

#include "my_memory"

typedef int (*callback_t)(int a, int b);
static callback_t my_callback;

int InvokeManagedCode(int a, int b) {
	if (my_callback == NULL) {
		printf("Managed code has not initialized this library yet");
	}
	return (*my_callback)(a, b);
}

void task_body(void *cookie) {
	int i = 1;
	for (;;) {
		InvokeManagedCode(i++, 5);
		rt_task_sleep(100000000);
	}
}


extern "C" {

void RegisterCallback(callback_t cb) {
	my_callback = cb;
}


int Start() {
	InvokeManagedCode (4, 5);

	int err;
	/* ... */
	err = rt_task_create(&task_desc, "MyTaskName", 	TASK_STKSZ, TASK_PRIO, 	TASK_MODE);
	if (!err)
		rt_task_start(&task_desc, &task_body, NULL);

	return err;
}

int Stop() {
	return rt_task_delete(&task_desc);
}

}



/*
CDotNetWrapper::CDotNetWrapper() :
		CModule(1024) {
}

CDotNetWrapper::~CDotNetWrapper() {
}

bool CDotNetWrapper::init(const mongo::BSONObj& initObject) {
	if (!CModule::init(initObject)) {
		return false;
	}
	CString pathToBin = "PwmIn.bin";
	int pruNumber = 0;
	if (initObject.hasElement("params")) {
		mongo::BSONElement elemParam = initObject["params"];
		mongo::BSONObj objParam = elemParam.Obj();
		if (objParam.hasElement("Pru Binary")) {
			pathToBin = objParam["Pru Binary"].String().c_str();
		}
		if (objParam.hasElement("Pru Device")) {
			pruNumber = (int) objParam["Pru Device"].Number();
		}
	}
	return true;
}

bool CDotNetWrapper::start() {
	CModule::start(this, &CDotNetWrapper::moduleTask);
	return true;
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

bool CDotNetWrapper::initPwmInput() {
	return true;
}

void CDotNetWrapper::moduleTask() {
	RTIME time = rt_timer_read();

	RTIME diff = time - rt_timer_read();
	SRTIME el = rt_timer_ticks2ns(diff);
	uint64_t elapsed = abs(el) / 1000;
	//printf("%5d\r",m_pwm[0]);
	//Logger() << "PwmTASK " << elapsed;
	//CSystem::sleep(10);
}
*/

