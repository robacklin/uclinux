#!/bin/sh

if [ -d moonshine ] ; then
	cd moonshine ; git pull
	cd ..
else
	git clone git://github.com/robacklin/moonshine.git
fi


sh ./moonshine/scripts/build-uclinux.sh

