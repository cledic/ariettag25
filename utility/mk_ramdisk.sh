#!/bin/sh

echo -n "Mounting RAMDisk... "

grep /tmp/tmpdata /etc/mtab > /dev/null
if [ $? -eq 0 ]; then
  echo "already mounted..."
  exit
fi

if [ ! -e /tmp/tmpdata ]; then
  mkdir /tmp/tmpdata
fi

#
mount -t tmpfs -o size=8M,mode=0744 tmpfs /tmp/tmpdata/
chmod 777 /tmp/tmpdata/ -R

echo "done!"
