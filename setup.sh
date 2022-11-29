#!/bin/bash
##############################################################################
#                                                                            #
# Copyright 2020 Lukas Jünger                                                #
#                                                                            #
# This software is licensed under the MIT license.                           #
# A copy of the license can be found in the LICENSE file at the root         #
# of the source tree.                                                        #
#                                                                            #
#############################################################################

set -e
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


# If AVP64_SW is set, overwrite files with local version
if [ -z "$AVP64_SW" ]; then
    echo "Error, specify AVP64_SW environment variable"
    exit 0
fi


if [ -d $DIR/build/debug ] || [ -d $DIR/build/release ]; then
    BUILD_DEBUG="$DIR/build/debug"
    BUILD_RELEASE="$DIR/build/release"
else
    BUILD_DEBUG="$DIR/BUILD/DEBUG"
    BUILD_RELEASE="$DIR/BUILD/RELEASE"
fi

rm -rf "$BUILD_DEBUG/sw"
rm -rf "$BUILD_RELEASE/sw"

# Linux Kernel build
AVP64_LINUX_BUILD="$AVP64_SW/linux/BUILD"
AVP64_LINUX_KERNEL_IMAGE="$AVP64_LINUX_BUILD/buildroot/output/linux/images/Image"
AVP64_LINUX_KERNEL_ELF="$AVP64_LINUX_BUILD/buildroot/output/linux/build/linux-4.19.4/vmlinux"
AVP64_LINUX_DTB="$AVP64_LINUX_BUILD/buildroot/output/linux/images/avp64_linux.dtb"
AVP64_LINUX_SDCARD="$AVP64_LINUX_BUILD/buildroot/output/linux/images/sdcard.img"
AVP64_LINUX_BOOT="$AVP64_LINUX_BUILD/linux_bootcode/el3/boot.bin"

if [ -f "$AVP64_LINUX_KERNEL_IMAGE" ]; then
    echo "Found Linux kernel build at $AVP64_LINUX_BUILD"

    mkdir -p "$BUILD_DEBUG/sw/arm64/linux"
    mkdir -p "$BUILD_RELEASE/sw/arm64/linux"
    cp -rv "$AVP64_LINUX_KERNEL_IMAGE" "$BUILD_DEBUG/sw/arm64/linux/Image-4.19.4"
    ln -s -r  "$BUILD_DEBUG/sw/arm64/linux/Image-4.19.4" "$BUILD_RELEASE/sw/arm64/linux/Image-4.19.4"
    cp -rv "$AVP64_LINUX_KERNEL_ELF" "$BUILD_DEBUG/sw/arm64/linux/vmlinux-4.19.4"
    ln -s -r "$BUILD_DEBUG/sw/arm64/linux/vmlinux-4.19.4" "$BUILD_RELEASE/sw/arm64/linux/vmlinux-4.19.4"
    cp -rv "$AVP64_LINUX_DTB" "$BUILD_DEBUG/sw/arm64/linux/linux.dtb"
    ln -s -r "$BUILD_DEBUG/sw/arm64/linux/linux.dtb" "$BUILD_RELEASE/sw/arm64/linux/linux.dtb"
    cp -rv "$AVP64_LINUX_SDCARD" "$BUILD_DEBUG/sw/arm64/linux/sdcard.img"
    chmod -w "$BUILD_DEBUG/sw/arm64/linux/sdcard.img"
    ln -s -r "$BUILD_DEBUG/sw/arm64/linux/sdcard.img" "$BUILD_RELEASE/sw/arm64/linux/sdcard.img"
    cp -rv "$AVP64_LINUX_BOOT" "$BUILD_DEBUG/sw/arm64/linux/boot.bin"
    ln -s -r "$BUILD_DEBUG/sw/arm64/linux/boot.bin" "$BUILD_RELEASE/sw/arm64/linux/boot.bin"

    AVP64X2_LINUX_DTB="$AVP64_LINUX_BUILD/buildroot/output/linux/images/avp64_linux_dualcore.dtb"
    mkdir -p "$BUILD_DEBUG/sw/arm64x2/linux"
    mkdir -p "$BUILD_RELEASE/sw/arm64x2/linux"
    ln -s -r "$BUILD_DEBUG/sw/arm64/linux/Image-4.19.4" "$BUILD_DEBUG/sw/arm64x2/linux/Image-4.19.4"
    ln -s -r "$BUILD_RELEASE/sw/arm64/linux/Image-4.19.4" "$BUILD_RELEASE/sw/arm64x2/linux/Image-4.19.4"
    ln -s -r "$BUILD_DEBUG/sw/arm64/linux/vmlinux-4.19.4" "$BUILD_DEBUG/sw/arm64x2/linux/vmlinux-4.19.4"
    ln -s -r "$BUILD_RELEASE/sw/arm64/linux/vmlinux-4.19.4" "$BUILD_RELEASE/sw/arm64x2/linux/vmlinux-4.19.4"
    cp -rv "$AVP64X2_LINUX_DTB" "$BUILD_DEBUG/sw/arm64x2/linux/smp2_linux.dtb"
    cp -rv "$AVP64X2_LINUX_DTB" "$BUILD_RELEASE/sw/arm64x2/linux/smp2_linux.dtb"
    ln -s -r "$BUILD_DEBUG/sw/arm64/linux/sdcard.img" "$BUILD_DEBUG/sw/arm64x2/linux/sdcard.img"
    ln -s -r "$BUILD_RELEASE/sw/arm64/linux/sdcard.img" "$BUILD_RELEASE/sw/arm64x2/linux/sdcard.img"
    ln -s -r "$BUILD_DEBUG/sw/arm64/linux/boot.bin" "$BUILD_DEBUG/sw/arm64x2/linux/boot.bin"
    ln -s -r "$BUILD_RELEASE/sw/arm64/linux/boot.bin" "$BUILD_RELEASE/sw/arm64x2/linux/boot.bin"
else
    echo "Linux kernel build not found at $AVP64_LINUX_BUILD"
fi

# Xen build
AVP64_XEN_BUILD="$AVP64_SW/xen/BUILD"
AVP64_XEN_KERNEL_IMAGE="$AVP64_XEN_BUILD/buildroot/output/dom0/images/Image.gz"
AVP64_XEN_KERNEL_ELF="$AVP64_XEN_BUILD/buildroot/output/dom0/build/linux-4.19.4/vmlinux"
AVP64_XEN_DTB="$AVP64_XEN_BUILD/buildroot/output/dom0/images/avp64_xen.dtb"
AVP64_XEN_SDCARD="$AVP64_XEN_BUILD/sdcard_image/images/sdcard.img"
AVP64_XEN_BOOT="$AVP64_XEN_BUILD/xen_bootcode/boot.bin"
AVP64_XEN_IMAGE="$AVP64_XEN_BUILD/buildroot/output/dom0/images/xen"
AVP64_XEN_SYMS="$AVP64_XEN_BUILD/buildroot/output/dom0/build/xen-4.13.0/xen/xen-syms"

if [ -f "$AVP64_XEN_IMAGE" ]; then
    echo "Found Xen build at $AVP64_XEN_BUILD"

    mkdir -p "$BUILD_DEBUG/sw/arm64/xen"
    mkdir -p "$BUILD_RELEASE/sw/arm64/xen"
    cp -rv "$AVP64_XEN_KERNEL_IMAGE" "$BUILD_DEBUG/sw/arm64/xen/Image-4.19.4.gz"
    ln -s -r "$BUILD_DEBUG/sw/arm64/xen/Image-4.19.4.gz" "$BUILD_RELEASE/sw/arm64/xen/Image-4.19.4.gz"
    cp -rv "$AVP64_XEN_KERNEL_ELF" "$BUILD_DEBUG/sw/arm64/xen/vmlinux-4.19.4"
    ln -s -r "$BUILD_DEBUG/sw/arm64/xen/vmlinux-4.19.4" "$BUILD_RELEASE/sw/arm64/xen/vmlinux-4.19.4"
    cp -rv "$AVP64_XEN_DTB" "$BUILD_DEBUG/sw/arm64/xen/xen.dtb"
    ln -s -r "$BUILD_DEBUG/sw/arm64/xen/xen.dtb" "$BUILD_RELEASE/sw/arm64/xen/xen.dtb"
    cp -rv "$AVP64_XEN_SDCARD" "$BUILD_DEBUG/sw/arm64/xen/sdcard.img"
    chmod -w "$BUILD_DEBUG/sw/arm64/xen/sdcard.img"
    ln -s -r "$BUILD_DEBUG/sw/arm64/xen/sdcard.img" "$BUILD_RELEASE/sw/arm64/xen/sdcard.img"
    cp -rv "$AVP64_XEN_BOOT" "$BUILD_DEBUG/sw/arm64/xen/boot.bin"
    ln -s -r "$BUILD_DEBUG/sw/arm64/xen/boot.bin" "$BUILD_RELEASE/sw/arm64/xen/boot.bin"
    cp -rv "$AVP64_XEN_IMAGE" "$BUILD_DEBUG/sw/arm64/xen/xen"
    ln -s -r "$BUILD_DEBUG/sw/arm64/xen/xen" "$BUILD_RELEASE/sw/arm64/xen/xen"
    cp -rv "$AVP64_XEN_SYMS" "$BUILD_DEBUG/sw/arm64/xen/xen-syms"
    ln -s -r "$BUILD_DEBUG/sw/arm64/xen/xen-syms" "$BUILD_RELEASE/sw/arm64/xen/xen-syms"

    AVP64X2_XEN_DTB="$AVP64_XEN_BUILD/buildroot/output/dom0/images/avp64_xen_dualcore.dtb"

    mkdir -p "$BUILD_DEBUG/sw/arm64x2/xen"
    mkdir -p "$BUILD_RELEASE/sw/arm64x2/xen"
    ln -s -r "$BUILD_DEBUG/sw/arm64/xen/Image-4.19.4.gz" "$BUILD_RELEASE/sw/arm64x2/xen/Image-4.19.4.gz"
    ln -s -r "$BUILD_DEBUG/sw/arm64/xen/Image-4.19.4.gz" "$BUILD_DEBUG/sw/arm64x2/xen/Image-4.19.4.gz"
    ln -s -r "$BUILD_DEBUG/sw/arm64/xen/vmlinux-4.19.4" "$BUILD_RELEASE/sw/arm64x2/xen/vmlinux-4.19.4"
    ln -s -r "$BUILD_DEBUG/sw/arm64/xen/vmlinux-4.19.4" "$BUILD_DEBUG/sw/arm64x2/xen/vmlinux-4.19.4"
    cp -rv "$AVP64X2_XEN_DTB" "$BUILD_DEBUG/sw/arm64x2/xen/xen_dualcore.dtb"
    ln -s -r "$BUILD_DEBUG/sw/arm64x2/xen/xen_dualcore.dtb" "$BUILD_RELEASE/sw/arm64x2/xen/xen_dualcore.dtb"
    ln -s -r "$BUILD_DEBUG/sw/arm64/xen/sdcard.img" "$BUILD_RELEASE/sw/arm64x2/xen/sdcard.img"
    ln -s -r "$BUILD_DEBUG/sw/arm64/xen/sdcard.img" "$BUILD_DEBUG/sw/arm64x2/xen/sdcard.img"
    ln -s -r "$BUILD_DEBUG/sw/arm64/xen/boot.bin" "$BUILD_RELEASE/sw/arm64x2/xen/boot.bin"
    ln -s -r "$BUILD_DEBUG/sw/arm64/xen/boot.bin" "$BUILD_DEBUG/sw/arm64x2/xen/boot.bin"
    ln -s -r "$BUILD_DEBUG/sw/arm64/xen/xen" "$BUILD_RELEASE/sw/arm64x2/xen/xen"
    ln -s -r "$BUILD_DEBUG/sw/arm64/xen/xen" "$BUILD_DEBUG/sw/arm64x2/xen/xen"
    ln -s -r "$BUILD_DEBUG/sw/arm64/xen/xen-syms" "$BUILD_RELEASE/sw/arm64x2/xen/xen-syms"
    ln -s -r "$BUILD_DEBUG/sw/arm64/xen/xen-syms" "$BUILD_DEBUG/sw/arm64x2/xen/xen-syms"
else
    echo "Xen build not found at $AVP64_XEN_BUILD"
fi

# benchmarks
AVP64_BENCH_BUILD="$AVP64_SW/benchmark/BUILD"

function copy_benchmark {
    BENCH_BUILD=$AVP64_BENCH_BUILD/$1
    BENCH_BIN=$BENCH_BUILD/$1.bin
    BENCH_ELF=$BENCH_BUILD/$1.elf

    if [ -f $BENCH_BIN ]; then
        echo "Found benchmark $1 at $BENCH_BUILD"
        mkdir -p "$BUILD_DEBUG/sw/arm64/benchmark/$1"
        mkdir -p "$BUILD_RELEASE/sw/arm64/benchmark/$1"

        cp -rv "$BENCH_BIN" "$BUILD_DEBUG/sw/arm64/benchmark/$1/$1.bin"
        ln -s -r "$BUILD_DEBUG/sw/arm64/benchmark/$1/$1.bin" "$BUILD_RELEASE/sw/arm64/benchmark/$1/$1.bin"

        if [ -f $BENCH_ELF ]; then
            cp -rv "$BENCH_ELF" "$BUILD_DEBUG/sw/arm64/benchmark/$1/$1.elf"
            ln -s -r "$BUILD_DEBUG/sw/arm64/benchmark/$1/$1.elf" "$BUILD_RELEASE/sw/arm64/benchmark/$1/$1.elf"
        fi
    else
        echo "Binary file of benchmark $1 not found"
    fi
}

copy_benchmark coremark
copy_benchmark dhrystone
copy_benchmark stream
copy_benchmark whetstone
