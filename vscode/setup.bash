#!/bin/env bash

##############################################################################
#                                                                            #
# Copyright 2023 Nils Bosbach                                                #
#                                                                            #
# This software is licensed under the MIT license.                           #
# A copy of the license can be found in the LICENSE file at the root         #
# of the source tree.                                                        #
#                                                                            #
##############################################################################


# Get directory of script itself
SOURCE="${BASH_SOURCE[0]}"
# resolve $SOURCE until the file is no longer a symlink
while [ -h "$SOURCE" ]; do
        DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
        SOURCE="$(readlink "$SOURCE")"
        # if $SOURCE was a relative symlink, we need to resolve it relative to the
        # path where the symlink file was located
        [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE"
done
DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"

LINUX_SW=https://github.com/aut0/avp64_sw/releases/latest/download/linux.tar.gz
LINUX_SRC=https://mirrors.edge.kernel.org/pub/linux/kernel/v4.x/linux-4.19.4.tar.gz
LINUX_ARCHIVE="${LINUX_SRC##*/}"
LINUX_DIRNAME="${LINUX_ARCHIVE%.tar.gz}"
ROOT_DIR=${DIR}/..
VSCODE_DIR=${ROOT_DIR}/.vscode
EXTENSIONS=(llvm-vs-code-extensions.vscode-clangd twxs.cmake ms-vscode.cmake-tools vadimcn.vscode-lldb webfreak.debug)

# fetch linux
wget ${LINUX_SW}
tar -xvf linux.tar.gz
rm linux.tar.gz

# copy sw folder
mkdir -p ${ROOT_DIR}/build/{debug,release}
mv sw ${ROOT_DIR}/build/debug
ln -s -r ${ROOT_DIR}/build/debug/sw ${ROOT_DIR}/build/release/sw

# get linux sources
wget ${LINUX_SRC}
tar -xf ${LINUX_ARCHIVE}
rm ${LINUX_ARCHIVE}
mv ${LINUX_DIRNAME} ${ROOT_DIR}/build/release/sw/arm64/linux/

# setup .vscode folder
mkdir ${ROOT_DIR}/.vscode
cp ${DIR}/settings.json ${VSCODE_DIR}/
cp ${DIR}/launch.json ${VSCODE_DIR}/

# apply patches
pushd ${ROOT_DIR}/deps/ocx-qemu-arm/unicorn > /dev/null
git apply ${ROOT_DIR}/patches/unicorn-*.patch
popd > /dev/null

# open VS Code
code ${ROOT_DIR}
