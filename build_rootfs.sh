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

USAGE="Example usage: `basename $0` -b bbb -x xeno2 -d /dev/sdc\n\n
------------ Options -------------- \n
help:                            -h \n
version:                         -v \n
use menuconfig for buld kernel:  -m \n
sdcard device name:              -d (dev/sdc)   \n
board name:                      -b (bbb, rpi)  \n
xenomai type                     -x (xeno2, cobalt, mercury) \n
clean directory                  -c (rootfs xenomai kernel cc mongoc libweb) \n
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

# Clean directory if set in variable ${CLEAN}
clean_directory() {
if [ $(echo ${CLEAN} | grep -o $1) ] && \
   [ -d "$2" ]; then
    print "Clean directory $2"
    sudo rm -Rf "$2"
fi
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
for x in $(mount | grep /dev/mapper/loop | awk '{print $1}')
do
    print "Umount partitions: ${x}"
    sudo umount ${x}
done

# Удаляем устройства-разделы с помощью kpartx
for x in $(losetup -a | awk '{print $1}' | tr -d \:)
do
    print "Remove device-partitions ${x}"
    sudo losetup -d ${x}
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

# Copy to /boot
sudo rm -R /mnt/boot/* || true
if [ ${AT91BS_USE} ]; then
    sudo cp -v ${CHROOT_DIR}/boot/boot.bin /mnt/boot/
fi

if [ ${UBOOT_USE} ]; then
    for ITEM in ${UBOOT_FILES}; do
      sudo cp -v ${CHROOT_DIR}/boot/${ITEM} /mnt/boot/
    done
    sudo cp -v ${CHROOT_DIR}/boot/uEnv.txt /mnt/boot/
fi
sudo cp -R -u ${CHROOT_DIR}/boot/* /mnt/boot/
sync

# Размонтируем образ диска
sudo umount /dev/mapper/${DEV_LOOP}p1
sudo umount /dev/mapper/${DEV_LOOP}p2
sudo losetup -d /dev/${DEV_LOOP}
sudo kpartx -dv /dev/${DEV_LOOP}

print "create arhive the image sdcard ${DISK_IMAGE}.bz2"
cd ${BOARD_DIR}
if [ -f ${DISK_IMAGE}.bz2 ]; then
    sudo rm ${DISK_IMAGE}.bz2
fi
pbzip2 -k -9 -p${CORES} ${DISK_IMAGE}

lsblk
print "To copy a disk image on a flash card, enter one of these commands, edit /dev/sdX"
print "sudo  sh -c \"bzcat ${DISK_IMAGE}.bz2 > ${DEV_DISK}\""
print "sudo dd if=${DISK_IMAGE} of=${DEV_DISK} bs=1M"
#sudo sh -c "bzcat ${DISK_IMAGE}.bz2 > ${DEV_DISK}"
#sudo dd if=${DISK_IMAGE} of=${DEV_DISK} bs=1M

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
pre-up modprobe g_ether
auto usb0
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

print "Downloads scripts for expand rootfs, copy to /home/ld"
if [ ! -f ${LDDOWNL_DIR}/expand_rootfs.sh ]; then
    print "Download expand_rootfs.sh"
    wget -c -P ${LDDOWNL_DIR} http://wiki.linuxdrone.org/download/attachments/5275816/expand_rootfs.sh
    chmod +x ${LDDOWNL_DIR}/expand_rootfs.sh
fi
sudo cp ${LDDOWNL_DIR}/expand_rootfs.sh ${CHROOT_DIR}/home/ld
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
/dev/mmcblk0p1 /boot  auto  defaults  0  2
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
UENV='EOF
##This will work with: Angstrom 2013.06.20 u-boot.
uname_r=3.8.13

loadaddr=0x82000000
fdtaddr=0x88000000
rdaddr=0x88080000

initrd_high=0xffffffff
fdt_high=0xffffffff

console=ttyO0,115200n8
mmcroot=/dev/mmcblk0p2 ro
mmcrootfstype=ext4 rootwait fixrtc

# Enable I2C1 bus? disable HDMI/eMMC
optargs=quiet capemgr.enable_partno=BB-I2C1-400, capemgr.disable_partno=BB-BONELT-HDMI,BB-BONELT-HDMIN

loadximage=load mmc 0:1 \${loadaddr} /vmlinuz-\${uname_r}
loadxfdt=load mmc 0:1 \${fdtaddr} /dtbs/\${uname_r}/\${fdtfile}
loadxrd=load mmc 0:1 \${rdaddr} /initrd.img-\${uname_r}; setenv rdsize \${filesize}
loaduEnvtxt=load mmc 0:1 \${loadaddr} /uEnv.txt ; env import -t \${loadaddr} \${filesize};
loadall=run loaduEnvtxt; run loadximage; run loadxfdt;

mmcargs=setenv bootargs console=tty0 console=\${console} \${optargs} \${cape_disable} \${cape_enable} root=\${mmcroot} rootfstype=\${mmcrootfstype} \${cmdline}

uenvcmd=run loadall; run mmcargs; bootz \${loadaddr} - \${fdtaddr};
EOF
'
sudo sh -c "cat >${CHROOT_DIR}/boot/uEnv.txt << ${UENV}"
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

# Settings Arietta G25 for the rootfs
roofs_settings_g25() {
print "Apply settings for ${BOARD_FULL_NAME}"
print "Adding deb sources to /etc/apt/sources.list"
sudo sh -c "echo 'deb ${DISTR_MIRROR} ${DISTR} main contrib non-free' >> ${CHROOT_DIR}/etc/apt/sources.list"
sudo sh -c "echo 'deb-src ${DISTR_MIRROR} ${DISTR} main contrib non-free' >> ${CHROOT_DIR}/etc/apt/sources.list"

rootfs_common_settings

# Remove eth0 in /etc/network/interfaces
sed -i "s/auto eth0//g" ${CHROOT_DIR}/etc/network/interfaces
sed -i "s/iface eth0 inet dhcp//g" ${CHROOT_DIR}/etc/network/interfaces

print "Edit /etc/fstab"
sudo sh -c "cat> ${CHROOT_DIR}/etc/fstab << EOF
/dev/mmcblk0p2  /  auto  errors=remount-ro  0  1
/dev/mmcblk0p1  /boot  auto  defaults  0  2
EOF
"

print "Create 70-persistent-net.rules"
sudo sh -c "cat >${CHROOT_DIR}/etc/udev/rules.d/70-persistent-net.rules << EOF
'# Arietta G25: net device ()
SUBSYSTEM==\"net\", ACTION==\"add\", DRIVERS==\"?*\", ATTR{dev_id}==\"0x0\", ATTR{type}==\"1\", KERNEL==\"wlan*\", NAME=\"wlan0\"
EOF
"

print "Set debug port ttyS0 115200"
sudo sh -c "cat >>${CHROOT_DIR}/etc/inittab << EOF
T0:2345:respawn:/sbin/getty -L ttyS0 115200 vt100
EOF
"

print "Create /boot/uEnv.txt"
UENV='EOF
##This will work with: Angstrom 2013.06.20 u-boot.
uname_r=3.14.17-xeno2.zImage

loadzimage=load mmc ${mmcdev}:${mmcpart} ${loadaddr} ${uname_r}

#SAM9G25-EK
fdtfile=/dtbs/at91sam9g25ek.dtb

#Default u-boot settings:
#console=ttyS0,115200
#optargs=console=tty0

##Un-comment to enable systemd in Debian Wheezy
#optargs=quiet init=/lib/systemd/systemd

mmcroot=/dev/mmcblk0p2 ro
mmcrootfstype=ext4 rootwait fixrtc

#Run custom u-boot commands early:
#uenvcmd=

EOF
'
sudo sh -c "cat >${CHROOT_DIR}/boot/uEnv.txt << ${UENV}"
}


# Installing packages in rootfs
rootfs_installing_packages() {
print "Updating and upgrade installing packages"

PKG_LIST_COMMON="ssh sudo mc wget git screen build-essential cpufrequtils i2c-tools usbutils \
wpasupplicant wireless-tools gdbserver \
libboost-system.dev libboost-filesystem.dev libboost-thread.dev libboost-program-options.dev \
scons htop ntpdate libssl-dev udhcpd vim"

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

#GIT_LINUX_KERNEL_SRC="https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git"
GIT_LINUX_KERNEL_SRC="https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git"

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

# Git clone kernel source
if [ ! -f "${LINUX_KERNEL_SRC_DIR}/.git/config" ]; then
    cd ${LDTOOLS_DIR}
    if [ -d "${LINUX_KERNEL_SRC_DIR}" ]; then
        rm -rf "${LINUX_KERNEL_SRC_DIR}"
    fi
    if [ -f ${LDDOWNL_DIR}/linux-src.tar.bz2 ]; then
        print "unpacking linux-src.tar.bz2"
        pbzip2 -dc -p${CORES} ${LDDOWNL_DIR}/linux-src.tar.bz2 | sudo tar x
    else
        print "Start cloning ${GIT_LINUX_KERNEL_SRC} into default location: ${LINUX_KERNEL_SRC_DIR}"
        git clone ${GIT_LINUX_KERNEL_SRC} ${LINUX_KERNEL_SRC_DIR}

        print "Start create archive for ${LINUX_KERNEL_SRC_DIR}"
        sudo tar -c ./linux-src/ | pbzip2 -c -5 -p${CORES} > ${LDDOWNL_DIR}/linux-src.tar.bz2
        print "End created archive linux-src repository into ${LDDOWNL_DIR}"
    fi
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

CC_PREFIX=arm-linux-gnueabihf

clean_directory " cc " "${CC_DIR}"

if [ ! -f ${CC_DIR}/bin/${CC_PREFIX}-gcc ]; then
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

export CC=${CC_DIR}/bin/${CC_PREFIX}-
export PATH=${CC_DIR}/bin:$PATH
}

download_install_crosscompiler_bbb() {

CC_PREFIX=arm-linux-gnueabihf

clean_directory " cc " "${CC_DIR}"

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
  mv ./*-${CC_PREFIX}*/* .
  rm -R ./*-${CC_PREFIX}*
else
  print "Cross-compiler is already installed for bbb"
fi

export CC=${CC_DIR}/bin/${CC_PREFIX}-
export PATH=${CC_DIR}/bin:$PATH
}

download_install_crosscompiler_g25() {

clean_directory " cc " "${CC_DIR}"

#CC_PREFIX=arm-none-eabi
CC_PREFIX=arm-linux-gnueabi

if [ ! -f ${CC_DIR}/bin/${CC_PREFIX}-gcc ]; then
  if [ ! -d ${CC_DIR} ]; then
      print "Create a directory for cross-compiler"
      mkdir -p ${CC_DIR}
  elif [ $(find ${CC_DIR} -type f | wc -l) -ne 0 ]; then
      print "Clean ${CC_DIR}"
      rm -R ${CC_DIR}/*
  fi

  if [ ! -f ${LDDOWNL_DIR}/${BOARD}/crosscompiler.tar.bz2 ]; then
    print "Download cross-compiler bbb"
    #wget -O ${LDDOWNL_DIR}/${BOARD}/crosscompiler.tar.gz https://releases.linaro.org/14.09/components/toolchain/binaries/gcc-linaro-arm-none-eabi-4.9-2014.09_linux.tar.xz
    wget -O ${LDDOWNL_DIR}/${BOARD}/crosscompiler.tar.bz2 https://releases.linaro.org/12.04/components/toolchain/binaries/gcc-linaro-arm-linux-gnueabi-2012.04-20120426_linux.tar.bz2
  fi

  print "Installing cross-compiler bbb"
  cd ${CC_DIR}
  tar -xjf ${LDDOWNL_DIR}/${BOARD}/crosscompiler.tar.bz2
  mv ./gcc-linaro-${CC_PREFIX}*/* .
  rm -R ./gcc-linaro-${CC_PREFIX}*
else
  print "Cross-compiler is already installed for bbb"
fi

export CC=${CC_DIR}/bin/${CC_PREFIX}-
export PATH=${CC_DIR}/bin:$PATH
}

download_install_pasm() {
print "Download, configure and build pasm"

AM335x_PRU_PACKAGE_DIR=${MAKE_DIR}/am335x_pru_package-master

clean_directory " pasm " "${PASM_DIR}"
clean_directory " pasm " "${AM335x_PRU_PACKAGE_DIR}"

if [ -f ${PASM_DIR}/pasm ]; then
    print "PASM already installed"
    return 0
fi

if [ ! -d ${AM335x_PRU_PACKAGE_DIR} ]; then
    if [ ! -f ${LDDOWNL_DIR}/${BOARD}/am335x_pru_package.zip ]; then
        print "Download AM335x PRU PACKAGE"
        wget -O ${LDDOWNL_DIR}/${BOARD}/am335x_pru_package.zip https://github.com/beagleboard/am335x_pru_package/archive/master.zip
    fi
    print "unpacking am335x_pru_package.zip intro ${MAKE_DIR}"
    cd ${MAKE_DIR}
    unzip -n ${LDDOWNL_DIR}/${BOARD}/am335x_pru_package.zip
fi

print "Build PASM for linux"
cd ${AM335x_PRU_PACKAGE_DIR}/pru_sw/utils/pasm_source
./linuxbuild
mkdir -p ${PASM_DIR} || true
cp -uv ${AM335x_PRU_PACKAGE_DIR}/pru_sw/utils/pasm ${PASM_DIR}/
}


# Download, patch, configure and build u-boot
compiled_and_install_uboot() {

UBOOT_USE=1

clean_directory " u-boot " "${MAKE_DIR}/u-boot"

# Test installed uboot files to rootfs
for ITEM in ${UBOOT_FILES}; do
    if  [ ! -f ${CHROOT_DIR}/boot/${ITEM} ]; then
        UBOOT_INST=YES
        break
    fi
done

if  [ "x${UBOOT_INST}" = "x" ]; then
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
make ARCH=arm CROSS_COMPILE=${CC} ${UBOOT_DEFCONFIG} -j${CORES}
make ARCH=arm CROSS_COMPILE=${CC} -j${CORES}
print "End build u-boot"

print "Copy to  /boot u-boot files"
for ITEM in ${UBOOT_FILES}; do
  sudo cp -v ${MAKE_DIR}/u-boot/${ITEM} ${CHROOT_DIR}/boot/
done
}

# Download, patch, configure and build AT91Bootstrap
compiled_and_install_at91bootstrap() {

AT91BS_USE=1
AT91BS_DIR="${MAKE_DIR}/at91bootstrap"

clean_directory " at91bootstrap " "${AT91BS_DIR}"

# Test installed AT91Bootstrap files to rootfs
if  [ -f ${CHROOT_DIR}/boot/boot.bin ]; then
    print "AT91Bootstrap is already installed"
    return 0
fi

cd ${MAKE_DIR}

if [ -d "${AT91BS_DIR}" ]; then
    rm -Rf ${AT91BS_DIR}
fi

if [ -f ${LDDOWNL_DIR}/${BOARD}/at91bootstrap.tar.bz2 ]; then
    print "unpacking at91bootstrap.tar.bz2"
    pbzip2 -dc -p${CORES} ${LDDOWNL_DIR}/${BOARD}/at91bootstrap.tar.bz2 | sudo tar x
else
    print "Git clone at91bootstrap repository"
    git clone ${AT91BS_GIT}

    print "Start created archive for at91bootstrap repository"
    sudo tar -c ./at91bootstrap/ | pbzip2 -c -5 -p${CORES} > ${LDDOWNL_DIR}/${BOARD}/at91bootstrap.tar.bz2
    print "End created archive at91bootstrap repository into ${LDDOWNL_DIR}/${BOARD}"
fi

cd ${AT91BS_DIR}
git checkout ${AT91BS_BRANCH} -b tmp

if [ ! -f ${LDDOWNL_DIR}/${BOARD}/${UBOOT_PATCH_NAME} ]; then
    print "Downloads patch at91bootstrap for ${BOARD_FULL_NAME}"
    wget -cN -P ${LDDOWNL_DIR}/${BOARD}/ ${UBOOT_PATCH_URL}
fi

print "Start build at91bootstrap"
make ARCH=arm CROSS_COMPILE=${CC} distclean -j${CORES}
make ARCH=arm CROSS_COMPILE=${CC} ${AT91BS_DEFCONFIG} -j${CORES}
make ARCH=arm CROSS_COMPILE=${CC} -j${CORES}
print "End build at91bootstrap"

print "Copy boot files to to /boot partitions"
sudo cp -v ${AT91BS_DIR}/binaries/${AT91BS_FILES} ${CHROOT_DIR}/boot/boot.bin

#for ITEM in ${AT91BS_FILES}; do
#  sudo cp -v ${AT91BS_DIR}/${AT91BS_DIR}/binaries/${ITEM} ${CHROOT_DIR}/boot/
#done
}


# Download, configure and build libwebsockets
compiled_and_install_libwebsockets() {
print "Start compiled_and_install_libwebsockets"
LWS_IPV6=ON

clean_directory " libweb " "${MAKE_DIR}/libwebsockets"

if [ -f ${CHROOT_DIR}/usr/local/include/libwebsockets.h ]; then
    print "libwebsockets is already installed"
    return
fi

copy_truncate_rootfs

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

print "Create CMake Toolchain file for crosscompiling on ${BOARD_FULL_NAME}"
cat >${MAKE_DIR}/libwebsockets/${BOARD}.cmake << EOF
# CMake Toolchain file for crosscompiling on ${BOARD_FULL_NAME}.
set(RFS_DIR ${ROOTFS_TRUNC_DIR})

set(CROSS "${CC}")

set(CMAKE_SYSTEM_NAME Linux)

set(CMAKE_C_COMPILER "\${CROSS}gcc")
set(CMAKE_CXX_COMPILER "\${CROSS}g++")

set(CMAKE_FIND_ROOT_PATH "\${RFS_DIR}")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

find_path(OPENSSL_DIR
    NAMES openssl/opensslconf.h
    PATHS /usr/include /usr/include/arm-linux-gnueabihf )

if( OPENSSL_DIR )
    mark_as_advanced(OPENSSL_DIR)
    include_directories(\${INCLUDE_DIRECTORIES} \${OPENSSL_DIR})
else( OPENSSL_DIR )
    MESSAGE(STATUS "openssl/opensslconf.h NOT found.")
endif( OPENSSL_DIR )
EOF

if [ ${BOARD} = rpi ]; then
    LWS_IPV6=OFF
fi

if [ -d ${MAKE_DIR}/libwebsockets/build ]; then
    rm -Rf ${MAKE_DIR}/libwebsockets/build
fi

mkdir -p ${MAKE_DIR}/libwebsockets/build
cd ${MAKE_DIR}/libwebsockets/build

cmake .. -DCMAKE_INSTALL_PREFIX:PATH=${CHROOT_DIR}/usr/local \
        -DCMAKE_TOOLCHAIN_FILE=${MAKE_DIR}/libwebsockets/${BOARD}.cmake \
        -DLWS_IPV6=${LWS_IPV6}

make -j${CORES}
sudo make install -j${CORES}

print "libwebsockets installed"
}


# Compiled MongoDB for RaspberryPI in system debian wheenzy
compiled_and_install_mongodb_in_chroot() {
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

clean_directory " mongoc " "${MAKE_DIR}/mongo-c-driver"

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

# Set fot mfpu or msoft in CFLAGS depending on the target platform
if [ ${MFPU} ]; then
    MFPU_MSOFT="mfpu=${MFPU}"
else
    MFPU_MSOFT=msoft-float
fi

env \
CPP="${CC}gcc -E" \
STRIP="${CC}strip" \
OBJCOPY="${CC}objcopy" \
AR="${CC}ar" \
RANLIB="${CC}ranlib" \
LD="${CC}ld" \
OBJDUMP="${CC}objdump" \
CC="${CC}gcc" \
CXX="${CC}g++" \
NM="${CC}nm" \
AS="${CC}as" \
./configure CFLAGS="-march=${MARCH} -${MFPU_MSOFT}" --host=${CC_PREFIX} --prefix=/usr/local
make -j${CORES}
sudo env PATH=$PATH make DESTDIR=${CHROOT_DIR} install -j${CORES}

print "mongo-c-driver-$NAME_MONGOC installed"
}


# Xenomai patch kernel and build library
xeno_apply() {

XENO_INSTALL_DIR="/usr/local/xenomai"

# Variable ${XENO}=[xeno2, cobalt, mercury]
if [ "x${XENO}" = "xxeno2" ]; then
    XENO_NAME="xenomai-2.6.4"
    XENO_URL=http://download.gna.org/xenomai/stable/${XENO_NAME}.tar.bz2
    XENO_PATCH_DIR="${MAKE_DIR}/xenomai/ksrc/arch/arm/patches"
elif [ "x${XENO}" = "xcobalt" ] || \
     [ "x${XENO}" = "xmercury" ]; then
    XENO_NAME="xenomai-3.0-rc2"
    XENO_URL=http://download.gna.org/xenomai/testing/${XENO_NAME}.tar.bz2
    XENO_PATCH_DIR="${MAKE_DIR}/xenomai/kernel/cobalt/arch/arm/patches"
else
    print "Wrong option is specified parameter -x, available options [xeno2, cobalt, mercury]"
    exit 1
fi

clean_directory "xenomai" "${MAKE_DIR}/xenomai"

# Download Xenomai
if [ ! -d "${MAKE_DIR}/xenomai" ]; then
    cd ${MAKE_DIR}
    if [ ! -f ${LDDOWNL_DIR}/${XENO_NAME}.tar.bz2 ]; then
      print "Download ${XENO_NAME}.tar.gz"
      wget -c -P ${LDDOWNL_DIR} ${XENO_URL}
    fi
    print "unpacking ${XENO_NAME}.tar.bz2 intro ${MAKE_DIR}"
    tar -xjf ${LDDOWNL_DIR}/${XENO_NAME}.tar.bz2
    mv ${XENO_NAME}* xenomai
fi

# Check availability of a patch for the specified version of the kernel.
XENO_TEST_VER=$(ls ${XENO_PATCH_DIR} | grep -o "${KERNEL_VER}" || true)
if [ "x${XENO_TEST_VER}" = "x" ]; then
    print "No patch xenomai for the specified version ${KERNEL_VER} of the kernel."
    exit 1
fi

unset XENO_BOARD_NAME
case "${BOARD}" in
    bbb)
        XENO_BOARD_NAME=beaglebone
        ;;
    rpi)
        XENO_BOARD_NAME=raspberry
        ;;
esac

XENO_PATCH_PRE="$(find ${XENO_PATCH_DIR} -iname ipipe-core-${KERNEL_VER}-${XENO_BOARD_NAME}-pre*.patch)"
XENO_PATCH_POST="$(find ${XENO_PATCH_DIR} -iname ipipe-core-${KERNEL_VER}-${XENO_BOARD_NAME}-post*.patch)"
XENO_PATCH="$(find ${XENO_PATCH_DIR} -iname ipipe-core-${KERNEL_VER}-arm*.patch)"


print "Apply Xenomai ipipe core patch for ${BOARD}-${KERNEL_VER}-${XENO}"
case "${BOARD}-${KERNEL_VER}-${XENO}" in
    bbb-3.8.13-xeno2)
        sed -i "/echo \"patch.sh ran successfully\"/q" ${KERNEL_PATCH_DIR}/patch.sh
        cat >>${KERNEL_PATCH_DIR}/patch.sh << EOF

        echo "Apply Xenomai ipipe core patch"
        cd ${KERNEL_DIR}
        patch -Np1 < ${XENO_PATCH_PRE}
        ${MAKE_DIR}/xenomai/scripts/prepare-kernel.sh --arch=arm --linux=${KERNEL_DIR} --adeos=${XENO_PATCH}
        patch -Np1 < ${XENO_PATCH_POST}
EOF
        ;;
    bbb-*-cobalt)
        sed -i "/echo \"patch.sh ran successfully\"/q" ${KERNEL_PATCH_DIR}/patch.sh
        cat >>${KERNEL_PATCH_DIR}/patch.sh << EOF

        echo "Apply Xenomai ipipe core patch"
        cd ${KERNEL_DIR}
        ${MAKE_DIR}/xenomai/scripts/prepare-kernel.sh --arch=arm --ipipe=${XENO_PATCH}
EOF
        ;;
    rpi-3.8.13-xeno2)
        cd ${KERNEL_DIR}
        patch -Np1 < ${XENO_PATCH_PRE}
        ${MAKE_DIR}/xenomai/scripts/prepare-kernel.sh --arch=arm --linux=${KERNEL_DIR} --adeos=${XENO_PATCH}
        patch -Np1 < ${XENO_PATCH_POST}
        ;;
    *-*-xeno2)
        cd ${KERNEL_DIR}
        ${MAKE_DIR}/xenomai/scripts/prepare-kernel.sh --arch=arm --linux=${KERNEL_DIR} --adeos=${XENO_PATCH}
        ;;
    *-*-cobalt)
        cd ${KERNEL_DIR}
        ${MAKE_DIR}/xenomai/scripts/prepare-kernel.sh --arch=arm --ipipe=${XENO_PATCH}
        ;;
    *)
        print "Xenomai working in mercury mode, not patches kernel"
        ;;
esac

print "Compile and install library, headers Xenomai to rootfs"
cd ${MAKE_DIR}/xenomai

# Set fot mfpu or msoft in CFLAGS depending on the target platform
if [ ${MFPU} ]; then
    MFPU_MSOFT="mfpu=${MFPU}"
else
    MFPU_MSOFT=msoft-float
fi

if [ "x${MARCH}" != "xarmv5" ]; then
    XENO_SMP="--enable-smp"
fi

if [ ${XENO} = "xeno2" ]; then
    ./configure CFLAGS="-march=${MARCH} -${MFPU_MSOFT}" LDFLAGS="-march=${MARCH}" \
                --host="${CC_PREFIX}" --with-cc=${CC}gcc --prefix=${XENO_INSTALL_DIR}
else
    ./configure CFLAGS="-march=${MARCH} -${MFPU_MSOFT} -marm" LDFLAGS="-march=${MARCH}" \
                --host="${CC_PREFIX}" --with-cc=${CC}gcc --prefix=${XENO_INSTALL_DIR} \
                --with-core=${XENO} "${XENO_SMP}"
fi

make -j${CORES}
sudo env PATH=$PATH make DESTDIR=${CHROOT_DIR} install -j${CORES}
sudo sh -c "echo /usr/local/xenomai/lib/ > ${CHROOT_DIR}/etc/ld.so.conf.d/xenomai.conf"

# Run ldconfig in chroot
run_cmd_chroot "ldconfig -v"
}

# Download, configure and build  kernel for Raspberry PI
build_kernel_xenomai_rpi() {

KERNEL_URL="git://github.com/raspberrypi/linux.git"
KERNEL_DIR="${MAKE_DIR}/kernel"
KBUILD_DIR="${KERNEL_DIR}/build"

unset XENO_BOARD_NAME
if [ "x${KERNEL_VER}" = "x3.14.17" ]; then
    KERNEL_BRANCH_CHA=946de0e6b6ed49eacb03e3cddfcb1d774d6378ed
fi

clean_directory " kernel " "${KERNEL_DIR}"

if  [ -f ${CHROOT_DIR}/boot/kernel.img ] && \
    [ -f ${CHROOT_DIR}/boot/bootcode.bin ] && \
    [ -f ${CHROOT_DIR}/usr/local/xenomai/include/xeno_config.h ] && \
    [ -d ${CHROOT_DIR}/opt/vc ]; then
    print "kernel and xenomai for rpi is already installed in rootfs"
    return 0
fi
print "Start compiled_and_install_kernel_rpi"

# Download kernel
cd ${MAKE_DIR}
if [  ! -f "${KERNEL_DIR}/.git/config" ]; then
    if [ ! -f ${LDDOWNL_DIR}/${BOARD}/${KERNEL_BRANCH}.tar.gz ]; then
        print "git clone ${KERNEL_BRANCH}"
        if [ ${KERNEL_BRANCH_CHA} ]; then
            git clone -b ${KERNEL_BRANCH} ${KERNEL_URL} kernel
            cd ${KERNEL_DIR}
            git reset --hard ${KERNEL_BRANCH_CHA}
            cd ${MAKE_DIR}
        else
            git clone -b ${KERNEL_BRANCH} --depth 1 ${KERNEL_URL} kernel
        fi
        print "end clone ${KERNEL_BRANCH}"
        print "create archive ${KERNEL_BRANCH}.tar.gz intro ${LDDOWNL_DIR}/${BOARD}"
        tar -czf ${LDDOWNL_DIR}/${BOARD}/${KERNEL_BRANCH}.tar.gz ./kernel
        #tar -c ./kernel/ | pbzip2 -c -5 -p${CORES} > ${LDDOWNL_DIR}/${BOARD}/${KERNEL_BRANCH}.tar.bz2
        print "end create archive ${KERNEL_BRANCH}.tar.gz"
    else
        print "unpacking ${KERNEL_BRANCH}.tar.gz intro ${MAKE_DIR}"
        tar -xf ${LDDOWNL_DIR}/${BOARD}/${KERNEL_BRANCH}.tar.gz -C ${MAKE_DIR}
    fi
else
    print "git reset and clean for ${KERNEL_BRANCH}"
    cd ${KERNEL_DIR}
    git reset --hard ${KERNEL_BRANCH_CHA}
    git clean -fdx
    #git pull
    #cd ${MAKE_DIR}; tar -czf ${LDDOWNL_DIR}/${BOARD}/${KERNEL_BRANCH}.tar.gz ./kernel
fi

# Xenomai patch kernel and build library
xeno_apply

mkdir -p ${KBUILD_DIR} || true
cd ${KERNEL_DIR}

# Download minimal config
if [ ! -f ${LDDOWNL_DIR}/${BOARD}/${BOARD}-${KERNEL_VER}-${XENO}-config ]; then
    print "Download ${BOARD}-${KERNEL_VER}-${XENO}-config"
    wget -c -P ${LDDOWNL_DIR}/${BOARD} https://cloud.mail.ru/public/95fd0247c068/kernel_config/${BOARD}-${KERNEL_VER}-${XENO}-config
fi

TEST_CONFIG=$(grep "# Linux/arm ${KERNEL_VER} Kernel Configuration" ${LDDOWNL_DIR}/${BOARD}/${BOARD}-${KERNEL_VER}-${XENO}-config || true)
if [ "x${TEST_CONFIG}" = "x" ]; then
    print "Not downloads config kernel: ${BOARD}-${KERNEL_VER}-${XENO}-config"
    rm ${LDDOWNL_DIR}/${BOARD}/${BOARD}-${KERNEL_VER}-${XENO}-config || true
    make ARCH=arm O=build bcm2835_defconfig
else
    print "Copy configuration file to KERNEL_DIR"
    cp ${LDDOWNL_DIR}/${BOARD}/${BOARD}-${KERNEL_VER}-${XENO}-config ${KBUILD_DIR}/.config
fi

# Enabled used menuconfig for config kernel
if [ "x${MENUCONFIG}" = "xYES" ];then
    make ARCH=arm O=build menuconfig
else
    make ARCH=arm O=build oldconfig
fi

print "Apply mrproper for kernel"
make mrproper
cat ${KBUILD_DIR}/.config

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
sudo cp ${KBUILD_DIR}/.config ${CHROOT_DIR}/boot/config-${KERNEL_BRANCH}-xenomai+
sudo cp ${KBUILD_DIR}/arch/arm/boot/Image ${CHROOT_DIR}/boot/kernel.img

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


# Download, configure and build  kernel for BeagleBone Black
build_kernel_xenomai_bbb() {

KERNEL_URL="https://github.com/RobertCNelson/bb-kernel.git"
KERNEL_PATCH_DIR="${MAKE_DIR}/kernel"
KERNEL_DIR="${KERNEL_PATCH_DIR}/KERNEL"
KBUILD_DIR="${KERNEL_PATCH_DIR}/deploy"
FULL_REBUILD_KERNEL=YES

clean_directory " kernel " "${KERNEL_PATCH_DIR}"

if  [ -f ${CHROOT_DIR}/boot/vmlinuz-*-bone*1 ] && \
    [ -f ${CHROOT_DIR}/usr/local/xenomai/include/xeno_config.h ]; then
    print "kernel and xenomai is already installed in rootfs"
    return 0
fi
print "Start compiled and install kernel for ${BOARD_FULL_NAME}"

# Download kernel
cd ${MAKE_DIR}
if [ ! -f "${KERNEL_PATCH_DIR}/.git/config" ]; then
    if [ ! -f ${LDDOWNL_DIR}/${BOARD}/${KERNEL_BRANCH}.tar.gz ]; then
        print "git clone ${KERNEL_BRANCH}"
        git clone -b ${KERNEL_BRANCH} --depth 1 ${KERNEL_URL} kernel
        if [ ! -f ${KERNEL_PATCH_DIR}/system.sh ] ; then
                cp -v ${KERNEL_PATCH_DIR}/system.sh.sample ${KERNEL_PATCH_DIR}/system.sh
        fi
        sed -i "s:#CC=<enter full path>/bin/arm-none-eabi-:CC=${CC}:" ${KERNEL_PATCH_DIR}/system.sh
        print "set dir source kernel"
        sed -i "s:#LINUX_GIT=/home/user/linux-stable/:LINUX_GIT=${LINUX_KERNEL_SRC_DIR}:" ${KERNEL_PATCH_DIR}/system.sh
        #cd ${KERNEL_PATCH_DIR}
        #/bin/sh -e "${MAKE_DIR}/kernel/scripts/git.sh" || { exit 1 ; }
        #cd ${MAKE_DIR}
        print "create archive ${KERNEL_BRANCH}.tar.gz intro ${LDDOWNL_DIR}/${BOARD}"
        tar -czf ${LDDOWNL_DIR}/${BOARD}/${KERNEL_BRANCH}.tar.gz ./kernel
    else
        print "unpacking ${KERNEL_BRANCH}.tar.gz intro ${MAKE_DIR}"
        tar -xf ${LDDOWNL_DIR}/${BOARD}/${KERNEL_BRANCH}.tar.gz -C ${MAKE_DIR}
    fi
fi

# Xenomai patch kernel and build library
xeno_apply

mkdir -p ${KBUILD_DIR} || true

# Download minimal config
if [ ! -f ${LDDOWNL_DIR}/${BOARD}/${BOARD}-${KERNEL_VER}-${XENO}-config ]; then
    print "Download ${BOARD}-${KERNEL_VER}-${XENO}-config"
    wget -c -P ${LDDOWNL_DIR}/${BOARD} https://cloud.mail.ru/public/95fd0247c068/kernel_config/${BOARD}-${KERNEL_VER}-${XENO}-config
fi

TEST_CONFIG=$(grep "# Linux/arm ${KERNEL_VER} Kernel Configuration" ${LDDOWNL_DIR}/${BOARD}/${BOARD}-${KERNEL_VER}-${XENO}-config || true)
if [ "x${TEST_CONFIG}" = "x" ]; then
    print "Not downloads config kernel: ${BOARD}-${KERNEL_VER}-${XENO}-config"
    rm ${LDDOWNL_DIR}/${BOARD}/${BOARD}-${KERNEL_VER}-${XENO}-config
else
    print "Copy configuration file to KERNEL_DIR"
    cp ${LDDOWNL_DIR}/${BOARD}/${BOARD}-${KERNEL_VER}-${XENO}-config ${KBUILD_DIR}/.config
    sed -i "s:\${DIR}/patches/defconfig:${LDDOWNL_DIR}/${BOARD}/${BOARD}-${KERNEL_VER}-${XENO}-config:" ${KERNEL_PATCH_DIR}/build_kernel.sh
fi

# Enabled used menuconfig for config kernel
if [ "x${MENUCONFIG}" = "xYES" ];then
    sed -i "s:^AUTO_BUILD=1:#AUTO_BUILD=1:" ${KERNEL_PATCH_DIR}/system.sh
else
    sed -i "s:^#AUTO_BUILD=1:AUTO_BUILD=1:" ${KERNEL_PATCH_DIR}/system.sh
    #FIND_STR="^if.*!.*AUTO_BUILD.*; then"
    FIND_STR="^\s*make_menuconfig$"
    INSERT_STR="else cd ${KERNEL_DIR}; make ARCH=arm oldconfig"
    sed -i "/${FIND_STR}/a${INSERT_STR}" ${KERNEL_PATCH_DIR}/build_kernel.sh
fi

# Enabled full rebuild kernel
if [ ${FULL_REBUILD_KERNEL} = YES ];then
    sed -i "s:^unset FULL_REBUILD:FULL_REBUILD=1:" ${KERNEL_PATCH_DIR}/build_kernel.sh
else
    sed -i "s:^FULL_REBUILD=1:unset FULL_REBUILD:" ${KERNEL_PATCH_DIR}/build_kernel.sh
fi

print "Start build kernel"
sed -i "/# Export kernel_version/q" ${KERNEL_PATCH_DIR}/build_kernel.sh
cat >>${KERNEL_PATCH_DIR}/build_kernel.sh << EOF
# Export kernel_version
echo "export kernel_version=\${KERNEL_UTS}" > \${DIR}/.kernel_version
EOF

cd ${KERNEL_PATCH_DIR};
./build_kernel.sh
. ${KERNEL_PATCH_DIR}/.kernel_version
print "End build kernel"

# As a result of execution of the script build_kernel.sh, the variable ${kernel_version} contains the kernel version.
print "Edit kernel_version in /boot/uEnv.txt to ${kernel_version}"
sudo sed -i "s/\(^uname_r=\).*/\1${kernel_version}/"  ${CHROOT_DIR}/boot/uEnv.txt


print "Installing ${kernel_version}-modules.tar.gz to rootfs"
if [ -d "${CHROOT_DIR}/lib/modules/${kernel_version}" ] ; then
        sudo rm -rf "${CHROOT_DIR}/lib/modules/${kernel_version}" || true
fi
sudo tar xfv ${KBUILD_DIR}/${kernel_version}-modules.tar.gz -C ${CHROOT_DIR}/

if [ -f "${KBUILD_DIR}/config-${kernel_version}" ] ; then
        if [ -f "${CHROOT_DIR}/boot/config-${kernel_version}" ] ; then
                sudo rm -f "${CHROOT_DIR}/boot/config-${kernel_version}" || true
        fi
        sudo cp -v "${KBUILD_DIR}/config-${kernel_version}" "${CHROOT_DIR}/boot/config-${kernel_version}"
fi

print "Copy kernel device tree binaries to rootfs"
sudo mkdir -p ${CHROOT_DIR}/boot/dtbs/${kernel_version}/
sudo tar xfv ${MAKE_DIR}/kernel/deploy/${kernel_version}-dtbs.tar.gz -C ${CHROOT_DIR}/boot/dtbs/${kernel_version}/

print "Copy firmware to rootfs"
sudo tar xfv ${MAKE_DIR}/kernel/deploy/${kernel_version}-firmware.tar.gz -C ${CHROOT_DIR}/lib/firmware/
if [ ! -f ${LDDOWNL_DIR}/${BOARD}/linuxdrone-dtbo.tar.gz ]; then
    print "Download linuxdrone-dtbo.tar.gz"
    wget -c -P ${LDDOWNL_DIR}/${BOARD} http://wiki.linuxdrone.org/download/attachments/5275816/linuxdrone-dtbo.tar.gz
fi
sudo tar xfv ${LDDOWNL_DIR}/${BOARD}/linuxdrone-dtbo.tar.gz -C ${CHROOT_DIR}/lib/firmware/

print "Copy kernel Image to rootfs"
sudo cp -v ${MAKE_DIR}/kernel/deploy/${kernel_version}.zImage ${CHROOT_DIR}/boot/vmlinuz-${kernel_version}
sudo cp -v ${MAKE_DIR}/kernel/deploy/config-${kernel_version} ${CHROOT_DIR}/boot/config-${kernel_version} 

print "End build_kernel_xeno2_bbb"
}


# Download, configure and build  kernel for Arietta G25
build_kernel_xenomai_g25() {

KERNEL_DIR="${MAKE_DIR}/kernel"
KBUILD_DIR="${KERNEL_DIR}/build"
KERNEL_CONF="${BOARD}-${KERNEL_VER}-${XENO}-config"
image="zImage"
BUILD="${XENO}"

unset XENO_BOARD_NAME

clean_directory " kernel " "${KERNEL_DIR}"
clean_directory " kernel-patch " "${KERNEL_PATCH_DIR}"

if  [ -f "${CHROOT_DIR}/boot/${KERNEL_VER}-${XENO}.${image}" ] && \
    [ -d "${CHROOT_DIR}/boot/dtbs" ] && \
    [ -f "${CHROOT_DIR}/usr/local/xenomai/include/xeno_config.h" ]; then
    print "kernel and xenomai for Arietta G25 is already installed in rootfs"
    return 0
fi
print "Start compiled_and_install_kernel_g25"

# Download kernel
cd ${MAKE_DIR}
if  [  ! -f "${KERNEL_DIR}/.git/config" ] || \
    [ -f "${KERNEL_DIR}/.git/index.lock" ]; then

    rm -rf ${KERNEL_DIR}/ || true
    if [ ! -f ${LDDOWNL_DIR}/${BOARD}/${KERNEL_BRANCH}.tar.gz ]; then
        print "git clone ${KERNEL_BRANCH}"
        git clone --shared ${LINUX_KERNEL_SRC_DIR} ${KERNEL_DIR}
        cd ${KERNEL_DIR}
        git reset --hard HEAD
        git checkout master -f
        git pull || true
        git tag | grep v${KERNEL_VER} | grep -v rc >/dev/null 2>&1 || print "Not found kernel version v${KERNEL_VER} in repository"
        print "git checkout and create ${KERNEL_BRANCH}"
        if [ ${KERNEL_SHA} ] ; then
            git checkout "v${KERNEL_SHA}" -b ${KERNEL_BRANCH}
        else
            git checkout "v${KERNEL_VER}" -b ${KERNEL_BRANCH}
        fi
        git describe
        print "end clone ${KERNEL_BRANCH}"

        print "create archive ${KERNEL_BRANCH}.tar.gz intro ${LDDOWNL_DIR}/${BOARD}"
        cd ${MAKE_DIR}
        tar -czf ${LDDOWNL_DIR}/${BOARD}/${KERNEL_BRANCH}.tar.gz ./kernel
        #tar -c ./kernel/ | pbzip2 -c -5 -p${CORES} > ${LDDOWNL_DIR}/${BOARD}/${KERNEL_BRANCH}.tar.bz2
        print "end create archive ${KERNEL_BRANCH}.tar.gz"
    else
        rm -Rf ${KERNEL_DIR} || true
        print "unpacking ${KERNEL_BRANCH}.tar.gz intro ${MAKE_DIR}"
        tar -xf ${LDDOWNL_DIR}/${BOARD}/${KERNEL_BRANCH}.tar.gz -C ${MAKE_DIR}
    fi
else
    print "git reset and clean for ${KERNEL_BRANCH}"
    cd ${KERNEL_DIR}
    git reset --hard ${KERNEL_CHA}
    git clean -fdx
    #git pull
    #cd ${MAKE_DIR}; tar -czf ${LDDOWNL_DIR}/${BOARD}/${KERNEL_BRANCH}.tar.gz ./kernel
fi

# Download kernel patch
cd ${MAKE_DIR}
if  [ ! -f "${KERNEL_PATCH_DIR}/.git/config" ] || \
    [ -f "${KERNEL_DIR}/.git/index.lock" ]; then

    rm -rf ${KERNEL_PATCH_DIR}/ || true
    if [ ! -f ${LDDOWNL_DIR}/${BOARD}/${KERNEL_PATCH_BRANCH}.tar.gz ]; then
        print "git clone kernel patch ${KERNEL_PATCH_URL}"
        git clone -b ${KERNEL_PATCH_BRANCH} --depth 1 ${KERNEL_PATCH_URL} kernel-patch
        print "create archive ${KERNEL_PATCH_BRANCH}.tar.gz intro ${LDDOWNL_DIR}/${BOARD}"
        tar -czf ${LDDOWNL_DIR}/${BOARD}/${KERNEL_PATCH_BRANCH}.tar.gz ./kernel-patch
    else
        print "unpacking ${KERNEL_PATCH_BRANCH}.tar.gz intro ${MAKE_DIR}"
        tar -xf ${LDDOWNL_DIR}/${BOARD}/${KERNEL_PATCH_BRANCH}.tar.gz -C ${MAKE_DIR}
    fi
fi

print "Apply kernel pach ${KERNEL_PATCH_URL}"
cd ${KERNEL_DIR}
chmod +x ${KERNEL_PATCH_DIR}/patch.sh
env DIR=${KERNEL_PATCH_DIR} ${KERNEL_PATCH_DIR}/patch.sh


# Xenomai patch kernel and build library
xeno_apply

mkdir -p ${KBUILD_DIR} || true
cd ${KERNEL_DIR}
make mrproper

# Download minimal config
if [ ! -f ${LDDOWNL_DIR}/${BOARD}/${KERNEL_CONF}-config ]; then
    print "Download ${KERNEL_CONF}"
    wget -c -P ${LDDOWNL_DIR}/${BOARD} https://cloud.mail.ru/public/95fd0247c068/kernel_config/${KERNEL_CONF}
    #wget -c -P ${LDDOWNL_DIR}/${BOARD} http://www.acmesystems.it/www/compile_linux_3_16/acme-arietta_defconfig
    #cp ${LDDOWNL_DIR}/${BOARD}/${KERNEL_CONF} ${KERNEL_DIR}/arch/arm/configs/${BOARD}-${KERNEL_VER}-${XENO}_defconfig
fi

#wget -c -P ${LDDOWNL_DIR}/${BOARD} http://www.acmesystems.it/www/compile_linux_3_16/acme-arietta.dts
#cp ${LDDOWNL_DIR}/${BOARD}/acme-arietta.dts ${KERNEL_DIR}/arch/arm/boot/dts/

TEST_CONFIG=$(grep "# Linux/arm .* Kernel Configuration" ${LDDOWNL_DIR}/${BOARD}/${KERNEL_CONF} || true)
TEST_CONFIG_VER=$(echo "${TEST_CONFIG}" | grep -o ${KERNEL_VER} || true)
if [ "x${TEST_CONFIG}" = "x" ]; then
    print "Incorrect configuration of the kernel: ${KERNEL_CONF}"
    rm ${LDDOWNL_DIR}/${BOARD}/${KERNEL_CONF} || true
    exit 1
elif [ "x${TEST_CONFIG_VER}" = "x" ]; then
    print "Loaded version of the kernel configuration differs from the compile, run menuconfig"
    MENUCONFIG=YES
fi

print "Copy configuration file to KERNEL_DIR"
cp ${LDDOWNL_DIR}/${BOARD}/${KERNEL_CONF} ${KBUILD_DIR}/.config

# Enabled used menuconfig for config kernel
if [ "x${MENUCONFIG}" = "xYES" ];then
    make ARCH=arm O=build menuconfig
fi

#make ARCH=arm O=${KBUILD_DIR} CROSS_COMPILE=${CC} -j${CORES} acme-arietta.dtb

print "make ARCH=arm CROSS_COMPILE="${CC}" -j${CORES} LOCALVERSION=-${BUILD} ${image} modules"
make ARCH=arm O=${KBUILD_DIR} CROSS_COMPILE="${CC}" -j${CORES} LOCALVERSION=-${BUILD} ${image} modules

unset DTBS
cat  ${KERNEL_DIR}/arch/arm/Makefile | grep "dtbs:" >/dev/null 2>&1 && DTBS=enable

unset has_dtbs_install
if [ "x${DTBS}" = "x" ] ; then
    cat  ${KERNEL_DIR}/arch/arm/Makefile | grep "dtbs dtbs_install:" >/dev/null 2>&1 && DTBS=enable
    if [ "x${DTBS}" = "xenable" ] ; then
        has_dtbs_install=enable
    fi
fi

if [ "x${DTBS}" = "xenable" ] ; then
    print "make ARCH=arm CROSS_COMPILE="${CC}" -j${CORES} LOCALVERSION=-${BUILD} dtbs"
    make ARCH=arm O=${KBUILD_DIR} CROSS_COMPILE="${CC}" -j${CORES} LOCALVERSION=-${BUILD} dtbs
    ls arch/arm/boot/* | grep dtb >/dev/null 2>&1 || unset DTBS
fi

KERNEL_UTS=$(cat ${KBUILD_DIR}/include/generated/utsrelease.h | awk '{print $3}' | sed 's/\"//g' )

# Remove old kernel modules
if [ -d "${CHROOT_DIR}/lib/modules/${KERNEL_UTS}" ] ; then
    sudo rm -rf "${CHROOT_DIR}/lib/modules/${KERNEL_UTS}" || true
fi
# Remove old kernel config in rootfs
sudo rm -f "${CHROOT_DIR}/boot/${KERNEL_CONF}" || true

# Install modules
make -s ARCH=arm O=${KBUILD_DIR} INSTALL_MOD_PATH=dist modules_install -j${CORES}
# Install firmware
make -s ARCH=arm O=${KBUILD_DIR} INSTALL_FW_PATH=dist/lib/firmware firmware_install -j${CORES}
# Install headers
make -s ARCH=arm O=${KBUILD_DIR} INSTALL_HDR_PATH=dist/usr headers_install -j${CORES}
find ${KBUILD_DIR}/dist/usr/include \( -name .install -o -name ..install.cmd \) -delete
# Install dtbs
if [ "x${has_dtbs_install}" = "xenable" ] ; then
    make -s ARCH=arm O=${KBUILD_DIR} LOCALVERSION=-${BUILD} CROSS_COMPILE="${CC}" dtbs_install INSTALL_DTBS_PATH=dist/boot/dtbs/${KERNEL_UTS}
else
    mkdir -p ${KBUILD_DIR}/dist/boot/dtbs/${KERNEL_UTS}/
    find ${KBUILD_DIR}/arch/arm/boot/ -iname "*.dtb" -exec cp -v '{}' ${KBUILD_DIR}/dist/boot/dtbs/${KERNEL_UTS}/ \;
fi
# Install kernel, config
sudo cp ${KBUILD_DIR}/.config ${KBUILD_DIR}/dist/boot/${KERNEL_CONF}
sudo cp ${KBUILD_DIR}/arch/arm/boot/${image} ${KBUILD_DIR}/dist/boot/${KERNEL_UTS}.${image}

print "Copy kernel, modules, dtbs, firmware, and headers to rootfs"
sudo cp -rf ${KBUILD_DIR}/dist/* ${CHROOT_DIR}/

print "Edit kernel_version in /boot/uEnv.txt to uname_r=${KERNEL_UTS}.${image}"
sudo sed -i "s/\(^uname_r=\).*/\1${KERNEL_UTS}.${image}/"  ${CHROOT_DIR}/boot/uEnv.txt

print "End build_kernel_xeno2_g25"
}


# Compiled_and_install_nodejs for rpi
compiled_and_install_nodejs_npm() {
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

make clean

env \
CPP="${CC}gcc -E" \
STRIP="${CC}strip" \
OBJCOPY="${CC}objcopy" \
AR="${CC}ar" \
RANLIB="${CC}ranlib" \
LD="${CC}ld" \
OBJDUMP="${CC}objdump" \
CC="${CC}gcc" \
CXX="${CC}g++" \
NM="${CC}nm" \
AS="${CC}as" \
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

clean_directory " rootfs " "${CHROOT_DIR}"

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

if [ ! -f ${BOARD_DIR}/rootfs-${BOARD}-${DISTR}.tar.bz2 ]; then
    cd ${MAKE_DIR}
    print "Start created archive rootfs-${BOARD}-${DISTR}.tar.bz2"
    sudo tar -c ./rootfs/ | pbzip2 -c -5 -p${CORES} > ${BOARD_DIR}/rootfs-${BOARD}-${DISTR}.tar.bz2
    print "End created archive rootfs-${BOARD}-${DISTR}.tar.bz2 into ${BOARD_DIR}"
fi

print "End debootstrap for ${BOARD_FULL_NAME}"
return 0
}


# Copy truncated rootfs
copy_truncate_rootfs() {
print "Start copy truncated rootfs in ${ROOTFS_TRUNC_DIR}"

run_cmd_chroot "mkdir -p /home/ld/code/rootfs || true"

# Move rootfs trunc to chroot directory
if [ -d ${ROOTFS_TRUNC_DIR} ]; then
    sudo mv ${ROOTFS_TRUNC_DIR} ${CHROOT_DIR}/home/ld/code
fi

run_cmd_chroot "rsync -rL --delete --ignore-errors --exclude="/usr/local/xenomai" /usr /home/ld/code/rootfs/ || true"
run_cmd_chroot "cp -R /usr/local/xenomai /home/ld/code/rootfs/usr/local/"
sudo chown -R $(whoami):$(whoami) ${CHROOT_DIR}/home/ld/code
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

# Default settings variable
START_SHELL=NO
DEV_DISK="/dev/sdX"
DISK_IMAGE_CLEAN=NO
XENO=xeno2
unset UBOOT_USE

# Разбор параметров командной строки
while getopts hvmsb:d:c:x:r: OPT; do
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
            CLEAN="$OPTARG"
            ;;
        r)
            REBUILD="$OPTARG"
            ;;
        x)
            XENO=$OPTARG
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

if [ ! ${BOARD} ]; then
    echo "Not specified board type"
    echo $USAGE
    exit 1
fi

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
PASM_DIR=${BOARD_DIR}/pasm
MAKE_DIR=${BOARD_DIR}/make_dir
CHROOT_DIR=${MAKE_DIR}/rootfs
ROOTFS_TRUNC_DIR=${BOARD_DIR}/rootfs
LOCALES=${LANG}
CORES=$(grep "^cpu cores" /proc/cpuinfo | awk -F : '{print $2}' | head -1 | sed 's/^[ ]*//')
CORES=$((${CORES} + 1))
DISK_SIZE=2048
LINUX_KERNEL_SRC_DIR=${LDTOOLS_DIR}/linux-src

if [ ! -d ${LDDOWNL_DIR}/${BOARD} ]; then
    print "Create a directory where all downloads soft"
    mkdir -p ${LDDOWNL_DIR}/${BOARD}
fi

if [ ! -d ${MAKE_DIR} ]; then
    print "Create a directory for build packages"
    mkdir -p ${MAKE_DIR}
fi

umount_proc_sysfs_devpts
sudo_timestamp_timeout 800
install_common_software_the_host

case "${BOARD}" in
    bbb)
        BOARD_FULL_NAME="BeagleBone Black"
        DISTR=trusty
        #DISTR=saucy
        DISTR_MIRROR=http://ports.ubuntu.com/ubuntu-ports/
        PKG_LIST_BOARD="mongodb nodejs npm"

        ARCH=armhf
        MARCH=armv7-a
        MFPU=vfp3

        KERNEL_BRANCH="am33x-v3.8"
        KERNEL_VER="3.8.13"
        #KERNEL_BRANCH="am33x-v3.14"
        #KERNEL_VER="3.14.17"

        UBOOT_BRANCH="v2014.10"
        UBOOT_PATCH_NAME="0001-am335x_evm-uEnv.txt-bootz-n-fixes.patch"
        UBOOT_PATCH_URL=https://raw.githubusercontent.com/eewiki/u-boot-patches/master/${UBOOT_BRANCH}/${UBOOT_PATCH_NAME}
        UBOOT_DEFCONFIG="am335x_evm_config"
        UBOOT_FILES="MLO u-boot.img"

        NAME_LIBWEB=libwebsockets-1.3-chrome37-firefox30.tar.gz
        NAME_MONGOC=0.94.2

        download_install_crosscompiler_bbb
        download_install_pasm
        debootstrap
        #rootfs_installing_packages
        if [ ${START_SHELL} = YES ]; then
            start_shell_in_chroot
        fi
        compiled_and_install_uboot
        build_kernel_xenomai_bbb
        ;;

    rpi)
        BOARD_FULL_NAME="Raspberry PI"
        DISTR=wheezy
        DISTR_MIRROR=http://archive.raspbian.org/raspbian/
        #DISTR_MIRROR=http://mirrors-ru.go-parts.com/raspbian/
        #DISTR_MIRROR=http://mirror.netcologne.de/raspbian/raspbian/
        #DISTR=jessie
        #DISTR_MIRROR=http://mirrordirector.raspbian.org/raspbian
        PKG_LIST_BOARD=""

        ARCH=armhf
        MARCH=armv6 #j
        MFPU=vfp
        MCPU=arm1176jzf-s
        MFLOAT_ABI=hard

        KERNEL_BRANCH="rpi-3.8.y"
        KERNEL_VER="3.8.13"
        #KERNEL_BRANCH="rpi-3.14.y"
        #KERNEL_VER="3.14.17"

        NAME_LIBWEB=libwebsockets-1.3-chrome37-firefox30.tar.gz
        NAME_MONGOC=0.94.2

        download_install_crosscompiler_rpi
        debootstrap
        #rootfs_installing_packages
        if [ ${START_SHELL} = YES ]; then
            start_shell_in_chroot
        fi
        build_kernel_xenomai_rpi
        compiled_and_install_mongodb_in_chroot
        compiled_and_install_nodejs_npm
        ;;

    g25)
        BOARD_FULL_NAME="Arietta G25"
        DISTR=wheezy
        #DISTR_MIRROR=http://cdn.debian.net/debian/
        DISTR_MIRROR=http://http.debian.net/debian/
        PKG_LIST_BOARD=""

        ARCH=armel
        MARCH=armv5

        KERNEL_VER="3.14.17"
        KERNEL_BRANCH="v${KERNEL_VER}-${BOARD}"
        KERNEL_PATCH_URL="https://github.com/RobertCNelson/armv5_devel.git"
        KERNEL_PATCH_DIR="${MAKE_DIR}/kernel-patch"
        KERNEL_PATCH_BRANCH="v3.14.x-at91"

        AT91BS_GIT="git://github.com/linux4sam/at91bootstrap.git"
        AT91BS_BRANCH="496c083"
        AT91BS_DEFCONFIG="at91sam9x5eksd_uboot_defconfig"
        AT91BS_FILES="at91sam9x5ek-sdcardboot-uboot-*.bin"
        #AT91BS_GIT="git://github.com/tanzilli/at91bootstrap.git"
        #AT91BS_DEFCONFIG="arietta-256m_defconfig"
        #AT91BS_FILES="at91sam9x5_arietta-sdcardboot-linux-zimage-dt-*.bin"

        UBOOT_BRANCH="v2014.10"
        UBOOT_PATCH_NAME="0001-at91sam9x5ek-uEnv.txt-bootz-n-fixes.patch"
        UBOOT_PATCH_URL=https://raw.githubusercontent.com/eewiki/u-boot-patches/master/${UBOOT_BRANCH}/${UBOOT_PATCH_NAME}
        UBOOT_DEFCONFIG="at91sam9x5ek_mmc_defconfig"
        UBOOT_FILES="u-boot.bin"


        NAME_LIBWEB=libwebsockets-1.3-chrome37-firefox30.tar.gz
        NAME_MONGOC=0.94.2

        download_install_crosscompiler_g25
        debootstrap
        #rootfs_installing_packages
        if [ ${START_SHELL} = YES ]; then
            start_shell_in_chroot
        fi
        compiled_and_install_at91bootstrap
        compiled_and_install_uboot
        build_kernel_xenomai_g25
        compiled_and_install_mongodb_in_chroot
        compiled_and_install_nodejs_npm
        ;;
    *)
        print "Unknown name board"
        echo ${USAGE}
        exit 1
        ;;
esac

compiled_and_install_libwebsockets
compiled_and_install_mongoc
copy_truncate_rootfs
build_install_linuxdrone
create_image

umount_proc_sysfs_devpts
sudo_timestamp_timeout 10

exit 0
