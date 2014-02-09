#!/bin/bash -e
#
# THIS FILE IS TEMPORARY, it will be reworked later
#

ROOT_DIR=`pwd`
#CMAKE="$ROOT_DIR/tools/cmake-2.8.12.2/bin/cmake"
CMAKE="cmake"

TOOLCHAIN=-DCMAKE_TOOLCHAIN_FILE=cmake/boards/beaglebone.cmake
BUILD_TYPE=Debug
#BUILD_TYPE=Release
BUILD="$ROOT_DIR/build.$BUILD_TYPE"
#GENERATOR=-G"Unix Makefiles"
GENERATOR=-G"Eclipse CDT4 - Unix Makefiles"

"$CMAKE" -E make_directory "$BUILD"
cd "$BUILD"

"$CMAKE" "$GENERATOR" $TOOLCHAIN -DCMAKE_BUILD_TYPE=$BUILD_TYPE "$ROOT_DIR" $*
