#!/bin/bash

if [ $# -ne 3 ]; then
  echo "usage: $0 <weather.txt> <weater.png> <final_image.rgb>"
  echo "       merge a text file, an icone 64x64 into a final RGB image 131x131"
  exit 1
fi

/bin/rm /tmp/tmpdata/backgrond.png &> /dev/null
/bin/rm /tmp/tmpdata/weatherimg.png &> /dev/null

# Costruisco l'immagine di backgound
convert -size 131x131 xc:lightblue /tmp/tmpdata/backgrond.png
# Creo una immagine del testo
convert -background lightblue  -fill blue -pointsize 12 -size 131x62 caption:@$1 -depth 8 -flip /tmp/tmpdata/weathertxt.png
# Compongo l'immagine del meteo nell'immagine del background
composite -geometry +32+64 $2 /tmp/tmpdata/backgrond.png /tmp/tmpdata/weatherimg.png
# Metto insieme l'immagine del testo con quella del meteo composta precedentemente
composite -geometry +0+0 /tmp/tmpdata/weathertxt.png /tmp/tmpdata/weatherimg.png $3

# La visualizzo
# ./NokiaLCD_lib_tst -s /tmp/tmpdata/weatherimg.rgb

exit 0
