#!/bin/sh
#
# This scripts uses CMake to build protobuf with static libraries using MinGW
# W64 targeting Windows x64. It also creates a debian package with cpack. The
# contents of the package are installed under "/usr/x86_64-w64-mingw32".
#

# Include binaries in the cura development environment
CURA_DEV_ENV_ROOT=/opt/cura-dev
export PATH="${CURA_DEV_ENV_ROOT}/bin:${PATH}"

apt -y remove cmake
apt -y --no-install-recommends install python3-pip
pip3 install cmake --upgrade

mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw_toolchain.cmake ..
make
cpack \
    --config ../cmake/cpack_config_deb_mingw64.cmake \
    -D CPACK_INSTALL_CMAKE_PROJECTS="$(pwd);libArcus;ALL;/"
