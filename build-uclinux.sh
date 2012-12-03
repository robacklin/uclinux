#!bin/sh

fail_not_exist() {
	if [ ! -r "$1" ] ; then
		echo "not found $1"
		exit 1
	fi
}

#if [ ! -d "uClinux-dist" ] ; then
#	wget http://www.uclinux.org/pub/uClinux/dist/uClinux-dist-20120620.tar.bz2
#	tar jxvf uClinux-dist-20120620.tar.bz2
#fi

if [ ! -d "arm-2009q1" ] ; then
	wget http://www.ackatech.net/downloads/arm-2009q1.tar.bz2
	fail_not_exist arm-2009q1.tar.bz2
	tar xvjf arm-2009q1.tar.bz2
fi

export ARCH=arm
export CROSS_COMPILE=`pwd`/arm-2009q1/bin/arm-none-linux-gnueabi-


#cd uClinux-dist

cd linux-3.x
make mrproper
#make menuconfig
#make O=/home/racklin/Projects/builds/uclinux/linux dep
#make 
