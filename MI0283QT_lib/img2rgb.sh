# img2rgb <img.png> <img.rgb>
convert $1 -resize x240 -resize '320x<' -gravity center  -crop 320x240+0+0 +repage $2
