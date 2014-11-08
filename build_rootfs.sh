#!/bin/sh -e

#--------------------------------------------------------------------
# This file was created as a part of the LinuxDrone project:
#                http://www.linuxdrone.org
#
# Distributed under the Creative Commons Attribution-ShareAlike 4.0
# International License (see accompanying License.txt file or a copy
# at http://creativecommons.org/licenses/by-sa/4.0/legalcode)
#
# The human-readable summary of (and not a substitute for) the
# license: http://creativecommons.org/licenses/by-sa/4.0/
#--------------------------------------------------------------------

echo "\nThe script is designed to build a system image for LinuxDrone project.\n"

USAGE="Example usage: `basename $0` -b bbb -d /dev/sdc\n\n
------------ Options -------------- \n
help:                            -h \n
version:                         -v \n
use menuconfig for buld kernel:  -m \n
sdcard device name:              -d (dev/sdc)   \n
board name:                      -b (bbb, rpi)  \n
"


print() {
#2>&1
date +%d-%m-%Y\ %H:%M:%S | tr -d '\012' | tee -a ${LDROOT_DIR}/build_rootfs.log
echo " --- $1" | tee -a ${LDROOT_DIR}/build_rootfs.log

#date +%d-%m-%Y\ %H:%M:%S | tr -d '\012'; echo " --- $1";
}

sudo_timestamp_timeout() {

FIND_STRING=Defaults:$(whoami)[[:blank:]]*timestamp_timeout=

if sudo grep -q "${FIND_STRING}" /etc/sudoers; then
    print "Set timestamp_timeout=$1 for $(whoami) in /etc/sudoers"
    sudo sed -i "s/${FIND_STRING}.*/Defaults:$(whoami)\t\ttimestamp_timeout=$1/" /etc/sudoers
else
    print "Add timestamp_timeout=$1 for $(whoami) in /etc/sudoers"
    sudo sed -i "$ a Defaults:$(whoami)\t\ttimestamp_timeout=$1" /etc/sudoers
fi
}

qemu_copy_to_roofs() {
if [ ! -f ${CHROOT_DIR}/usr/bin/qemu-arm-static ]; then
    sudo cp /usr/bin/qemu-arm-static ${CHROOT_DIR}/usr/bin/
fi
}

qemu_remove_from_roofs() {
if [ -f ${CHROOT_DIR}/usr/bin/qemu-arm-static ]; then
    sudo rm ${CHROOT_DIR}/usr/bin/qemu-arm-static
fi
}

# Umount file systems proc, sysfs, devpts
umount_proc_sysfs_devpts() {
if mount | grep -q ${CHROOT_DIR}/proc; then
  print "Umount file systems proc, sysfs, devpts"
  sudo umount ${CHROOT_DIR}/proc
  sudo umount ${CHROOT_DIR}/sys
  sudo umount ${CHROOT_DIR}/dev/pts
fi
}

# Mount file systems proc, sysfs, devpts
mount_proc_sysfs_devpts() {
if mount | grep -q ${CHROOT_DIR}/proc; then
  umount_proc_sysfs_devpts
fi
print "Mount file systems proc, sysfs, devpts"
sudo mount proc ${CHROOT_DIR}/proc -t proc
sudo mount sysfs ${CHROOT_DIR}/sys -t sysfs
sudo mount devpts ${CHROOT_DIR}/dev/pts -t devpts
}

# Run command in chroot
run_cmd_chroot() {
qemu_copy_to_roofs
mount_proc_sysfs_devpts
sudo chroot ${CHROOT_DIR} /bin/bash -c "$1"
umount_proc_sysfs_devpts
qemu_remove_from_roofs
}

# Create image sdcard
create_image() {
print "Start create image sdcard"
# Argument 1 name file image
# Argument 2 size image in Mb
#DISK_DIR=`dirname $1`
#DISK_SIZE=$2
SDCARD=$1
DISK_IMAGE=${BOARD_DIR}/${BOARD}-ld.img

# Only assumed two partitions, best to umount all before running this script
if [ $(mount | grep -c /dev/mapper/loop) -ne 0 ]; then
  sudo umount /dev/mapper/loop*
  print "umount /dev/mapper/loop*"
fi

# Удаляем устройства-разделы с помощью kpartx
if [ -f ${DISK_IMAGE} ]; then
    sudo kpartx -dv ${DISK_IMAGE}
fi

# Only assumed two partitions, best to umount all before running this script
#if [ $(mount | grep -c /dev/loop0) -eq 1 ]; then
#  sudo umount /dev/loop0
#  print "umount /dev/loop0"
#fi

# Create a directory to put the image of the root file system board
sudo mkdir -p /mnt/boot
sudo mkdir -p /mnt/rootfs

if  [ ${DISK_IMAGE_CLEAN} = YES ] || \
    [ ! -f ${DISK_IMAGE} ];then
    # Delete old image disk
    if [ -f ${DISK_IMAGE} ]; then
        sudo rm ${DISK_IMAGE}
    fi

    # Создаем пустой образ диска
    dd if=/dev/zero of=${DISK_IMAGE} count=0 bs=1M seek=2048

    # Create a new partition table
    # Size partiton /boot = 100M
sfdisk --in-order --Linux --unit M ${DISK_IMAGE} <<-__EOF__
1,100,0xE,*
,,,-
__EOF__


    # Подключаем образ диска
    DEV_LOOP=$(sudo kpartx -av ${DISK_IMAGE} | grep -o 'loop.p' | uniq | grep -o 'loop.')

    # Format the card sections
    sudo mkfs.vfat -F 16 /dev/mapper/${DEV_LOOP}p1 -n boot
    sudo mkfs.ext4 /dev/mapper/${DEV_LOOP}p2 -L rootfs
    sync
    sudo kpartx -dv ${DISK_IMAGE}
fi

# Подключаем образ диска
DEV_LOOP=$(sudo kpartx -av ${DISK_IMAGE} | grep -o 'loop.p' | uniq | grep -o 'loop.')

# Монтируем созданные разделы
sudo mount -t vfat /dev/mapper/${DEV_LOOP}p1 /mnt/boot -o rw,shortname=mixed,dmask=0000,utf8=1,showexec,flush
sudo mount -t ext4 /dev/mapper/${DEV_LOOP}p2 /mnt/rootfs -o rw

print "Копируем файлы из rootfs в образ диска"
sudo rsync -a --delete --exclude="/boot/*" ${CHROOT_DIR}/ /mnt/rootfs/
sudo cp -u ${CHROOT_DIR}/boot/* /mnt/boot/
sync

# Размонтируем образ диска
sudo umount /dev/mapper/${DEV_LOOP}p1
sudo umount /dev/mapper/${DEV_LOOP}p2
sudo kpartx -dv ${DISK_IMAGE}

print "create arhive the image sdcard"
cd ${BOARD_DIR}
if [ -f ${DISK_IMAGE}.bz2 ]; then
    sudo rm ${DISK_IMAGE}.bz2
fi
#bzip2 -9 ${DISK_IMAGE}

print "Copy image to sdcard"
#sudo sh -c "bzcat ${DISK_IMAGE}.bz2 > /dev/${SDCARD}"
#sudo dd if=${DISK_IMAGE} of=/dev/${SDCARD} bs=1M

print "End create image sdcard"
}

# Setting common settings for the rootfs
rootfs_common_settings() {

print "Setting hostname"
sudo sh -c "echo 'linuxdrone' > ${CHROOT_DIR}/etc/hostname"
sudo sh -c "echo '127.0.0.1\tlinuxdrone' >> ${CHROOT_DIR}/etc/hosts"

print "Edit network interface"
sudo sh -c "cat> ${CHROOT_DIR}/etc/network/interfaces << EOF
auto lo
iface lo inet loopback

auto eth0
iface eth0 inet dhcp
EOF
"

#print "Set locale to ignore warnings"
#sudo sh -c "cat >> ${CHROOT_DIR}/root/.bashrc << EOF
#LC_ALL=C
#LANGUAGE=C
#LANG=C
#EOF
#"

print "Create /etc/resolv.conf"
sudo cp /etc/resolv.conf ${CHROOT_DIR}/etc

print "Create xenomai group"
sudo chroot ${CHROOT_DIR} /bin/bash -c "addgroup xenomai"
sudo chroot ${CHROOT_DIR} /bin/bash -c "addgroup root xenomai"
print "Create user linuxdrone"
sudo chroot ${CHROOT_DIR} /bin/bash -c "useradd -m ld -c LinuxDrone,,,,"
sudo chroot ${CHROOT_DIR} /bin/bash -c "usermod -a -G xenomai,sudo,staff,kmem,plugdev ld"
sudo chroot ${CHROOT_DIR} /bin/bash -c "cat << EOF | passwd ld
1
1
EOF
"

print "Choice of the shell for the user ld"
sudo chroot ${CHROOT_DIR} /bin/bash -c "chsh -s /bin/bash ld"

print "Set root password"
sudo chroot ${CHROOT_DIR} /bin/bash -c "cat << EOF | passwd
1
1
EOF
"

print "Configure udev rules"
sudo sh -c "cat >${CHROOT_DIR}/etc/udev/rules.d/xenomai.rules<<EOF
# allow RW access to /dev/mem
KERNEL=="mem", MODE="0660", GROUP="kmem"
# real-time heap device (Xenomai:rtheap)
KERNEL=="rtheap", MODE="0660", GROUP="xenomai"
# real-time pipe devices (Xenomai:rtpipe)
KERNEL=="rtp[0-9]*", MODE="0660", GROUP="xenomai"
EOF
"
}

# Installing packages in rootfs
rootfs_installing_packages() {

print "Updating apt sources and installing packages"
sudo chroot ${CHROOT_DIR} /bin/bash -c "apt-get update"

sudo chroot ${CHROOT_DIR} /bin/bash -c "apt-get -y install locales locales-all"
sudo chroot ${CHROOT_DIR} /bin/bash -c "locale-gen ${LOCALES}"
sudo chroot ${CHROOT_DIR} /bin/bash -c "dpkg-reconfigure locales-all"

sudo chroot ${CHROOT_DIR} /bin/bash -c "apt-get -y upgrade"

sudo chroot ${CHROOT_DIR} /bin/bash -c "apt-get -y install ssh sudo mc wget git screen build-essential cpufrequtils i2c-tools usbutils wpasupplicant wireless-tools gdbserver"
sudo chroot ${CHROOT_DIR} /bin/bash -c "apt-get -y install libboost-system.dev libboost-filesystem.dev libboost-thread.dev libboost-program-options.dev"
sudo chroot ${CHROOT_DIR} /bin/bash -c "apt-get -y install scons htop ntpdate libssl-dev"

if [ $DISTR = wheezy ]; then
    print "compiling and installing packages"
    compiled_and_install_mongodb_rpi
else
    sudo chroot ${CHROOT_DIR} /bin/bash -c "apt-get -y install mongodb nodejs npm "
#    sudo chroot ${CHROOT_DIR} /bin/bash -c "apt-get -y install libwebsockets-dev libwebsockets-test-server libwebsockets3 libwebsockets3-dbg"
fi

sudo chroot ${CHROOT_DIR} /bin/bash -c "apt-get clean"
sudo chroot ${CHROOT_DIR} /bin/bash -c "apt-get autoremove"
}


# Manual additional installing packages
start_shell_in_chroot() {
print "Starting chroot bash session, type exit to end"
qemu_copy_to_roofs
mount_proc_sysfs_devpts
sudo chroot ${CHROOT_DIR} /bin/bash

sudo chroot ${CHROOT_DIR} /bin/bash -c "apt-get clean"
sudo chroot ${CHROOT_DIR} /bin/bash -c "apt-get autoremove"
umount_proc_sysfs_devpts
qemu_remove_from_roofs
}


# Download cross-compiler
download_install_crosscompiler_rpi() {
if [ ! -f ${CC_DIR}/bin/arm-linux-gnueabihf-gcc ]; then
  if [ ! -d ${LDDOWNL_DIR} ]; then
      print "Create a directory where all downloads soft"
      mkdir -p ${LDDOWNL_DIR}
  fi

  if [ ! -d ${CC_DIR} ]; then
      print "Create a directory for cross-compiler"
      mkdir -p ${CC_DIR}
  elif [ $(find ${CC_DIR} -type f | wc -l) -ne 0 ]; then
      print "Clean ${CC_DIR}"
      rm -R ${CC_DIR}/*
  fi

  if [ ! -f ${LDDOWNL_DIR}/rpi-cc-master.tar.gz ]; then
    print "Download cross-compiler rpi"
    wget -O ${LDDOWNL_DIR}/rpi-cc-master.tar.gz https://github.com/raspberrypi/tools/archive/master.tar.gz
  fi

  print "Installing cross-compiler rpi"
  cd ${CC_DIR}
  tar -xzf ${LDDOWNL_DIR}/rpi-cc-master.tar.gz tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64
  mv ./tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/* .
  rm -R ./tools-master
else
  print "Cross-compiler is already installed for rpi"
fi

export CROSS=${CC_DIR}/bin/arm-linux-gnueabihf-
export PATH=${CC_DIR}/bin:$PATH
}


# Download, configure and build libwebsockets
compiled_and_install_libwebsockets() {
print "Start compiled_and_install_libwebsockets"

if [ -f ${CHROOT_DIR}/usr/local/include/libwebsockets.h ]; then
    print "libwebsockets is already installed"
    return
fi

if [ ! -d "${MAKE_DIR}/libwebsockets" ]; then
    if [ ! -f ${LDDOWNL_DIR}/${NAME_LIBWEB} ]; then
        print "Download libwebsockets ${NAME_LIBWEB}"
        wget -c  -P ${LDDOWNL_DIR} http://git.libwebsockets.org/cgi-bin/cgit/libwebsockets/snapshot/${NAME_LIBWEB}
    fi
    cd ${MAKE_DIR}
    print "unpacking ${NAME_LIBWEB} intro ${MAKE_DIR}"
    tar -xzf ${LDDOWNL_DIR}/${NAME_LIBWEB}
    mv libwebsockets* libwebsockets
    #git clone git://git.libwebsockets.org/libwebsockets
fi

cd ${MAKE_DIR}/libwebsockets
sed -i "s:/opt/gcc-linaro-arm-linux-gnueabihf-4.7-2013.02-01-20130221_linux:${CC_DIR}:"\
      ${MAKE_DIR}/libwebsockets/cross-arm-linux-gnueabihf.cmake

LWS_IPV6=ON
if [ ${BOARD} = rpi ]; then
    #   sed -i 's/LWS_IPV6 "Compile with support for ipv6" ON/\
    #	      LWS_IPV6 "Compile with support for ipv6" OFF/'\
    #	    ${MAKE_DIR}/libwebsockets/CMakeLists.txt
    LWS_IPV6=OFF
fi

if [ -d ${MAKE_DIR}/libwebsockets/build ]; then
    rm -R build
fi

mkdir -p ${MAKE_DIR}/libwebsockets/build
cd ${MAKE_DIR}/libwebsockets/build

cmake .. -DCMAKE_INSTALL_PREFIX:PATH=${CHROOT_DIR}/usr/local \
       -DCMAKE_TOOLCHAIN_FILE=${MAKE_DIR}/libwebsockets/cross-arm-linux-gnueabihf.cmake \
       -DLWS_WITHOUT_EXTENSIONS=OFF -DLWS_WITH_SSL=ON \
       -DZLIB_INCLUDE_DIR=${CHROOT_DIR}/usr/include \
       -DZLIB_LIBRARY=${CHROOT_DIR}/lib/arm-linux-gnueabihf/libz.so.1 \
       -DOPENSSL_ROOT_DIR=${CHROOT_DIR}/usr \
       -DOPENSSL_INCLUDE_DIR=${CHROOT_DIR}/usr/include \
       -DOPENSSL_SSL_LIBRARY=${CHROOT_DIR}/usr/lib/arm-linux-gnueabihf/libssl.so \
       -DOPENSSL_CRYPTO_LIBRARY=${CHROOT_DIR}/usr/lib/arm-linux-gnueabihf/libcrypto.so \
       -DLWS_IPV6=$LWS_IPV6

make -j${CORES}
sudo make install -j${CORES}

print "libwebsockets installed"
}


# Compiled MongoDB for RaspberryPI in system debian wheenzy
compiled_and_install_mongodb_rpi() {
print "compiling and installing mongodb for rpi"
MONGO_INSTALL_DIR=/usr/local/mongo
if [ -f ${MONGO_INSTALL_DIR}/lib/libmongoclient.a ] && [ -f ${CHROOT_DIR}/etc/init.d/mongod ]; then
  print "mongodb is already installed in rootfs"
  return
fi

if [ ! -d ${CHROOT_DIR}/home/ld/code ]; then
 mkdir -p ${CHROOT_DIR}/home/ld/code
fi

if [ ! -d ${CHROOT_DIR}/home/ld/code/mongo-nonx86 ]; then
  if [ -f ${CHROOT_DIR}/../mongodb_wheezy.tar.gz ]; then
    print "Архив с исходниками и собранными бинарниками mongodb уже есть в наличии, распаковываем"
    sudo tar -xf ${CHROOT_DIR}/../mongodb_wheezy.tar.gz -C ${CHROOT_DIR}/home/ld/code
  else
    print "Скачиваем репозиторий с исходниками mongodb, собираем и устанавливаем, создаем архив"
    sudo chroot ${CHROOT_DIR} /bin/bash -c "cd /home/ld/code/; git clone https://github.com/skrabban/mongo-nonx86"
  fi
fi

print "Каталог с исходниками mongodb в наличии"
if [ ! -f ${MONGO_INSTALL_DIR}/lib/libmongoclient.a ]; then
    print "Cобираем и устанавливаем mongodb"
    sudo chroot ${CHROOT_DIR} /bin/bash -c "cd /home/ld/code/mongo-nonx86; scons --prefix=${MONGO_INSTALL_DIR} install -j${CORES}"
else
    print "mongodb уже установлен"
fi

if [ ! -f ${BOARD_DIR}/mongodb_wheezy.tar.gz ]; then
    sudo sh -c "cd ${CHROOT_DIR}/home/ld/code; tar -czf ${BOARD_DIR}/mongodb_wheezy.tar.gz ./mongo-nonx86"
    print "mongodb_wheezy.tar.gz created into ${BOARD_DIR}"
fi

print "preparatory operations to run mongodb"
# Delete previous settings
if grep -q "mongodb:" ${CHROOT_DIR}/etc/passwd; then
print "deluser"
    sudo chroot ${CHROOT_DIR} /bin/bash -c "deluser mongodb"
fi
if [ -f ${CHROOT_DIR}/etc/mongodb.conf ]; then
    sudo rm ${CHROOT_DIR}/etc/mongodb.conf
fi
if [ -f ${CHROOT_DIR}/etc/init.d/mongod ]; then
    sudo rm ${CHROOT_DIR}/etc/init.d/mongod
fi
if [ -L ${CHROOT_DIR}/usr/bin/mongod ]; then
    sudo rm ${CHROOT_DIR}/usr/bin/mongod
fi

# Permissions for users and groups mongodb
sudo chroot ${CHROOT_DIR} /bin/bash -c "adduser --firstuid 100 --ingroup nogroup --shell /etc/false --disabled-password --gecos '' --no-create-home mongodb"
# A folder for log files mongodb
sudo chroot ${CHROOT_DIR} /bin/bash -c "mkdir -p /var/log/mongodb/"
# Permissions for log file
sudo chroot ${CHROOT_DIR} /bin/bash -c "chown mongodb:nogroup /var/log/mongodb/"
# A folder for state data mongodb
sudo chroot ${CHROOT_DIR} /bin/bash -c "mkdir -p /var/lib/mongodb"
# Permissions for the folder
sudo chroot ${CHROOT_DIR} /bin/bash -c "chown mongodb:nogroup /var/lib/mongodb"
# Moving init.d script to etc
sudo cp ${CHROOT_DIR}/home/ld/code/mongo-nonx86/debian/init.d ${CHROOT_DIR}/etc/init.d/mongod
# Moving our config file to etc
sudo cp ${CHROOT_DIR}/home/ld/code/mongo-nonx86/debian/mongodb.conf ${CHROOT_DIR}/etc/
# Linking folders up
sudo chroot ${CHROOT_DIR} /bin/bash -c "ln -s ${MONGO_INSTALL_DIR}/bin/mongod /usr/bin/mongod"
print "minimize MongoDB journaling files"
echo 'smallfiles=true' | sudo tee -a ${CHROOT_DIR}/etc/mongodb.conf

sudo chroot ${CHROOT_DIR} /bin/bash -c "chmod u+x /etc/init.d/mongod"
sudo chroot ${CHROOT_DIR} /bin/bash -c "update-rc.d mongod defaults"


print "Remove the build directory of the rootfs mongodb"
sudo sh -c "rm -R ${CHROOT_DIR}/home/ld/code/mongo-nonx86"
}


compiled_and_install_mongodb() {

print "compiling and installing mongodb"

cd ${MAKE_DIR}/mongodb
scons --propagate-shell-environment --cc=${CC_DIR}/bin/arm-linux-gnueabihf-gcc --cxx=${CC_DIR}/bin/arm-linux-gnueabihf-g++ --full install --prefix=${MAKE_DIR} -j${CORES}

print "End compiling and installing mongodb"
}

# Download, configure and build mongo-c
compiled_and_install_mongoc() {
print "Start compiled_and_install_mongoc"

if [ -f ${CHROOT_DIR}/usr/local/include/libmongoc-1.0/mongoc.h ]; then
    print "mongo-c-driver-$NAME_MONGOC is already installed in rootfs"
    return
fi

if [ ! -d "${MAKE_DIR}/mongo-c-driver" ]; then
    if [ ! -f ${LDDOWNL_DIR}/mongo-c-driver-$NAME_MONGOC.tar.gz ]; then
      print "Download mongo-c-driver-$NAME_MONGOC.tar.gz"
      wget -c -P ${LDDOWNL_DIR} https://github.com/mongodb/mongo-c-driver/releases/download/$NAME_MONGOC/mongo-c-driver-$NAME_MONGOC.tar.gz
    fi
    cd ${MAKE_DIR}
    print "unpacking mongo-c-driver-$NAME_MONGOC.tar.gz intro ${MAKE_DIR}"
    tar -xzf ${LDDOWNL_DIR}/mongo-c-driver-$NAME_MONGOC.tar.gz
    mv mongo-c-driver* mongo-c-driver
fi

cd ${MAKE_DIR}/mongo-c-driver

#sudo env PATH=$PATH make clean
./configure CFLAGS="-march=${MARCH} -mfpu=${MFPU}" --host=arm-linux-gnueabihf --prefix=/usr/local
make -j${CORES}
sudo env PATH=$PATH make DESTDIR=${CHROOT_DIR} install -j${CORES}
print "mongo-c-driver-$NAME_MONGOC installed"
}


# Download, configure and build  kernel for Raspberry PI
build_kernel_xeno2_rpi() {

NAME_KERNEL=rpi-3.8.y
NAME_XENO=xenomai-2.6.4
KBUILD_DIR=${MAKE_DIR}/kernel/build
#NAME_XENO=xenomai-3.0-rc1

if  [ -f ${CHROOT_DIR}/boot/kernel.img ] && \
    [ -f ${CHROOT_DIR}/boot/bootcode.bin ] && \
    [ -f ${CHROOT_DIR}/usr/local/xenomai/include/xeno_config.h ] && \
    [ -d ${CHROOT_DIR}/opt/vc ]; then
    print "kernel and xenomai 2 for rpi is already installed in rootfs"
    return 0
fi
print "Start compiled_and_install_kernel_rpi"

# Download kernel
cd ${MAKE_DIR}
if [ ! -d "${MAKE_DIR}/kernel" ]; then
    if [ ! -f ${LDDOWNL_DIR}/${NAME_KERNEL}.tar.gz ]; then
        #wget -c -P ${LDDOWNL_DIR} https://github.com/raspberrypi/linux/archive/${NAME_KERNEL}.zip
        print "git clone ${NAME_KERNEL}"
        git clone -b ${NAME_KERNEL} --depth 1 git://github.com/raspberrypi/linux.git kernel
        print "end clone ${NAME_KERNEL}"
        print "create archive ${NAME_KERNEL}.tar.gz intro ${LDDOWNL_DIR}"
        tar -czf ${LDDOWNL_DIR}/${NAME_KERNEL}.tar.gz ./kernel
        print "end create archive ${NAME_KERNEL}.tar.gz"
    else
        print "unpacking ${NAME_KERNEL}.tar.gz intro ${MAKE_DIR}"
        tar -xf ${LDDOWNL_DIR}/${NAME_KERNEL}.tar.gz -C ${MAKE_DIR}
    fi
else
    print "git reset and clean for ${NAME_KERNEL}"
    cd ${MAKE_DIR}/kernel
    git reset --hard
    git clean -fdx
    git pull
    #cd ${MAKE_DIR}; tar -czf ${LDDOWNL_DIR}/${NAME_KERNEL}.tar.gz ./kernel
fi

# Download Xenomai
if [ ! -d "${MAKE_DIR}/xenomai" ]; then
    cd ${MAKE_DIR}
    if [ ! -f ${LDDOWNL_DIR}/${NAME_XENO}.tar.gz ]; then
      print "Download ${NAME_XENO}.tar.gz"
      #git clone git://git.xenomai.org/xenomai-head.git xenomai-head
      wget -c -P ${LDDOWNL_DIR} http://download.gna.org/xenomai/stable/latest/${NAME_XENO}.tar.bz2
      #http://download.gna.org/xenomai/testing/latest/xenomai-3.0-rc1.tar.bz2
    fi
    cd ${MAKE_DIR}
    print "unpacking ${NAME_XENO}.tar.bz2 intro ${MAKE_DIR}"
    tar -xjf ${LDDOWNL_DIR}/${NAME_XENO}.tar.bz2
    mv ${NAME_XENO}* xenomai
fi


# Download minimal config
if [ ! -f ${LDDOWNL_DIR}/rpi_xenomai_config ]; then
    print "Download rpi_xenomai_config"
    wget -c -P ${LDDOWNL_DIR} https://www.dropbox.com/s/dcju74md5sz45at/rpi_xenomai_config
fi

#1- Checkout the "rpi-3.8.y" branch in the repository [4], commit d996a1b
#2- Apply raspberry/ipipe-core-3.8.13-raspberry-pre-2.patch
#3- Apply ipipe-core-3.8.13-arm patch
#2- apply raspberry/ipipe-core-3.8.13-raspberry-post-2.patch
#3- you can resume to generic installation instructions.

print "Apply ipipe core pre-patch"
cd ${MAKE_DIR}/kernel
patch -Np1 < ${MAKE_DIR}/xenomai/ksrc/arch/arm/patches/raspberry/ipipe-core-3.8.13-raspberry-pre-2.patch
print "Apply Xenomai ipipe core patch"
${MAKE_DIR}/xenomai/scripts/prepare-kernel.sh --arch=arm --linux=${MAKE_DIR}/kernel --adeos=${MAKE_DIR}/xenomai/ksrc/arch/arm/patches/ipipe-core-3.8.13-arm-4.patch
print "Apply ipipe core post-patch"
cd ${MAKE_DIR}/kernel
patch -Np1 < ${MAKE_DIR}/xenomai/ksrc/arch/arm/patches/raspberry/ipipe-core-3.8.13-raspberry-post-2.patch

# Create build directory
mkdir -p ${KBUILD_DIR}
cd ${MAKE_DIR}/kernel

# minimal configuration file
cp ${LDDOWNL_DIR}/rpi_xenomai_config ${KBUILD_DIR}/.config

print "mrproper"
make mrproper

if [ $MENUCONFIG = YES ];then
    make ARCH=arm O=build menuconfig
else
    make ARCH=arm O=build oldconfig
fi

print "Compile kernel"
make ARCH=arm O=${KBUILD_DIR} CROSS_COMPILE=${CC_DIR}/bin/arm-linux-gnueabihf- -j${CORES}
# Install modules
make ARCH=arm O=${KBUILD_DIR} INSTALL_MOD_PATH=dist modules_install -j${CORES}
# Install headers
make ARCH=arm O=${KBUILD_DIR} INSTALL_HDR_PATH=dist headers_install -j${CORES}
find ${KBUILD_DIR}/dist/include \( -name .install -o -name ..install.cmd \) -delete

print "Copy Xenomai kernel modules and headers to rootfs"
sudo cp -a ${KBUILD_DIR}/dist/lib/modules ${CHROOT_DIR}/lib/
sudo cp -a ${KBUILD_DIR}/dist/include/* ${CHROOT_DIR}/usr/include
sudo cp ${KBUILD_DIR}/.config ${CHROOT_DIR}/boot/config-${NAME_KERNEL}-xenomai+
sudo cp ${KBUILD_DIR}/arch/arm/boot/Image ${CHROOT_DIR}/boot/kernel.img

print "Compile and install library, headers Xenomai to rootfs"
cd ${MAKE_DIR}/xenomai
./configure CFLAGS="-march=${MARCH} -mfpu=${MFPU}" LDFLAGS="-march=${MARCH}" --host=arm-linux-gnueabihf --prefix=/usr/local/xenomai
make -j${CORES}
sudo env PATH=$PATH make DESTDIR=${CHROOT_DIR} install -j${CORES}
sudo sh -c "echo /usr/local/xenomai/lib/ > ${CHROOT_DIR}/etc/ld.so.conf.d/xenomai.conf"

# Run ldconfig in chroot
run_cmd_chroot "ldconfig -v"

# Installing firmware rpi
if  [ ! -d ${CHROOT_DIR}/opt/vc ] || \
    [ ! -f ${CHROOT_DIR}/boot/bootcode.bin ]; then

    if [ ! -f ${LDDOWNL_DIR}/rpi-firmware.zip ]; then
        print "Download firmware for rpi"
        wget -O ${LDDOWNL_DIR}/rpi-firmware.zip https://github.com/raspberrypi/firmware/archive/master.zip
    fi

    print "Installing firmware rpi"
    cd ${MAKE_DIR}
    unzip -n ${LDDOWNL_DIR}/rpi-firmware.zip
    sudo cp -a -u  ${MAKE_DIR}/firmware-master/hardfp/opt/vc ${CHROOT_DIR}/opt/
    sudo cp -u ${MAKE_DIR}/firmware-master/boot/*bin ${CHROOT_DIR}/boot/
    sudo cp -u ${MAKE_DIR}/firmware-master/boot/*dat ${CHROOT_DIR}/boot/
    sudo cp -u ${MAKE_DIR}/firmware-master/boot/*elf ${CHROOT_DIR}/boot/
else
    print "firmware is already installed for rpi"
fi

print "End build_kernel_xeno2_rpi"
}

compiled_and_install_nodejs_npm_rpi() {
print "Start compiled_and_install_nodejs for rpi"

if  [ -f ${CHROOT_DIR}/usr/local/include/nodejs/node.h ] || \
    [ -f ${CHROOT_DIR}/usr/include/nodejs/node.h ]  || \
    [ -f ${CHROOT_DIR}/usr/local/include/node/node.h ] || \
    [ -f ${CHROOT_DIR}/usr/include/node/node.h ]; then
    print "NodeJS is already installed in rootfs"
    return
fi

if [ ! -d "${MAKE_DIR}/nodejs" ]; then
    if [ ! -f ${LDDOWNL_DIR}/${BOARD}-nodejs.zip ]; then
      print "Download NodeJS src"
      wget -c -O ${LDDOWNL_DIR}/${BOARD}-nodejs.zip https://github.com/joyent/node/archive/v0.10.32-release.zip
    fi
    cd ${MAKE_DIR}
    print "unpacking ${BOARD}-nodejs.zip intro ${MAKE_DIR}"
    unzip -n ${LDDOWNL_DIR}/${BOARD}-nodejs.zip
    mv node* nodejs
fi

cd ${MAKE_DIR}/nodejs

export HOST="arm-linux-gnueabihf"
export CPP="${HOST}-gcc -E"
export STRIP="${HOST}-strip"
export OBJCOPY="${HOST}-objcopy"
export AR="${HOST}-ar"
export RANLIB="${HOST}-ranlib"
export LD="${HOST}-ld"
export OBJDUMP="${HOST}-objdump"
export CC="${HOST}-gcc"
export CXX="${HOST}-g++"
export NM="${HOST}-nm"
export AS="${HOST}-as"
#export PS1="[${HOST}] \w$ "
#bash --norc

print "make clean"
make clean
./configure --without-snapshot --dest-cpu=arm --dest-os=linux --prefix=/usr/local
make -j${CORES} PORTABLE=1
sudo env PATH=$PATH make DESTDIR=${CHROOT_DIR} install -j${CORES} PORTABLE=1

#sudo cp -R ${MAKE_DIR}/nodejs ${CHROOT_DIR}/root/
#run_cmd_chroot "cd /root/nodejs; ./configure --prefix=/usr/local && make -j${CORES} && sudo make install -j${CORES}"

#npm config set registry http://registry.npmjs.org/
#npm config set proxy http://my-proxy.com:1080
#npm config set https-proxy http://my-proxy.com:1080
#npm config list | grep registry
#npm config set strict-ssl false
#npm install -g supervisor


print "End  compiled_and_install_nodejs for rpi"
}

# Create rootfs for BeagleBone Black use debootstrap
debootstrap_bbb() {
print "Start debootstrap for BeagleBone Black"

# Create a directory where all the stuff goes
if [ ! -d ${MAKE_DIR} ]; then
    mkdir -p ${MAKE_DIR}
fi

if [ ! -d ${CHROOT_DIR} ]; then
    debootstrap --arch=armhf --include=ssh --foreign $DISTR ${CHROOT_DIR} $DISTR_MIRROR
fi

cp /usr/bin/qemu-arm-static ${CHROOT_DIR}/usr/bin/

if [ -d ${CHROOT_DIR}/debootstrap ]; then
    chroot ${CHROOT_DIR} /debootstrap/debootstrap --second-stage
    mount proc ${CHROOT_DIR}/proc -t proc
    mount sysfs ${CHROOT_DIR}/sys -t sysfs
    mount devpts ${CHROOT_DIR}/dev/pts -t devpts
    print "Adding deb sources to /etc/apt/sources.list"
    chroot ${CHROOT_DIR} /bin/bash -c "echo 'deb $DISTR_MIRROR $DISTR main universe multiverse' >> /etc/apt/sources.list"
    chroot ${CHROOT_DIR} /bin/bash -c "echo 'deb-src $DISTR_MIRROR $DISTR main universe multiverse' >> /etc/apt/sources.list"
    chroot ${CHROOT_DIR} /bin/bash -c "echo 'deb $DISTR_MIRROR $DISTR-updates main universe multiverse' >> /etc/apt/sources.list"
    chroot ${CHROOT_DIR} /bin/bash -c "echo 'deb-src $DISTR_MIRROR $DISTR-updates main universe multiverse' >> /etc/apt/sources.list"
    print "Setting hostname"
    chroot ${CHROOT_DIR} /bin/bash -c "echo 'linuxdrone' > /etc/hostname"
    chroot ${CHROOT_DIR} /bin/bash -c "echo -e '127.0.0.1\tlinuxdrone' >> /etc/hosts"
    print "Updating apt sources and installing packages"
    chroot ${CHROOT_DIR} /bin/bash -c "apt-get update"
    chroot ${CHROOT_DIR} /bin/bash -c "apt-get -y install mc wget git screen build-essential cpufrequtils i2c-tools usbutils wpasupplicant wireless-tools linux-firmware gdbserver"
    chroot ${CHROOT_DIR} /bin/bash -c "apt-get -y install mongodb nodejs npm libboost-system.dev libboost-filesystem.dev libboost-thread.dev libboost-program-options.dev"
    chroot ${CHROOT_DIR} /bin/bash -c "apt-get -y install htop"
    chroot ${CHROOT_DIR} /bin/bash -c "apt-get -y install libwebsockets-dev libwebsockets-test-server libwebsockets3 libwebsockets3-dbg"
    chroot ${CHROOT_DIR} /bin/bash -c "apt-get -y remove ntpdate"
    #chroot ${CHROOT_DIR} /bin/bash -c "apt-get -y upgrade"
    chroot ${CHROOT_DIR} /bin/bash -c "apt-get clean"
    chroot ${CHROOT_DIR} /bin/bash -c "apt-get autoremove"
    print "Create uboot folder"
    chroot ${CHROOT_DIR} /bin/bash -c "mkdir -p /boot/uboot"
    print "Create user ubuntu"
    chroot ${CHROOT_DIR} /bin/bash -c "adduser ld"
    chroot ${CHROOT_DIR} /bin/bash -c "usermod -a -G sudo ld"
    print "Set root password"
    chroot ${CHROOT_DIR} /bin/bash -c "passwd"

    chroot ${CHROOT_DIR} /bin/bash -c "locale-gen ${LOCALES}"
    chroot ${CHROOT_DIR} /bin/bash -c "dpkg-reconfigure locales"

    umount ${CHROOT_DIR}/proc
    umount ${CHROOT_DIR}/sys
    umount ${CHROOT_DIR}/dev/pts
fi

mount proc ${CHROOT_DIR}/proc -t proc
mount sysfs ${CHROOT_DIR}/sys -t sysfs
mount devpts ${CHROOT_DIR}/dev/pts -t devpts
print "Starting chroot bash session, type exit to end"
chroot ${CHROOT_DIR} /bin/bash

chroot ${CHROOT_DIR} /bin/bash -c "apt-get clean"
chroot ${CHROOT_DIR} /bin/bash -c "apt-get autoremove"

umount ${CHROOT_DIR}/proc
umount ${CHROOT_DIR}/sys
umount ${CHROOT_DIR}/dev/pts

rm ${CHROOT_DIR}/usr/bin/qemu-arm-static
if [ -f ${MAKE_DIR}/rootfs.tar.gz ]; then
    rm ${MAKE_DIR}/rootfs.tar.gz
fi
cd ${CHROOT_DIR}; tar -czvf ${MAKE_DIR}/rootfs.tar.gz .
print "rootfs.tar.gz created into ${MAKE_DIR}"
return 0
}

# Create rootfs for Raspberry PI use debootstrap
debootstrap_rpi() {
print "Start debootstrap for Raspberry Pi"

cd ${BOARD_DIR}

# Checking directory rootfs if it's not done unpacking archives with rootfs
if [ ! -d ${CHROOT_DIR} ]; then
    if [ -f ${BOARD_DIR}/rootfs-${BOARD}-${DISTR}-full.tar.gz ]; then
        print "unpacking rootfs-${BOARD}-${DISTR}-full.tar.gz"
        sudo tar -xzf ${BOARD_DIR}/rootfs-${BOARD}-${DISTR}-full.tar.gz
        return 0
    elif [ -f ${BOARD_DIR}/rootfs-${BOARD}-${DISTR}-mini.tar.gz ]; then
        print "unpacking rootfs-${BOARD}-${DISTR}-mini.tar.gz"
        sudo tar -xzf ${BOARD_DIR}/rootfs-${BOARD}-${DISTR}-mini.tar.gz
        return 0
    fi

    # Debootstrap step 1
    print "Debootstrap step 1"
    sudo debootstrap --arch=armhf --include=ssh --foreign --no-check-gpg --include=ca-certificates $DISTR ${CHROOT_DIR} $DISTR_MIRROR
fi

qemu_copy_to_roofs

# Debootstrap step 2
if [ -d ${CHROOT_DIR}/debootstrap ]; then
    print "Debootstrap step 2"
    sudo chroot ${CHROOT_DIR} /debootstrap/debootstrap --second-stage  --verbose
    mount_proc_sysfs_devpts
    print "Adding deb sources to /etc/apt/sources.list"
    sudo sh -c "echo 'deb $DISTR_MIRROR $DISTR main contrib non-free' >> ${CHROOT_DIR}/etc/apt/sources.list"
    sudo sh -c "echo 'deb-src $DISTR_MIRROR $DISTR main contrib non-free' >> ${CHROOT_DIR}/etc/apt/sources.list"

    rootfs_common_settings

    print "Edit /etc/fstab"
    sudo sh -c "cat> ${CHROOT_DIR}/etc/fstab << EOF
proc /proc proc defaults 0 0
/dev/mmcblk0p1 /boot vfat defaults 0 0
EOF
"

    # Create config.txt in rootfs/boot
    sudo sh -c "cat >${CHROOT_DIR}/boot/config.txt<<EOF
kernel=kernel.img
arm_freq=800
core_freq=250
sdram_freq=400
over_voltage=0
gpu_mem=16
EOF
"
    # Create cmdline.txt in rootfs/boot
    sudo sh -c "cat >${CHROOT_DIR}/boot/cmdline.txt<<EOF
dwc_otg.lpm_enable=0 root=/dev/mmcblk0p2 rootfstype=ext4 rootwait
EOF
"
    rootfs_installing_packages
    umount_proc_sysfs_devpts
fi

qemu_remove_from_roofs

if [ ! -f ${BOARD_DIR}/rootfs-${BOARD}-${DISTR}-mini.tar.gz ]; then
    cd ${BOARD_DIR}
    print "Start created archive rootfs-${BOARD}-${DISTR}-mini.tar.gz"
    sudo tar -czvf ${BOARD_DIR}/rootfs-${BOARD}-${DISTR}-mini.tar.gz ./rootfs
    print "End created archive rootfs-${BOARD}-${DISTR}-mini.tar.gz into ${BOARD_DIR}"
fi

return 0
}

# Build and install software LinuxDrone to rootfs
build_install_linuxdrone() {
print "Start build and install software LinuxDrone to rootfs"

INSTALL_DIR=${LDROOT_DIR}/build.Debug/install

# Clear old build LinuxDrone
if [ -d ${LDROOT_DIR}/build.Debug ]; then
    rm -R ${LDROOT_DIR}/build.Debug
fi

# Create project run cmake
cd ${LDROOT_DIR}
./configure.sh
cd ${LDROOT_DIR}/build.Debug
make -j${CORES}
make install -j${CORES}

# build Web Configurator
cd ${LDROOT_DIR}/webapps/configurator/public
sencha app build

rsync -zvr --delete --exclude="node_modules/*" --exclude="configurator/public/*" \
        "${LDROOT_DIR}/webapps/" "${INSTALL_DIR}/usr/local/linuxdrone/webapps/"


cp -R   ${LDROOT_DIR}/webapps/configurator/public/build/production/RtConfigurator/* \
        ${INSTALL_DIR}/usr/local/linuxdrone/webapps/configurator/public/

cp -R   ${LDROOT_DIR}/webapps/configurator/public/images \
        ${INSTALL_DIR}/usr/local/linuxdrone/webapps/configurator/public/

cp -R   ${LDROOT_DIR}/webapps/configurator/public/javascripts \
        ${INSTALL_DIR}/usr/local/linuxdrone/webapps/configurator/public/

cp -R   ${LDROOT_DIR}/webapps/configurator/public/favicon.ico \
        ${INSTALL_DIR}/usr/local/linuxdrone/webapps/configurator/public/favicon.ico

cp -R   ${LDROOT_DIR}/webapps/configurator/public/ModulesCommonParams.def.js \
        ${INSTALL_DIR}/usr/local/linuxdrone/webapps/configurator/public/ModulesCommonParams.def.js

cd ${INSTALL_DIR}/usr/local/linuxdrone/webapps/configurator/
npm install

sudo rsync -zvr --delete --exclude="/proc" "${INSTALL_DIR}/usr/local/linuxdrone/" "${CHROOT_DIR}/usr/local/linuxdrone/"

#sudo rsync -zvr --delete --exclude="/proc" --exclude="node_modules/*" \
#        "${INSTALL_DIR}/usr/local/linuxdrone/" "${CHROOT_DIR}/usr/local/linuxdrone/"

#run_cmd_chroot "cd /usr/local/linuxdrone/webapps/configurator; npm install"

# Run ldconfig for library LinuxDrone
sudo touch ${CHROOT_DIR}/etc/ld.so.conf.d/linuxdrone.conf
echo '# LinuxDrone' | sudo tee ${CHROOT_DIR}/etc/ld.so.conf.d/linuxdrone.conf
# Search directories with libraries linuxdrone
dirname `find ${INSTALL_DIR}/usr/local/linuxdrone/ -iname *so` | uniq | sed s:${INSTALL_DIR}::g | \
        sudo tee -a "${CHROOT_DIR}/etc/ld.so.conf.d/linuxdrone.conf"

run_cmd_chroot "ldconfig -v"

print "End build and install software LinuxDrone to rootfs"
}

install_software_the_host() {


# Installing SenchaCmd
if  [ ! -d ${CHROOT_DIR}/opt/vc ] || \
    [ ! -f ${CHROOT_DIR}/boot/bootcode.bin ]; then

    if [ ! -f ${LDDOWNL_DIR}/SenchaCmd.zip ]; then
        print "Download SenchaCmd"
        wget -O ${LDDOWNL_DIR}/SenchaCmd.zip http://cdn.sencha.com/cmd/5.0.3.324/SenchaCmd-5.0.3.324-linux-x64.run.zip
    fi

sencha upgrade --check
sencha upgrade

    print "Installing firmware rpi"
    cd ${MAKE_DIR}
    unzip -n ${LDDOWNL_DIR}/rpi-firmware.zip
    sudo cp -a -u  ${MAKE_DIR}/firmware-master/hardfp/opt/vc ${CHROOT_DIR}/opt/
    sudo cp -u ${MAKE_DIR}/firmware-master/boot/*bin ${CHROOT_DIR}/boot/
    sudo cp -u ${MAKE_DIR}/firmware-master/boot/*dat ${CHROOT_DIR}/boot/
    sudo cp -u ${MAKE_DIR}/firmware-master/boot/*elf ${CHROOT_DIR}/boot/
else
    print "firmware is already installed for rpi"
fi


}

#------------------------------------------------
#                Start script
#------------------------------------------------

START_SHELL=NO
DISK_IMAGE_CLEAN=NO

# Разбор параметров командной строки
while getopts hvmsb:d:c: OPT; do
    case "$OPT" in
        h)
            echo $USAGE
            exit 0
            ;;
        v)
            echo "`basename $0` version 0.1"
            exit 0
            ;;
        m)
            MENUCONFIG=YES
            ;;
        s)
            START_SHELL=YES
            ;;
        b)
            BOARD=$OPTARG
            ;;
        d)
            DEV_DISK=$OPTARG
            ;;
        c)
            CLEAN=$OPTARG
            ;;
        \?)
            # getopts вернул ошибку
            echo ${USAGE}
            exit 1
            ;;
    esac
done

# Удаляем обработанные выше параметры
shift `expr $OPTIND - 1`

echo ${CLEAN}

if [ ! ${BOARD} ]; then
    echo "Not specified board type"
    echo $USAGE
    exit 1
fi

#{CHROOT_DIR}=`pwd`/tools/board/${BOARD}/rootfs
#{CHROOT_DIR}=$(readlink -f $(readlink -f "$(dirname "${CHROOT_DIR}")")/$(basename "${CHROOT_DIR}"))
LDROOT_DIR=`pwd`

echo '' | tee ${LDROOT_DIR}/build_rootfs.log
print "Start scripts create rootfs"

LDDOWNL_DIR=${LDROOT_DIR}/downloads

BOARD_DIR=${LDROOT_DIR}/tools/board/${BOARD}
# Getting the full path if you had used a symbolic link to the folder
BOARD_DIR=$(readlink -f $(readlink -f "$(dirname "${BOARD_DIR}")")/$(basename "${BOARD_DIR}"))
CHROOT_DIR=${BOARD_DIR}/rootfs
# The path to the cross compiler
CC_DIR=${BOARD_DIR}/cc
MAKE_DIR=${BOARD_DIR}/make_dir
LOCALES=${LANG}
CORES=$(grep "^cpu cores" /proc/cpuinfo | awk -F : '{print $2}' | head -1 | sed 's/^[ ]*//')
CORES=$((${CORES} + 1))

DISK=`pwd`/tools/board/bbb/rootfs.img
DISK_SIZE=2048


sudo_timestamp_timeout 800

if [ ! -d ${LDDOWNL_DIR} ]; then
    print "Create a directory where all downloads soft"
    mkdir -p ${LDDOWNL_DIR}
fi

if [ ! -d ${MAKE_DIR} ]; then
    print "Create a directory for build packages"
    mkdir -p ${MAKE_DIR}
fi



case "${BOARD}" in
    bbb)
	MARCH=armv7-a
	MFPU=vfp3
        DISTR=trusty
        #DISTR=saucy
        DISTR_MIRROR=http://ports.ubuntu.com/ubuntu-ports/

	download_crosscompiler
        debootstrap_bbb
        ;;

    rpi)
        MARCH=armv6 #j
	MFPU=vfp
	MCPU=arm1176jzf-s
	MFLOAT_ABI=hard
	NAME_LIBWEB=libwebsockets-1.3-chrome37-firefox30.tar.gz
        NAME_MONGOC=0.94.2
        DISTR=wheezy
        DISTR_MIRROR=http://archive.raspbian.org/raspbian/
        #DISTR_MIRROR=http://mirrors-ru.go-parts.com/raspbian/
        #DISTR_MIRROR=http://mirror.netcologne.de/raspbian/raspbian/
        #DISTR=jessie
        #DISTR_MIRROR=http://mirrordirector.raspbian.org/raspbian

	download_install_crosscompiler_rpi
        debootstrap_rpi

        if [ ${START_SHELL} = YES ]; then
            start_shell_in_chroot
        fi

        compiled_and_install_nodejs_npm_rpi
        compiled_and_install_libwebsockets
        compiled_and_install_mongoc
        build_kernel_xeno2_rpi
        build_install_linuxdrone
        ;;

    *)
        print "Unknown name board"
        echo ${USAGE}
        exit 1
        ;;
esac

create_image
umount_proc_sysfs_devpts
sudo_timestamp_timeout 10

exit 0
