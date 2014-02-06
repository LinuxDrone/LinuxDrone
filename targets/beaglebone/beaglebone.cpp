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

#include "system/CSystem"
#include "text/CString"
#include "system/Logger"

#include "module/CModuleSystem"

#include "../../modules/mpu6050/cmpu6050meta.h"
#include "../../modules/sensors/csensorsmeta.h"
//#include "../../modules/bmp180/cbmp180meta.h"
//#include "../../modules/hmc5883/chmc5883meta.h"
//#include "../../modules/ms5611/cms5611meta.h"

#include "system/CSettings"

#include "my_memory"

int main()
{
	CSettings settings;
	settings.Init();
	mongo::BSONObj modules = settings.getModules();
	mongo::BSONObj links = settings.getLinks();

	CModuleSystem* system = CModuleSystem::instance();
	system->registerModuleMetainformation(new CMpu6050Meta());
	system->registerModuleMetainformation(new CSensorsMeta());
//	system->registerModuleMetainformation(new CBmp180Meta());
//	system->registerModuleMetainformation(new CHmc5883Meta());
//	system->registerModuleMetainformation(new CMs5611Meta());

	system->createModules(modules);
	system->linkObjects(links);
	system->start();

    for (;;) {
    	CSystem::sleep(3000);
    };
    return 0;
}
