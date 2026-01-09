#!/bin/bash
BUILD_DIR=../build_ios

# Configure the project with the right toolchain in the same folder as this file
cmake -B $BUILD_DIR -G Xcode -DCMAKE_BUILD_TYPE=$CONFIGURATION -DCMAKE_TOOLCHAIN_FILE=/Users/shanehaw/src/Commander-Genius/ios/ios.toolchain.cmake -DPLATFORM=SIMULATORARM64 -DENABLE_ARC=0 .. || exit 1

cmake .. \
  -GXcode \
  -DUSE_OPENGL=0 \
  -DCMAKE_TOOLCHAIN_FILE=/Users/shanehaw/src/Commander-Genius/ios/ios.toolchain.cmake \
  -DPLATFORM=SIMULATORARM64 \
  -DENABLE_ARC=0

# Build the project
cmake --build $BUILD_DIR --target ALL_BUILD --config=$CONFIGURATION || exit 1
