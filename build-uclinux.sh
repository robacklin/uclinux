#!/bin/sh

ms_git_pull() {
	echo "dir:$2 repo:$1"
	if [ -d "$2" ] ; then
		cd "$2"
		git pull
		cd ..
	else
		git clone $1 $2
	fi
}

ms_find_replace_gnueabi() {
	grep -R "arm-linux-" * | egrep -v "Documentation"  > /tmp/arm-linux-log.txt
	cat /tmp/arm-linux-log.txt | gawk -F':' '{print $1}' | sort -u > /tmp/arm-linux-gnueabi-log.txt
	for x in `cat /tmp/arm-linux-gnueabi-log.txt` ; do sed -i 's,arm-linux-,arm-linux-gnueabi-,g' "$x" ; done
}

#ms_git_pull git://github.com/robacklin/uclinux-freeswan.git freeswan
#ms_git_pull git://github.com/robacklin/uclinux-glibc.git glibc
ms_git_pull git://github.com/robacklin/uclinux-linux-2.0.x.git linux-2.0.x
ms_git_pull git://github.com/robacklin/uclinux-linux-2.4.x.git linux-2.4.x
ms_git_pull git://github.com/robacklin/uclinux-linux-3.x.git linux-3.x
#ms_git_pull git://github.com/robacklin/uclinux-openswan.git openswan
ms_git_pull git://github.com/robacklin/uclinux-uclibc.git uClib
ms_git_pull git://github.com/robacklin/uclinux-users.git user
ms_git_pull git://github.com/robacklin/moonshine.git moonshine

cp -f def_config .config

sh ./moonshine/scripts/build-uclinux.sh

