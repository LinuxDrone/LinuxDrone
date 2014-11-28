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

# Printing and logging message
print() {
#2>&1
date +%d-%m-%Y\ %H:%M:%S | tr -d '\012' | tee -a ${LDROOT_DIR}/build_rootfs.log
echo " --- $1" | tee -a ${LDROOT_DIR}/build_rootfs.log

#date +%d-%m-%Y\ %H:%M:%S | tr -d '\012'; echo " --- $1";
}

# Set timestamp_timeout for sudoers
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
  #print "Umount file systems proc, sysfs, devpts"
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
#print "Mount file systems proc, sysfs, devpts"
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

# Launch console chroot
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

# Check and installed packages use apt-get
check_and_installed_packages() {
unset PKG_FOR_INST
unset PKG_FIND

DPKG_LIST=$(dpkg --list)

PKG_LIST=$(echo $1 | tr " " "\n")
for x in ${PKG_LIST}
do
    unset PKG_FIND
    PKG_FIND=$( echo ${DPKG_LIST} | grep -o " $x " | uniq)
    if [ ! ${PKG_FIND} ]; then
        PKG_FOR_INST="${PKG_FOR_INST} ${x}"
    fi
done

if [ "${PKG_FOR_INST}" ]; then
    print "-- Install packages for this host: ${PKG_FOR_INST}"
    sudo apt-get -y install "${PKG_FOR_INST}"
fi

unset PKG_FOR_INST
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
  print "Umount partitions /dev/mapper/loop*"
  sudo umount /dev/mapper/loop*
  print "umount /dev/mapper/loop*"
fi

# Удаляем устройства-разделы с помощью kpartx
for x in $(losetup -a | awk '{print $3}' | tr -d \(\))
do
    print "Remove device-partitions kpartx in file: ${x}"
    sudo kpartx -dv ${x}
done

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

print "create arhive the image sdcard ${DISK_IMAGE}.bz2"
cd ${BOARD_DIR}
if [ -f ${DISK_IMAGE}.bz2 ]; then
    sudo rm ${DISK_IMAGE}.bz2
fi
pbzip2 -k -9 -p${CORES} ${DISK_IMAGE}

lsblk
print "To copy a disk image on a flash card, enter one of these commands, edit /dev/sdX"
print "sudo  sh -c \"bzcat ${DISK_IMAGE}.bz2 > /dev/sdX\""
print "sudo dd if=${DISK_IMAGE} of=/dev/sdx bs=1M"
#sudo sh -c "bzcat ${DISK_IMAGE}.bz2 > /dev/${SDCARD}"
#sudo dd if=${DISK_IMAGE} of=/dev/${SDCARD} bs=1M

print "End create image sdcard"
}


# Common settings for rootfs
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

# Ethernet/RNDIS gadget (g_ether)
# ... or on host side, usbnet and random hwaddr
iface usb0 inet static
    address 192.168.7.2
    netmask 255.255.255.0
    network 192.168.7.0
    gateway 192.168.7.1
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
run_cmd_chroot "addgroup xenomai"
run_cmd_chroot "addgroup root xenomai"
print "Create user linuxdrone"
run_cmd_chroot "useradd -m ld -c LinuxDrone,,,,"
run_cmd_chroot "usermod -a -G xenomai,sudo,staff,kmem,plugdev ld"
run_cmd_chroot "cat << EOF | passwd ld
1
1
EOF
"

print "Choice of the shell for the user ld"
run_cmd_chroot "chsh -s /bin/bash ld"

print "Set root password"
run_cmd_chroot "cat << EOF | passwd
1
1
EOF
"

print "Configure udev rules"
sudo sh -c "cat >${CHROOT_DIR}/etc/udev/rules.d/xenomai.rules<<EOF
# allow RW access to /dev/mem
KERNEL==\"mem\", MODE=\"0660\", GROUP=\"kmem\"
# real-time heap device (Xenomai:rtheap)
KERNEL==\"rtheap\", MODE=\"0660\", GROUP=\"xenomai\"
# real-time pipe devices (Xenomai:rtpipe)
KERNEL==\"rtp[0-9]*\", MODE=\"0660\", GROUP=\"xenomai\"
EOF
"
}

# Settings BeagleBone Black for the rootfs
roofs_settings_bbb() {
print "Apply settings for ${BOARD_FULL_NAME}"
print "Adding deb sources to /etc/apt/sources.list"
sudo sh -c "echo 'deb ${DISTR_MIRROR} ${DISTR} main universe multiverse' >> ${CHROOT_DIR}/etc/apt/sources.list"
sudo sh -c "echo 'deb-src ${DISTR_MIRROR} ${DISTR} main universe multiverse' >> ${CHROOT_DIR}/etc/apt/sources.list"
sudo sh -c "echo 'deb ${DISTR_MIRROR} ${DISTR}-updates main universe multiverse' >> ${CHROOT_DIR}/etc/apt/sources.list"
sudo sh -c "echo 'deb-src ${DISTR_MIRROR} ${DISTR}-updates main universe multiverse' >> ${CHROOT_DIR}/etc/apt/sources.list"

rootfs_common_settings

print "Edit /etc/fstab"
sudo sh -c "cat> ${CHROOT_DIR}/etc/fstab << EOF
#/dev/mmcblk0p1  /  auto  errors=remount-ro  0  1
/dev/mmcblk0p2 /  auto  errors=remount-ro  0  1
/dev/mmcblk0p1 /boot/uboot  auto  defaults  0  2
EOF
"

print "Create 70-persistent-net.rules"
sudo sh -c "cat >${CHROOT_DIR}/etc/udev/rules.d/70-persistent-net.rules << EOF
'# BeagleBone: net device ()
SUBSYSTEM==\"net\", ACTION==\"add\", DRIVERS==\"?*\", ATTR{dev_id}==\"0x0\", ATTR{type}==\"1\", KERNEL==\"eth*\", NAME=\"eth0\"
EOF
"
print "Create /etc/init/serial.conf"
sudo sh -c "cat >${CHROOT_DIR}/etc/init/serial.conf << EOF
start on stopped rc RUNLEVEL=[2345]
stop on runlevel [!2345]

respawn
exec /sbin/getty 115200 ttyO0
EOF
"

print "Create /boot/uEnv.txt"
sudo sh -c "cat >${CHROOT_DIR}/boot/uEnv.txt << EOF
##This will work with: Angstrom's 2013.06.20 u-boot.
unsme_r=3.8.14

loadaddr=0x82000000
fdtaddr=0x88000000
rdaddr=0x88080000

initrd_high=0xffffffff
fdt_high=0xffffffff

# Enable I2C1 bus? disable HDMI/eMMC
optargs=quiet capemgr.enable_partno=BB-I2C1-400, capemgr.disable_partno=BB-BONELT-HDMI,BB-BONELT-HDMIN

loadximage=load mmc 0:1 \${loadaddr} /boot/vmlinuz-\${uname_r}
loadxfdt=load mmc 0:1 \${fdtaddr} /boot/dtbs/\${uname_r}/\${fdtfile}
loadxrd=load mmc 0:1 \${rdaddr} /boot/initrd.img-\${uname_r}; setenv rdsize \${filesize}
loaduEnvtxt=load mmc 0:1 \${loadaddr} /boot/uEnv.txt ; env import -t \${loadaddr} \${filesize};
loadall=run loaduEnvtxt; run loadximage; run loadxfdt;

mmcargs=setenv bootargs console=tty0 console=\${console} \${optargs} \${cape_disable} \${cape_enable} root=\${mmcroot} rootfstype=\${mmcrootfstype} \${cmdline}

uenvcmd=run loadall; run mmcargs; bootz \${loadaddr} - \${fdtaddr};
EOF
"
}

# Settings Raspberry Pi for the rootfs
roofs_settings_rpi() {
print "Apply settings for ${BOARD_FULL_NAME}"
print "Adding deb sources to /etc/apt/sources.list"
sudo sh -c "echo 'deb ${DISTR_MIRROR} ${DISTR} main contrib non-free' >> ${CHROOT_DIR}/etc/apt/sources.list"
sudo sh -c "echo 'deb-src ${DISTR_MIRROR} ${DISTR} main contrib non-free' >> ${CHROOT_DIR}/etc/apt/sources.list"

rootfs_common_settings

print "Edit /etc/fstab"
sudo sh -c "cat> ${CHROOT_DIR}/etc/fstab << EOF
proc /proc proc defaults 0 0
/dev/mmcblk0p1 /boot vfat defaults 0 0
EOF
"

print "Create config.txt in rootfs/boot"
sudo sh -c "cat >${CHROOT_DIR}/boot/config.txt<<EOF
kernel=kernel.img
arm_freq=800
core_freq=250
sdram_freq=400
over_voltage=0
gpu_mem=16
EOF
"
print "Create cmdline.txt in rootfs/boot"
sudo sh -c "cat >${CHROOT_DIR}/boot/cmdline.txt<<EOF
dwc_otg.lpm_enable=0 root=/dev/mmcblk0p2 rootfstype=ext4 rootwait
EOF
"
}

# Installing packages in rootfs
rootfs_installing_packages() {
print "Updating and upgrade installing packages"

PKG_LIST_COMMON="ssh sudo mc wget git screen build-essential cpufrequtils i2c-tools usbutils \
wpasupplicant wireless-tools gdbserver \
libboost-system.dev libboost-filesystem.dev libboost-thread.dev libboost-program-options.dev \
scons htop ntpdate libssl-dev udhcpd"

PKG_LIST_LIBWEB="libwebsockets-dev libwebsockets-test-server libwebsockets3 libwebsockets3-dbg"

run_cmd_chroot "apt-get update"
if  [ $DISTR = wheezy ] || \
    [ $DISTR = jessie ]; then
    print "Set locales for Debian systems"
    run_cmd_chroot "apt-get -y install locales locales-all"
    run_cmd_chroot "locale-gen ${LOCALES}"
    run_cmd_chroot "dpkg-reconfigure locales-all"
elif [ $DISTR = saucy ] || \
    [ $DISTR = trusty ]; then
    print "Set locales for Ubuntu systems"
    run_cmd_chroot "apt-get -y install locales"
    run_cmd_chroot "locale-gen ${LOCALES}"
    run_cmd_chroot "update-locale LANG=${LOCALES}"
fi
run_cmd_chroot "apt-get -y upgrade"

run_cmd_chroot "apt-get -y install ${PKG_LIST_COMMON} ${PKG_LIST_BOARD}"
run_cmd_chroot "apt-get clean; apt-get autoremove"
}


# Install common softwares to this host and project LinuxDrone
install_common_software_the_host() {
print "Start install softwares to this host"

PKG_LIST="libc6:i386 libstdc++6:i386 libncurses5:i386 zlib1g:i386 \
            device-tree-compiler lzma lzop u-boot-tools debootstrap \
            git qemu-user-static cmake vim ruby nodejs npm pbzip2 pigz"

SENCHA_CMD_VER="5.0.3.324"

check_and_installed_packages "${PKG_LIST}"

# Downloads and build and install Device Tree Compiler
DTC_VER=$(dtc -v | grep "DTC 1.4.0-gf345d9e4" || true)
if [ "x${DTC_VER}" = "x" ]; then
    cd ${LDDOWNL_DIR}
    print "Downloads and build and install Device Tree Compiler"
    wget -cN https://raw.github.com/RobertCNelson/tools/master/pkgs/dtc.sh
    chmod +x dtc.sh
    ./dtc.sh
fi

# Installing SenchaCmd
#CHECK_SENCHA=$(sencha | grep -o "Sencha Cmd" | uniq)
if  [ ! -f "${LDTOOLS_DIR}/Sencha/Cmd/${SENCHA_CMD_VER}/sencha" ]; then
    if [ ! -f ${LDDOWNL_DIR}/SenchaCmd-${SENCHA_CMD_VER}-linux-x64.run.zip ]; then
        print "Download SenchaCmd"
        wget -P ${LDDOWNL_DIR} http://cdn.sencha.com/cmd/${SENCHA_CMD_VER}/SenchaCmd-${SENCHA_CMD_VER}-linux-x64.run.zip
    fi
    cd ${LDDOWNL_DIR}
    print "unpacking SenchaCmd-${SENCHA_CMD_VER}-linux-x64.run.zip intro ${LDDOWNL_DIR}"
    unzip -n ${LDDOWNL_DIR}/SenchaCmd-${SENCHA_CMD_VER}-linux-x64.run.zip
    #mv $(ls | grep "SenchaCmd-.*.run") SenchaCmd.run
    chmod +x ${LDDOWNL_DIR}/SenchaCmd-${SENCHA_CMD_VER}-linux-x64.run
    ${LDDOWNL_DIR}/SenchaCmd-${SENCHA_CMD_VER}-linux-x64.run --prefix ${LDTOOLS_DIR} --mode unattended
fi

# Installing GPL version Ext JS
if [ ! -d "${LDTOOLS_DIR}/extjs" ]; then
    if [ ! -f ${LDDOWNL_DIR}/extjs-gpl.zip ]; then
      print "Download extJS-gpl"
      wget -c -O ${LDDOWNL_DIR}/extjs-gpl.zip http://cdn.sencha.com/ext/gpl/ext-5.0.1-gpl.zip
    fi
    cd ${LDTOOLS_DIR}
    print "unpacking extjs-gpl.zip intro ${LDTOOLS_DIR}"
    unzip -n ${LDDOWNL_DIR}/extjs-gpl.zip
    mv ext-* extjs

    cd ${LDROOT_DIR}/webapps/configurator/public
    ${LDTOOLS_DIR}/Sencha/Cmd/${SENCHA_CMD_VER}/sencha app upgrade  ${LDTOOLS_DIR}/extjs
fi

print "End install softwares to this host"
}


# Download cross-compiler
download_install_crosscompiler_rpi() {
if [ ! -f ${CC_DIR}/bin/arm-linux-gnueabihf-gcc ]; then
  if [ ! -d ${CC_DIR} ]; then
      print "Create a directory for cross-compiler"
      mkdir -p ${CC_DIR}
  elif [ $(find ${CC_DIR} -type f | wc -l) -ne 0 ]; then
      print "Clean ${CC_DIR}"
      rm -R ${CC_DIR}/*
  fi

  if [ ! -f ${LDDOWNL_DIR}/${BOARD}/crosscompiler.tar.gz ]; then
    print "Download cross-compiler rpi"
    wget -O ${LDDOWNL_DIR}/${BOARD}/crosscompiler.tar.gz https://github.com/raspberrypi/tools/archive/master.tar.gz
  fi

  print "Installing cross-compiler rpi"
  cd ${CC_DIR}
  tar -xzf ${LDDOWNL_DIR}/${BOARD}/crosscompiler.tar.gz tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64
  mv ./tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/* .
  rm -R ./tools-master
else
  print "Cross-compiler is already installed for rpi"
fi

export CC=${CC_DIR}/bin/arm-linux-gnueabihf-
export PATH=${CC_DIR}/bin:$PATH
}

download_install_crosscompiler_bbb() {
if [ ! -f ${CC_DIR}/bin/arm-linux-gnueabihf-gcc ]; then
  if [ ! -d ${CC_DIR} ]; then
      print "Create a directory for cross-compiler"
      mkdir -p ${CC_DIR}
  elif [ $(find ${CC_DIR} -type f | wc -l) -ne 0 ]; then
      print "Clean ${CC_DIR}"
      rm -R ${CC_DIR}/*
  fi

  if [ ! -f ${LDDOWNL_DIR}/${BOARD}/crosscompiler.tar.gz ]; then
    print "Download cross-compiler bbb"
    wget -O ${LDDOWNL_DIR}/${BOARD}/crosscompiler.tar.gz https://releases.linaro.org/14.09/components/toolchain/binaries/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux.tar.xz
  fi

  print "Installing cross-compiler bbb"
  cd ${CC_DIR}
  tar -xf ${LDDOWNL_DIR}/${BOARD}/crosscompiler.tar.gz
  mv ./*-arm-linux-gnueabihf*/* .
  rm -R ./*-arm-linux-gnueabihf*
else
  print "Cross-compiler is already installed for bbb"
fi

export CC=${CC_DIR}/bin/arm-linux-gnueabihf-
export PATH=${CC_DIR}/bin:$PATH
}


# Download, patch, configure and build u-boot
compiled_and_install_uboot_bbb() {
UBOOT_BRANCH="v2014.10"
UBOOT_PATCH_NAME="0001-am335x_evm-uEnv.txt-bootz-n-fixes.patch"
UBOOT_PATCH_URL=https://raw.githubusercontent.com/eewiki/u-boot-patches/master/v2014.10/${UBOOT_PATCH_NAME}

if  [ -f ${CHROOT_DIR}/boot/MLO ] && \
    [ -f ${CHROOT_DIR}/boot/u-boot.img ]; then
    print "u-boot is already installed"
    return 0
fi

cd ${MAKE_DIR}

if [ -d "$MAKE_DIR/u-boot" ]; then
    rm -Rf ${MAKE_DIR}/u-boot
fi

if [ -f ${LDDOWNL_DIR}/${BOARD}/u-boot.tar.bz2 ]; then
    print "unpacking u-boot.tar.bz2"
    pbzip2 -dc -p${CORES} ${LDDOWNL_DIR}/${BOARD}/u-boot.tar.bz2 | sudo tar x
else
    print "Git clone u-boot repository"
    git clone git://git.denx.de/u-boot.git

    print "Start created archive for u-boot repository"
    sudo tar -c ./u-boot/ | pbzip2 -c -5 -p${CORES} > ${LDDOWNL_DIR}/${BOARD}/u-boot.tar.bz2
    print "End created archive u-boot repository into ${LDDOWNL_DIR}/${BOARD}"
fi

cd $MAKE_DIR/u-boot/
git checkout ${UBOOT_BRANCH} -b tmp

if [ ! -f ${LDDOWNL_DIR}/${BOARD}/${UBOOT_PATCH_NAME} ]; then
    print "Downloads patch u-boot for ${BOARD_FULL_NAME}"
    wget -cN -P ${LDDOWNL_DIR}/${BOARD}/ ${UBOOT_PATCH_URL}
fi

cp ${LDDOWNL_DIR}/${BOARD}/${UBOOT_PATCH_NAME} ${MAKE_DIR}/u-boot
print "Apply patch for u-boot"
patch -p1 < ./${UBOOT_PATCH_NAME}
print "Start build u-boot"
make ARCH=arm CROSS_COMPILE=${CC} distclean -j${CORES}
make ARCH=arm CROSS_COMPILE=${CC} am335x_evm_config -j${CORES}
make ARCH=arm CROSS_COMPILE=${CC} -j${CORES}
print "End build u-boot"


print "Копируем в /boot раздел карты бутлоадер"
sudo cp -v ${MAKE_DIR}/u-boot/MLO ${CHROOT_DIR}/boot/
sudo cp -v ${MAKE_DIR}/u-boot/u-boot.img ${CHROOT_DIR}/boot/
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
    rm -Rf ${MAKE_DIR}/libwebsockets/build
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
compiled_and_install_mongodb_in_chroot_rpi() {
print "Start compiling and installing mongodb for rpi"
MONGO_INSTALL_DIR=/usr/local/mongo

if [ -f ${CHROOT_DIR}/${MONGO_INSTALL_DIR}/lib/libmongoclient.a ] && [ -f ${CHROOT_DIR}/etc/init.d/mongod ]; then
  print "mongodb is already installed in rootfs"
  return 0
fi

if [ ! -d ${CHROOT_DIR}/home/ld/code ]; then
 mkdir -p ${CHROOT_DIR}/home/ld/code
fi

if [ ! -d ${CHROOT_DIR}/home/ld/code/mongo-nonx86 ]; then
  if [ -f ${BOARD_DIR}/mongodb_wheezy.tar.gz ]; then
    print "Archive of the source available, unpack"
    sudo tar -xf ${BOARD_DIR}/mongodb_wheezy.tar.gz -C ${CHROOT_DIR}/home/ld/code
  else
    print "Downloads repository mongodb"
    run_cmd_chroot "cd /home/ld/code/; git clone https://github.com/skrabban/mongo-nonx86"
  fi
fi

print "Build and installing mongodb"
run_cmd_chroot "cd /home/ld/code/mongo-nonx86; scons --prefix=${MONGO_INSTALL_DIR} install -j${CORES}"

if [ ! -f ${BOARD_DIR}/mongodb_wheezy.tar.gz ]; then
    print "Start create arhive mongodb"
    sudo sh -c "cd ${CHROOT_DIR}/home/ld/code; tar -czf ${BOARD_DIR}/mongodb_wheezy.tar.gz ./mongo-nonx86"
    print "End create mongodb_wheezy.tar.gz save into ${BOARD_DIR}"
fi

print "Preparatory operations to run mongodb"
# Delete previous settings
if grep -q "mongodb:" ${CHROOT_DIR}/etc/passwd; then
    print "deluser"
    run_cmd_chroot "deluser mongodb"
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
run_cmd_chroot "adduser --firstuid 100 --ingroup nogroup --shell /etc/false --disabled-password --gecos '' --no-create-home mongodb"
# A folder for log files mongodb
run_cmd_chroot "mkdir -p /var/log/mongodb/"
# Permissions for log file
run_cmd_chroot "chown mongodb:nogroup /var/log/mongodb/"
# A folder for state data mongodb
run_cmd_chroot "mkdir -p /var/lib/mongodb"
# Permissions for the folder
run_cmd_chroot "chown mongodb:nogroup /var/lib/mongodb"
# Moving init.d script to etc
sudo cp ${CHROOT_DIR}/home/ld/code/mongo-nonx86/debian/init.d ${CHROOT_DIR}/etc/init.d/mongod
# Moving our config file to etc
sudo cp ${CHROOT_DIR}/home/ld/code/mongo-nonx86/debian/mongodb.conf ${CHROOT_DIR}/etc/
# Linking folders up
run_cmd_chroot "ln -s ${MONGO_INSTALL_DIR}/bin/mongod /usr/bin/mongod"
print "minimize MongoDB journaling files"
echo 'smallfiles=true' | sudo tee -a ${CHROOT_DIR}/etc/mongodb.conf

run_cmd_chroot "chmod u+x /etc/init.d/mongod"
run_cmd_chroot "update-rc.d mongod defaults"


print "Remove the build directory of the rootfs mongodb"
sudo sh -c "rm -R ${CHROOT_DIR}/home/ld/code/mongo-nonx86"
print "End compiling and installing mongodb for rpi"
}

compiled_and_install_mongodb() {

print "compiling and installing mongodb"

cd ${MAKE_DIR}/mongodb
scons --propagate-shell-environment --cc=${CC}gcc --cxx=${CC}g++ --full install --prefix=${MAKE_DIR} -j${CORES}

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
    if [ ! -f ${LDDOWNL_DIR}/${BOARD}/${NAME_KERNEL}.tar.gz ]; then
        #wget -c -P ${LDDOWNL_DIR}/${BOARD} https://github.com/raspberrypi/linux/archive/${NAME_KERNEL}.zip
        print "git clone ${NAME_KERNEL}"
        git clone -b ${NAME_KERNEL} --depth 1 git://github.com/raspberrypi/linux.git kernel
        print "end clone ${NAME_KERNEL}"
        print "create archive ${NAME_KERNEL}.tar.gz intro ${LDDOWNL_DIR}/${BOARD}"
        tar -czf ${LDDOWNL_DIR}/${BOARD}/${NAME_KERNEL}.tar.gz ./kernel
        print "end create archive ${NAME_KERNEL}.tar.gz"
    else
        print "unpacking ${NAME_KERNEL}.tar.gz intro ${MAKE_DIR}"
        tar -xf ${LDDOWNL_DIR}/${BOARD}/${NAME_KERNEL}.tar.gz -C ${MAKE_DIR}
    fi
else
    print "git reset and clean for ${NAME_KERNEL}"
    cd ${MAKE_DIR}/kernel
    git reset --hard
    git clean -fdx
    git pull
    #cd ${MAKE_DIR}; tar -czf ${LDDOWNL_DIR}/${BOARD}/${NAME_KERNEL}.tar.gz ./kernel
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
if [ ! -f ${LDDOWNL_DIR}/${BOARD}/rpi_xenomai_config ]; then
    print "Download rpi_xenomai_config"
    wget -c -P ${LDDOWNL_DIR}/${BOARD} https://www.dropbox.com/s/dcju74md5sz45at/rpi_xenomai_config
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
cp ${LDDOWNL_DIR}/${BOARD}/rpi_xenomai_config ${KBUILD_DIR}/.config

print "mrproper"
make mrproper

if [ $MENUCONFIG = YES ];then
    make ARCH=arm O=build menuconfig
else
    make ARCH=arm O=build oldconfig
fi

print "Compile kernel"
make ARCH=arm O=${KBUILD_DIR} CROSS_COMPILE=${CC} -j${CORES}
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

    if [ ! -f ${LDDOWNL_DIR}/${BOARD}/firmware.zip ]; then
        print "Download firmware for rpi"
        wget -O ${LDDOWNL_DIR}/${BOARD}/firmware.zip https://github.com/raspberrypi/firmware/archive/master.zip
    fi

    print "Installing firmware rpi"
    cd ${MAKE_DIR}
    unzip -n ${LDDOWNL_DIR}/${BOARD}/firmware.zip
    sudo cp -a -u  ${MAKE_DIR}/firmware-master/hardfp/opt/vc ${CHROOT_DIR}/opt/
    sudo cp -u ${MAKE_DIR}/firmware-master/boot/*bin ${CHROOT_DIR}/boot/
    sudo cp -u ${MAKE_DIR}/firmware-master/boot/*dat ${CHROOT_DIR}/boot/
    sudo cp -u ${MAKE_DIR}/firmware-master/boot/*elf ${CHROOT_DIR}/boot/
else
    print "firmware is already installed for rpi"
fi

print "End build_kernel_xeno2_rpi"
}


# Compiled_and_install_nodejs for rpi
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
    if [ ! -f ${LDDOWNL_DIR}/${BOARD}/nodejs.zip ]; then
      print "Download NodeJS src"
      wget -c -O ${LDDOWNL_DIR}/${BOARD}/nodejs.zip https://github.com/joyent/node/archive/v0.10.32-release.zip
    fi
    cd ${MAKE_DIR}
    print "unpacking nodejs.zip intro ${MAKE_DIR}"
    unzip -n ${LDDOWNL_DIR}/${BOARD}/nodejs.zip
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


# Create rootfs use debootstrap
debootstrap() {
print "Start debootstrap for ${BOARD_FULL_NAME}"

# Test old bad install rootfs and clean rootfs
if  [ -d ${CHROOT_DIR} ] && \
    [ ! -f ${BOARD_DIR}/rootfs-${BOARD}-${DISTR}.tar.bz2 ] && \
    [ $(ls -a ${CHROOT_DIR}/proc |cat -b|grep 3 > /dev/null;echo $?) ]; then
    print "Clean old bad install rootfs and clean rootfs"
    umount_proc_sysfs_devpts
    sudo rm -R ${CHROOT_DIR}
fi

# Checking directory rootfs if it's not done unpacking archives with rootfs
if [ ! -d ${CHROOT_DIR} ]; then
    cd ${MAKE_DIR}
    if  [ -f ${BOARD_DIR}/rootfs-${BOARD}-${DISTR}.tar.bz2 ]; then
        print "unpacking rootfs-${BOARD}-${DISTR}.tar.bz2"
        pbzip2 -dc -p${CORES} ${BOARD_DIR}/rootfs-${BOARD}-${DISTR}.tar.bz2 | sudo tar x
        return 0
    fi

    # Debootstrap step 1
    print "Debootstrap step 1"
    sudo debootstrap --arch=${ARCH} --include=ssh --foreign --no-check-gpg --include=ca-certificates $DISTR ${CHROOT_DIR} $DISTR_MIRROR
fi

# Debootstrap step 2
if [ -d ${CHROOT_DIR}/debootstrap ]; then
    print "Debootstrap step 2"
    qemu_copy_to_roofs
    sudo chroot ${CHROOT_DIR} /debootstrap/debootstrap --second-stage  --verbose
    qemu_remove_from_roofs

    roofs_settings_${BOARD}
    rootfs_installing_packages
fi

umount_proc_sysfs_devpts

if [ ! -f ${BOARD_DIR}/rootfs-${BOARD}-${DISTR}-mini.tar.bz2 ]; then
    cd ${MAKE_DIR}
    print "Start created archive rootfs-${BOARD}-${DISTR}-mini.tar.bz2"
    sudo tar -c ./rootfs/ | pbzip2 -c -5 -p${CORES} > ${BOARD_DIR}/rootfs-${BOARD}-${DISTR}-mini.tar.bz2
    print "End created archive rootfs-${BOARD}-${DISTR}-mini.tar.bz2 into ${BOARD_DIR}"
fi

return 0
}


# Copy truncated rootfs
copy_truncate_rootfs() {
print "Start copy truncated rootfs in ${BOARD_DIR}/rootfs"
# Move rootfs trunc to chroot directory
if [ -d ${ROOTFS_TRUNC_DIR} ]; then
    sudo mv ${ROOTFS_TRUNC_DIR} ${CHROOT_DIR}/home/ld/code/rootfs
fi

run_cmd_chroot "rsync -progress -rL --delete --exclude="/usr/local/xenomai" /usr /home/ld/code/rootfs/"
run_cmd_chroot "cp -R /usr/local/xenomai /home/ld/code/rootfs/usr/local/"
sudo chown -R $(whoami):$(whoami) ${CHROOT_DIR}/home/ld/code/rootfs
mv ${CHROOT_DIR}/home/ld/code/rootfs ${ROOTFS_TRUNC_DIR}

print "End copy truncated rootfs"
}


# Build and install software LinuxDrone to rootfs
build_install_linuxdrone() {
print "Start build and install software LinuxDrone to rootfs"

INSTALL_DIR=${LDROOT_DIR}/build.Debug/install

# Clear old build LinuxDrone
if [ -d ${LDROOT_DIR}/build.Debug ]; then
    rm -R ${LDROOT_DIR}/build.Debug
fi

# Downloads GPL version Ext JS
if [ ! -d "${LDTOOLS_DIR}/extjs" ]; then
    if [ ! -f ${LDDOWNL_DIR}/extjs-gpl.zip ]; then
      print "Download extJS-gpl"
      wget -c -O ${LDDOWNL_DIR}/extjs-gpl.zip http://cdn.sencha.com/ext/gpl/ext-5.0.1-gpl.zip
    fi
    cd ${LDTOOLS_DIR}
    print "unpacking extjs-gpl.zip intro ${LDTOOLS_DIR}"
    unzip -n ${LDDOWNL_DIR}/extjs-gpl.zip
    mv ext-* extjs
fi

# Create project run cmake
cd ${LDROOT_DIR}
./configure.sh -b ${BOARD}
cd ${LDROOT_DIR}/build.Debug
make -j${CORES}
make install -j${CORES}

# build Web Configurator
cd ${LDROOT_DIR}/webapps/configurator/public
${LDTOOLS_DIR}/Sencha/Cmd/${SENCHA_CMD_VER}/sencha app build

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
LDTOOLS_DIR=${LDROOT_DIR}/tools
BOARD_DIR=${LDTOOLS_DIR}/board/${BOARD}
# Getting the full path if you had used a symbolic link to the folder
#BOARD_DIR=$(readlink -f $(readlink -f "$(dirname "${BOARD_DIR}")")/$(basename "${BOARD_DIR}"))
# The path to the cross compiler
CC_DIR=${BOARD_DIR}/cc
MAKE_DIR=${BOARD_DIR}/make_dir
CHROOT_DIR=${MAKE_DIR}/rootfs
ROOTFS_TRUNC_DIR=${BOARD_DIR}/rootfs
LOCALES=${LANG}
CORES=$(grep "^cpu cores" /proc/cpuinfo | awk -F : '{print $2}' | head -1 | sed 's/^[ ]*//')
CORES=$((${CORES} + 1))
DISK_SIZE=2048

if [ ! -d ${LDDOWNL_DIR}/${BOARD} ]; then
    print "Create a directory where all downloads soft"
    mkdir -p ${LDDOWNL_DIR}/${BOARD}
fi

if [ ! -d ${MAKE_DIR} ]; then
    print "Create a directory for build packages"
    mkdir -p ${MAKE_DIR}
fi

sudo_timestamp_timeout 800
install_common_software_the_host

case "${BOARD}" in
    bbb)
        BOARD_FULL_NAME="BeagleBone Black"
        ARCH=armhf
        MARCH=armv7-a
        MFPU=vfp3
        DISTR=trusty
        #DISTR=saucy
        DISTR_MIRROR=http://ports.ubuntu.com/ubuntu-ports/
        PKG_LIST_BOARD="mongodb nodejs npm"

        download_install_crosscompiler_bbb
        debootstrap
        #rootfs_installing_packages
        if [ ${START_SHELL} = YES ]; then
            start_shell_in_chroot
        fi
        compiled_and_install_uboot_bbb
        ;;

    rpi)
        BOARD_FULL_NAME="Raspberry PI"
        ARCH=armhf
        MARCH=armv6 #j
        MFPU=vfp
        MCPU=arm1176jzf-s
        MFLOAT_ABI=hard
        NAME_LIBWEB=libwebsockets-1.3-chrome37-firefox30.tar.gz
        NAME_MONGOC=0.94.2
        DISTR=wheezy
        DISTR_MIRROR=http://archive.raspbian.org/raspbian/
        PKG_LIST_BOARD=""
        #DISTR_MIRROR=http://mirrors-ru.go-parts.com/raspbian/
        #DISTR_MIRROR=http://mirror.netcologne.de/raspbian/raspbian/
        #DISTR=jessie
        #DISTR_MIRROR=http://mirrordirector.raspbian.org/raspbian

        download_install_crosscompiler_rpi
        debootstrap
        rootfs_installing_packages
        if [ ${START_SHELL} = YES ]; then
            start_shell_in_chroot
        fi
        compiled_and_install_mongodb_in_chroot_rpi
        compiled_and_install_nodejs_npm_rpi
        compiled_and_install_libwebsockets
        compiled_and_install_mongoc
        build_kernel_xeno2_rpi
        copy_truncate_rootfs
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
