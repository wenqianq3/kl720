#!/bin/bash
function fail {
    printf '%s\n' "$1"
    exit "${2-1}"
}

build_type=$1
if [[ -z $build_type ]];then
    build_type=Debug
elif [[ $build_type == "debug" ]];then
    build_type=Debug
elif [[ $build_type == "release" ]];then
    build_type=Release
else
    fail "[ERROR] wrong argument! Usage: bulid.sh [debug|release]"
fi

echo Start to compile ...

if [ -d "./build" ];then
    rm -rf ./build
fi
mkdir build
cd build
if [[ -z ${MSYSTEM} ]]; then
    cmake ../../../../platform/kl720/ncpu -DCMAKE_TOOLCHAIN_FILE=../xt-toolchain-for-720.cmake -DCMAKE_BUILD_TYPE=$build_type
    status=$?
else
    cmake ../../../../platform/kl720/ncpu -DCMAKE_TOOLCHAIN_FILE=../xt-toolchain-for-720.cmake -G"MSYS Makefiles" -DCMAKE_BUILD_TYPE=$build_type
    status=$?
fi

[ $status -eq 0 ] && echo "[INFO] cmake done" || fail "[ERROR] cmake failed"

make -j
status=$?
[ $status -eq 0 ] && echo "[INFO] compile done" || fail "[ERROR] compile failed"


fw_util_path="../../../../utils"
echo copying fw_ncpu.bin to bin_gin/flash_bin/
cp -f ./ncpu_main/fw_ncpu.bin $fw_util_path/bin_gen/flash_bin/fw_ncpu.bin
echo copying fw_ncpu.bin to JLink_programmer/bin/
cp -f ./ncpu_main/fw_ncpu.bin $fw_util_path/JLink_programmer/bin/fw_ncpu.bin

echo [Done]

