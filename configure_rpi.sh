#!/bin/bash -e
#
# THIS FILE IS TEMPORARY, it will be reworked later
#

ROOT_DIR=`pwd`
#CMAKE="$ROOT_DIR/tools/cmake-2.8.12.2/bin/cmake"
CMAKE="cmake"

# Remove cmake cache if found in the source directory
"$CMAKE" -E remove "$ROOT_DIR/CMakeCache.txt"
"$CMAKE" -E remove_directory "$ROOT_DIR/CMakeFiles/"

TOOLCHAIN=-DCMAKE_TOOLCHAIN_FILE=cmake/boards/rpi.cmake
BUILD_TYPE=Debug
#BUILD_TYPE=Release

# If you face any of problems with Eclipse listed here:
# http://www.cmake.org/Wiki/Eclipse_CDT4_Generator
# please let us know.
BUILD="$ROOT_DIR/build.$BUILD_TYPE"
GENERATOR=-G"Eclipse CDT4 - Unix Makefiles"
#GENERATOR=-G"Unix Makefiles"

"$CMAKE" -E make_directory "$BUILD"
cd "$BUILD"

"$CMAKE" "$GENERATOR" $TOOLCHAIN -DCMAKE_BUILD_TYPE=$BUILD_TYPE "$ROOT_DIR" $*
