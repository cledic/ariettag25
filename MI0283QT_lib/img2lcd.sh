#!/bin/bash

################################################################################
# check if required tool exists
RES="$(which convert)"
if [ $? -ne 0 ]; then
  echo "please install convert"
  exit 1
fi

sync

RES="$(convert $1 -resize x240 -resize '320x<' -gravity center -crop 320x240+0+0 +repage /ramdisk/lcdimg.rgb)"
if [ $? -eq 0 ]; then
  /root/photoframe/MI0283QT_lcd -n -f /ramdisk/lcdimg.rgb
fi

rm $1 /ramdisk/lcdimg.rgb

exit 0
