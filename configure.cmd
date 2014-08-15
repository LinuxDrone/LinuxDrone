@echo off
rem
rem THIS FILE IS TEMPORARY, it will be reworked later
rem

for %%F in (.) do set ROOT_DIR=%%~fF
set CMAKE="%ROOT_DIR%/tools/cmake-2.8.12.2-win32-x86/bin/cmake"
rem set CMAKE="cmake"

# Use cross compiler for build
set CROSS_COMPILED_USE=YES
# Compile code for board
set BOARD_TYPE=beaglebone

rem Remove cmake cache if found in the source directory
%CMAKE% -E remove "%ROOT_DIR%/CMakeCache.txt"
%CMAKE% -E remove_directory "%ROOT_DIR%/CMakeFiles/"

set TOOLCHAIN=-DCMAKE_TOOLCHAIN_FILE=cmake/boards/%BOARD_TYPE%.cmake
set BUILD_TYPE=Debug
rem set BUILD_TYPE=Release

rem If you face any of problems with Eclipse listed here:
rem http://www.cmake.org/Wiki/Eclipse_CDT4_Generator
rem please let us know.
set BUILD="%ROOT_DIR%\build.%BUILD_TYPE%"
set GENERATOR=-G"Eclipse CDT4 - NMake Makefiles"
rem set GENERATOR=-G"Eclipse CDT4 - Unix Makefiles"
rem set GENERATOR=-G"NMake Makefiles"
rem set GENERATOR=-G"Unix Makefiles"

%CMAKE% -E make_directory %BUILD%
cd %BUILD%

rem Stop cmake complains if no CL vars in the PATH
if "%INCLUDE%" == "" set INCLUDE=*
if "%LIB%"     == "" set LIB=*
if "%LIBPATH%" == "" set LIBPATH=*

%CMAKE% %GENERATOR% %TOOLCHAIN% -DCMAKE_BUILD_TYPE=%BUILD_TYPE% "%ROOT_DIR%" %* -DBOARD_TYPE=%BOARD_TYPE% -DCROSS_COMPILED_USE=%CROSS_COMPILED_USE%
if not errorlevel 1 goto Done
echo *** CMAKE ERROR ***

:Done
