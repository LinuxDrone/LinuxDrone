Сборка в MacOS для Linux (arm)
=============================================

Первым делом, присваиваем переменной **PLATFORM** значение **XENO** в файле **CMakeLists.txt**, в корне проекта.  
{@img set-platform.png}  

## Сборка с использованием QT Creator

### Определить тулчейн
В настройках QT Creator укажем компилятор  
{@img gcc.png}
Пример пути к компилятору:
**/Users/vrubel/LinuxDrone/tools/gcc-linaro-arm-linux-gnueabihf-4.8-2013.10_osx/bin/arm-linux-gnueabihf-gcc**

И заведем новый тулчейн  
{@img toolchain.png}

### Указать мастеру cmake правильные параметры
{@img cmake.png}

Строка с параметрами cmake:  
**-DCROSS_COMPILED_USE=YES -DBOARD_TYPE=beaglebone -DCMAKE_TOOLCHAIN_FILE=cmake/boards/beaglebone.cmake**
