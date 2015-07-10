Сборка в Windows для Linux (arm)
=============================================

{@img RunCMake.png}  

**-DCMAKE_MAKE_PROGRAM="C:/Program Files (x86)/Microsoft Visual Studio 12.0/VC/bin/nmake.exe"**  
**-DCMAKE_C_COMPILER=D:/Projects/LinuxDrone/tools/gcc-linaro-arm-linux-gnueabihf-4.8-2013.10_win32/bin/arm-linux-gnueabihf-gcc.exe**  
**-DCMAKE_CXX_COMPILER=D:/Projects/LinuxDrone/tools/gcc-linaro-arm-linux-gnueabihf-4.8-2013.10_win32/bin/arm-linux-gnueabihf-g++.exe**  
**-G"Eclipse CDT4 - NMake Makefiles"**  
**-DCMAKE_TOOLCHAIN_FILE=cmake/boards/beaglebone.cmake**  
**-DCMAKE_BUILD_TYPE=Debug "D:\Projects\LinuxDrone"**  
**-DBOARD_TYPE=beaglebone**  
**-DCROSS_COMPILED_USE=YES**  

*-DCMAKE_MAKE_PROGRAM="C:/Program Files (x86)/Microsoft Visual Studio 12.0/VC/bin/nmake.exe"  -DCMAKE_C_COMPILER=D:/Projects/LinuxDrone/tools/gcc-linaro-arm-linux-gnueabihf-4.8-2013.10_win32/bin/arm-linux-gnueabihf-gcc.exe  -DCMAKE_CXX_COMPILER=D:/Projects/LinuxDrone/tools/gcc-linaro-arm-linux-gnueabihf-4.8-2013.10_win32/bin/arm-linux-gnueabihf-g++.exe -G"Eclipse CDT4 - NMake Makefiles" -DCMAKE_TOOLCHAIN_FILE=cmake/boards/beaglebone.cmake -DCMAKE_BUILD_TYPE=Debug "D:\Projects\LinuxDrone"  -DBOARD_TYPE=beaglebone -DCROSS_COMPILED_USE=YES*
