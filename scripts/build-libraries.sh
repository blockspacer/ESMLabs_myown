#!/bin/bash
WORKING_DIR=$(pwd)
export LLVM_TOOLCHAIN=/usr/lib/llvm-6.0/bin
export CC=$LLVM_TOOLCHAIN/clang
export CXX=$LLVM_TOOLCHAIN/clang++
export AR=$LLVM_TOOLCHAIN/llvm-ar
export AS=$LLVM_TOOLCHAIN/llvm-as
export LD=$LLVM_TOOLCHAIN/ld.lld

echo "##################### BEGIN INSTALL BUILD ESSENTIAL... #####################"
sudo apt-get update
sudo apt-get install build-essential

wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
sudo apt-get update
sudo apt install clang-6.0 lld-6.0 
echo "##################### END INSTALL BUILD ESSENTIAL #####################"

echo "##################### BEGIN INSTALL CUDA... #####################"
wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/x86_64/cuda-ubuntu1804.pin
sudo mv cuda-ubuntu1804.pin /etc/apt/preferences.d/cuda-repository-pin-600
wget http://developer.download.nvidia.com/compute/cuda/10.2/Prod/local_installers/cuda-repo-ubuntu1804-10-2-local-10.2.89-440.33.01_1.0-1_amd64.deb
sudo dpkg -i cuda-repo-ubuntu1804-10-2-local-10.2.89-440.33.01_1.0-1_amd64.deb
sudo apt-key add /var/cuda-repo-10-2-local-10.2.89-440.33.01/7fa2af80.pub
sudo apt-get update
sudo apt-get -y install cuda
rm ./cuda-repo-ubuntu1804-10-2-local-10.2.89-440.33.01_1.0-1_amd64.deb
echo "##################### END INSTALL CUDA #####################"

echo "##################### BEGIN BUILD OPENSSL-1.1.1D... #####################"
mkdir -p ../3rdparty/openssl/include
mkdir -p ../3rdparty/openssl/lib
wget https://www.openssl.org/source/openssl-1.1.1d.tar.gz
chmod +x ./openssl-1.1.1d.tar.gz
tar -xzvf ./openssl-1.1.1d.tar.gz
cd openssl-1.1.1d
CWD=$(pwd)
./Configure linux-x86_64-clang
make
mv *.a *.so* *.pc $WORKING_DIR/../3rdparty/openssl/lib
cp -rf include/openssl $WORKING_DIR/../3rdparty/openssl/include
cd $WORKING_DIR
rm openssl-1.1.1d.tar.gz
rm -rf openssl-1.1.1d
echo "##################### END BUILD OPENSSL-1.1.1D #####################"

echo "##################### BEGIN BUILD LIVE555... #####################"
mkdir -p ../3rdparty/live/include
mkdir -p ../3rdparty/live/lib
wget http://live555.com/liveMedia/public/live555-latest.tar.gz
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

echo "##################### BEGIN BUILD GPERFTOOLS-2.7... #####################"
mkdir -p ../3rdparty/gperftools/include
mkdir -p ../3rdparty/gperftools/lib
wget https://github.com/gperftools/gperftools/archive/gperftools-2.7.tar.gz
chmod +x ./gperftools-2.7.tar.gz
tar -xzvf ./gperftools-2.7.tar.gz
cd gperftools-gperftools-2.7
./autogen.sh
./configure --enable-minimal --prefix=$WORKING_DIR/../3rdparty/gperftools
make
make install
cd $WORKING_DIR
rm gperftools-2.7.tar.gz
rm -rf gperftools-gperftools-2.7
echo "##################### END BUILD GPERFTOOLS-2.7 #####################"

echo "##################### BEGIN BUILD BOOST 1.72.0... #####################"
mkdir -p ../3rdparty/boost
wget https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.tar.gz
chmod +x ./boost_1_72_0.tar.gz
tar -xzvf ./boost_1_72_0.tar.gz
cd boost_1_72_0
./bootstrap.sh --prefix=$WORKING_DIR/../3rdparty/boost --with-toolset=clang
./b2
./b2 install
cd $WORKING_DIR
rm boost_1_72_0.tar.gz
rm -rf boost_1_72_0
echo "##################### END BUILD BOOST 1.72.0 #####################"

echo "##################### BEGIN BUILD FFMPEG 4.2.1... #####################"
mkdir -p ../3rdparty/ffmpeg
wget https://ffmpeg.org/releases/ffmpeg-4.2.1.tar.bz2
chmod +x ./ffmpeg-4.2.1.tar.bz2
tar -xvjf ./ffmpeg-4.2.1.tar.bz2
cd ffmpeg-4.2.1
configCmd=("./configure"
     "--arch=x86_64"
     "--enable-yasm"
     "--enable-asm"
     "--enable-static"
     "--enable-shared"
     "--enable-cross-compile"
     "--enable-hardcoded-tables"
     "--prefix=$WORKING_DIR/../3rdparty/ffmpeg")
eval "${configCmd[*]}"
make
make install
cd $WORKING_DIR
rm ffmpeg-4.2.1.tar.bz2
rm -rf ffmpeg-4.2.1
echo "##################### END BUILD FFMPEG 4.2.1 #####################"

echo "##################### BEGIN BUILD TBB 2020... #####################"
mkdir -p ../3rdparty/tbb/include
mkdir -p ../3rdparty/tbb/lib
wget https://github.com/intel/tbb/archive/v2020.0.tar.gz
chmod +x ./v2020.0.tar.gz
tar -xzvf ./v2020.0.tar.gz
cd tbb-2020.0
make compiler=clang arch=intel64
cd ./build/linux_intel64_clang_cc7.4.0_libc2.27_kernel4.15.0_release/
cp *.so* *.def $WORKING_DIR/../3rdparty/tbb/lib
cp -rf ../../include/serial ../../include/tbb $WORKING_DIR/../3rdparty/tbb/include
cd $WORKING_DIR
rm v2020.0.tar.gz
rm -rf tbb-2020.0
echo "##################### END BUILD TBB #####################"

echo "##################### BEGIN BUILD OPENCV 4.1.2... #####################"
mkdir -p ../3rdparty/opencv
wget https://github.com/opencv/opencv/archive/4.1.2.zip
mv 4.1.2.zip opencv-4.1.2.zip
wget https://github.com/opencv/opencv_contrib/archive/4.1.2.zip
mv 4.1.2.zip opencv_contrib-4.1.2.zip
chmod +x ./opencv-4.1.2.zip
chmod +x ./opencv_contrib-4.1.2.zip
unzip ./opencv-4.1.2.zip
unzip ./opencv_contrib-4.1.2.zip
cd opencv-4.1.2
mkdir build
cd build
cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CC_COMPILER=clang -DCMAKE_INSTALL_PREFIX:PATH=$WORKING_DIR/../3rdparty/opencv -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib-4.1.2/modules .. 
make
make install
cd $WORKING_DIR
rm opencv-4.1.2.zip
rm opencv_contrib-4.1.2.zip
rm -rf opencv-4.1.2
rm -rf opencv_contrib-4.1.2
echo "##################### END BUILD OPENCV 4.1.2 #####################"

echo "##################### BEGIN BUILD NANO GUI... #####################"
mkdir -p ../3rdparty/nanogui/include
mkdir -p ../3rdparty/nanogui/lib
git clone https://github.com/wjakob/nanogui.git
cd nanogui
git submodule update --init --recursive
cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CC_COMPLIER=clang -DCMAKE_INSTALL_PREFIX:PATH=$WORKING_DIR/../3rdparty/nanogui
make
make install
cp -rf ./ext/eigen/Eigen $WORKING_DIR/../3rdparty/nanogui/include
cp -rf ./ext/nanovg/src/*.h $WORKING_DIR/../3rdparty/nanogui/include
cd $WORKING_DIR
rm -rf nanogui
echo "##################### END BUILD NANO GUI #####################"
