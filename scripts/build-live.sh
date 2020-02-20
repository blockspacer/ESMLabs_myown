#!/bin/bash
WORKING_DIR=$(pwd)
export LLVM_TOOLCHAIN=/usr/lib/llvm-6.0/bin
export CC=$LLVM_TOOLCHAIN/clang
export CXX=$LLVM_TOOLCHAIN/clang++
export AR=$LLVM_TOOLCHAIN/llvm-ar
export AS=$LLVM_TOOLCHAIN/llvm-as
export LD=$LLVM_TOOLCHAIN/ld.lld

echo "##################### BEGIN BUILD LIVE555... #####################"
mkdir -p ../3rdparty/live/include
mkdir -p ../3rdparty/live/lib
wget http://live555.com/liveMedia/public/live555-latest.tar.gz --no-check-certificate
chmod +x ./live555-latest.tar.gz
tar -xzvf ./live555-latest.tar.gz
cd live
cp ../config.live555-linux-64bit-clang-release .
cp ../config.live555-linux-64bit-clang-debug .
./genMakefiles live555-linux-64bit-clang-release
make
mv ./BasicUsageEnvironment/libBasicUsageEnvironment.a $WORKING_DIR/../3rdparty/live/lib
mv ./groupsock/libgroupsock.a $WORKING_DIR/../3rdparty/live/lib
mv ./liveMedia/libliveMedia.a $WORKING_DIR/../3rdparty/live/lib
mv ./UsageEnvironment/libUsageEnvironment.a $WORKING_DIR/../3rdparty/live/lib
cp -rf ./BasicUsageEnvironment/include/* ./groupsock/include/* ./liveMedia/include/* ./UsageEnvironment/include/* $WORKING_DIR/../3rdparty/live/include
make clean
./genMakefiles live555-linux-64bit-clang-debug
make
mv ./BasicUsageEnvironment/libBasicUsageEnvironment.a $WORKING_DIR/../3rdparty/live/lib/libBasicUsageEnvironment_debug.a
mv ./groupsock/libgroupsock.a $WORKING_DIR/../3rdparty/live/lib/libgroupsock_debug.a
mv ./liveMedia/libliveMedia.a $WORKING_DIR/../3rdparty/live/lib/libliveMedia_debug.a
mv ./UsageEnvironment/libUsageEnvironment.a $WORKING_DIR/../3rdparty/live/lib/libUsageEnvironment_debug.a
make clean
cd $WORKING_DIR
rm live555-latest.tar.gz
rm -rf live
echo "##################### END BUILD LIVE555 #####################"

