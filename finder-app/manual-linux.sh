#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u
umask 0022

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all
#    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs
fi

echo "Adding the Image in outdir"
cp -av ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
    echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
mkdir -v ${OUTDIR}/rootfs
cd ${OUTDIR}/rootfs 
mkdir -v bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -v usr/bin usr/lib64 usr/sbin
mkdir -v var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
    git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} distclean
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
else
    cd busybox
fi

# TODO: Make and install busybox
make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} busybox
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} CONFIG_PREFIX=${OUTDIR}/rootfs install
cd ${OUTDIR}/rootfs

echo "Library dependencies"
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs
ARCH_LIB_SO=`${CROSS_COMPILE}gcc -print-sysroot`
cp -av ${ARCH_LIB_SO}/etc/* ${OUTDIR}/rootfs/etc
cp -av ${ARCH_LIB_SO}/lib/* ${OUTDIR}/rootfs/lib
cp -av ${ARCH_LIB_SO}/lib64/* ${OUTDIR}/rootfs/lib64
cp -av ${ARCH_LIB_SO}/sbin/* ${OUTDIR}/rootfs/sbin
#cp -av ${ARCH_LIB_SO}/usr/* ${OUTDIR}/rootfs/usr
cp -av ${ARCH_LIB_SO}/var/* ${OUTDIR}/rootfs/var

# TODO: Make device nodes
cd ${OUTDIR}/rootfs/dev
sudo mknod -m 666 ram0 b 1 0  # all one needs is ram0
sudo mknod -m 666 ram1 b 1 1
ln -s ram0 ramdisk
sudo mknod -m 666 initrd b 1 250
sudo mknod -m 666 mem c 1 1
sudo mknod -m 666 kmem c 1 2
sudo mknod -m 666 null c 1 3
sudo mknod -m 666 port c 1 4
sudo mknod -m 666 zero c 1 5
sudo mknod -m 666 core c 1 6
sudo mknod -m 666 full c 1 7
sudo mknod -m 444 random c 1 8
sudo mknod -m 444 urandom c 1 9
sudo mknod -m 666 aio c 1 10
sudo mknod -m 666 kmsg c 1 11
sudo mknod -m 666 sda b 8 0
sudo mknod -m 666 tty0 c 4 0
sudo mknod -m 666 ttyS0 c 4 64
sudo mknod -m 666 ttyS1 c 4 65
sudo mknod -m 666 tty c 5 0
sudo mknod -m 622 console c 5 1
sudo mknod -m 666 ptmx c 5 2
sudo mknod -m 666 ttyprintk c 5 3
	
ln -s ../proc/self/fd fd
ln -s ../proc/self/fd/0 stdin # process i/o
ln -s ../proc/self/fd/1 stdout
ln -s ../proc/self/fd/2 stderr
ln -s ../proc/kcore kcore

sudo chown root:tty console
sudo chown root:tty ptmx
sudo chown root:tty tty

# TODO: Clean and build the writer utility
cd ${FINDER_APP_DIR}
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} clean
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} writer

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
cp -av ${FINDER_APP_DIR}/writer ${OUTDIR}/rootfs/home
cp -av ${FINDER_APP_DIR}/finder.sh ${OUTDIR}/rootfs/home
cp -av ${FINDER_APP_DIR}/finder-test.sh ${OUTDIR}/rootfs/home
cp -av ${FINDER_APP_DIR}/../conf ${OUTDIR}/rootfs/home/..
cp -av ${FINDER_APP_DIR}/conf ${OUTDIR}/rootfs/home
cp -av ${FINDER_APP_DIR}/autorun-qemu.sh ${OUTDIR}/rootfs/home

# TODO: Chown the root directory
sudo chown -R root.root ${OUTDIR}/rootfs

# TODO: Create initramfs.cpio.gz
cd ${OUTDIR}/rootfs
find . | cpio -o -H newc | gzip > ../initramfs.cpio.gz
