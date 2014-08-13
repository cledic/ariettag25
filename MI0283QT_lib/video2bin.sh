# video2bin.sh <movie.avi>
# create a sequence of rgb images from the first 10 seconds video
# save the sequence file as movie.bin
#
mplayer -endpos 10 -nosound -vo png:z=0 $1
mogrify -resize 320x240 *.png
mogrify -format rgb *.png
cat *.rgb > movie.bin
