@echo off
rem
rem THIS FILE IS TEMPORARY, it will be reworked later
rem

for %%F in (.) do set ROOT_DIR=%%~fF
set CMAKE="%ROOT_DIR%/tools/cmake-2.8.12.2/bin/cmake"
rem set CMAKE="cmake"

set TOOLCHAIN=-DCMAKE_TOOLCHAIN_FILE=cmake/boards/beaglebone.cmake
set BUILD_TYPE=Debug
rem set BUILD_TYPE=Release
set BUILD="%ROOT_DIR%\build.%BUILD_TYPE%"
set GENERATOR=-G"Unix Makefiles"
rem set GENERATOR=-G"NMake Makefiles"
rem set GENERATOR=-G"Eclipse CDT4 - NMake Makefiles"

%CMAKE% -E make_directory %BUILD%
cd %BUILD%

rem Stop cmake complains if no CL vars in the PATH
if "%INCLUDE%" == "" set INCLUDE=*
if "%LIB%"     == "" set LIB=*
if "%LIBPATH%" == "" set LIBPATH=*

%CMAKE% %GENERATOR% %TOOLCHAIN% -DCMAKE_BUILD_TYPE=%BUILD_TYPE% "%ROOT_DIR%" %*
if not errorlevel 1 goto Done
echo *** CMAKE ERROR ***

:Done
