#!/bin/bash

rm ./android/rctmgl/src/main/jniLibs/arm64-v8a/*.so
rm ./android/rctmgl/src/main/jniLibs/armeabi-v7a/*.so
rm ./android/rctmgl/src/main/jniLibs/x86/*.so
rm ./android/rctmgl/src/main/jniLibs/x86_64/*.so

cd example/android
#./gradlew clean
./gradlew build
cd ../..

cd android/rctmgl/build/intermediates/cxx/RelWithDebInfo
cd $(ls -d */|head -n 1)
mkdir -p ../../../../../src/main/jniLibs/arm64-v8a
mkdir -p ../../../../../src/main/jniLibs/armeabi-v7a
mkdir -p ../../../../../src/main/jniLibs/x86
mkdir -p ../../../../../src/main/jniLibs/x86_64
cp obj/arm64-v8a/*.so ../../../../../src/main/jniLibs/arm64-v8a/
cp obj/armeabi-v7a/*.so ../../../../../src/main/jniLibs/armeabi-v7a/
cp obj/x86/*.so ../../../../../src/main/jniLibs/x86/
cp obj/x86_64/*.so ../../../../../src/main/jniLibs/x86_64/

cd ../../../../..
pwd
sed -r -i 's/^[ , \n, \t]*path[ , \n, \t]*\"[.,\/,a-z, A-Z, 0-9]*CMakeLists\.txt\"/\/*&*\//g' ./build.gradle

printf "\nprepared sources for packing\n"