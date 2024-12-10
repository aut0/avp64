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

LINUX_SW=https://github.com/aut0/avp64_sw/releases/download/v2024.08.02/buildroot_6_5_6.tar.gz
LINUX_SRC=https://mirrors.edge.kernel.org/pub/linux/kernel/v6.x/linux-6.5.6.tar.gz
LINUX_ARCHIVE="${LINUX_SRC##*/}"
LINUX_DIRNAME="${LINUX_ARCHIVE%.tar.gz}"
ROOT_DIR=${DIR}/..
VSCODE_DIR=${ROOT_DIR}/.vscode

# fetch sw
wget ${LINUX_SW} -O linux.tar.gz
tar -xvf linux.tar.gz -C ${ROOT_DIR}/sw
rm linux.tar.gz

# get linux sources
wget ${LINUX_SRC}
tar -xf ${LINUX_ARCHIVE}
rm ${LINUX_ARCHIVE}
mv ${LINUX_DIRNAME} "${ROOT_DIR}/linux-src/"

# setup .vscode folder
mkdir "${ROOT_DIR}/.vscode"
cp "${DIR}/settings.json" "${VSCODE_DIR}/"
cp "${DIR}/launch.json" "${VSCODE_DIR}/"

# open VS Code
code ${ROOT_DIR}
