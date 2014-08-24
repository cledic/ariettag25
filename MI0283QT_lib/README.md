LCD grafico 320x240 MI0283QT
============================

Questo LCD grafico a colori, della risoluzione di 320x240 pixel, si può trovare presso la Watterott [1]

Il dispositivo viene visto come periferica SPI usando il device spidev.
In questo caso, per poter visualizzare dei filmati, realizzati come sequenze di immagini, ho dovuto ricompilare il driver spidev per ampliare il buffer dai 4KB prefissati a 153600 Byte, che corrispondono ad una immagine video RGB565.
A questo link ho inserito una descrizione dei passi per procedere alla modifica [2].


```C++
	/* Variabili alterabili da cmdline dall'utente */
  static const char *device = "/dev/spidev32766.0"
  
	fd = open(device, O_FSYNC|O_RDWR);
  if (fd < 0)
    pabort("can't open device");
  
  //
  mode |= SPI_CPHA;
  mode |= SPI_CPOL;
  
  ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
  if (ret == -1)
    pabort("can't set spi mode");

  ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
  if (ret == -1)
    pabort("can't get spi mode");
```

Riporto il codice che uso per dialogare con il dispositivo:
```C++
void lcd_cmd(unsigned char reg, unsigned char param)
{
  int ret;
  unsigned char cmd[2];
  
  cmd[0]=LCD_REGISTER;
  cmd[1]=reg;
  struct spi_ioc_transfer tr = {
    .tx_buf = (unsigned long)cmd,
    .rx_buf = (unsigned long)NULL,
    .len = ARRAY_SIZE( cmd),
    .delay_usecs = 0,
  };
  ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
  usleep( 50);
  
  if (ret < 1)
    pabort("can't send lcd_cmd::register message");

  cmd[0]=LCD_DATA;
  cmd[1]=param;
  struct spi_ioc_transfer tr2 = {
    .tx_buf = (unsigned long)cmd,
    .rx_buf = (unsigned long)NULL,
    .len = ARRAY_SIZE( cmd),
    .delay_usecs = 0,
  };
  ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr2);
  usleep( 50);

  if (ret < 1)
    pabort("can't send lcd_cmd::data message");

  return;
}
```
Una volta messe a punto queste primitive, il dialogo con il display avviene con costrutti più "elevati" e quindi più semplici.

Il programma permette di visualizzare immagini a full screen, o in paritcolari punti dello schermo. Permette di leggere file "binari" e visualizzarli ciclicamente secondo un framerate a default o impostabile via linea di comando.

Nel folder ci sono alcuni dei comandi che uso per convertire immagini nel formato RGB 24bit delle dimensioni 320x240.

E ci sono anche comandi per estrapolare pezzi di filmato e renderli poi file binari sempre in RGB 24bit da visualizzare in sequenza.

Questo script prende come unico argomento l'immagine da convertire. Crea un file RGB24 delle "esatte" dimensioni di 320x240.
```
# img2rgb <img.png> <img.rgb>
convert $1 -resize x240 -resize '320x<' -gravity center -crop 320x240+0+0 +repage $2
```

Questo script invece converte e visualizza l'immagine. Attenzione perché al termine cancella anche l'immagine originale.
```bash
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
```

Questo è un esempio di come costruire un file "binario" contenente più immagini RGB24bit da visualizzare in sequenza.
```bash
# video2bin.sh <movie.avi>
# create a sequence of rgb images from the first 10 seconds video
# save the sequence file as movie.bin
#
mplayer -endpos 10 -nosound -vo png:z=0 $1
mogrify -resize 320x240 *.png
mogrify -format rgb *.png
cat *.rgb > movie.bin
```
Adesso è il caso di vedere il video. Questo è il comando per visualizzare un filmato 320x240.
```bash
./MI0283QT_lcd -n -m ./perla.bin -t 20
```

Questa volta ho una immagine fissa in basso allo schermo, ed un video 320x110 in alto. Purtroppo non ho ancora fatto un video, ma si tratta di un esempio di stazione meteo, in cui nella parte alta un filmato riproduce il tempo in atto, e nella parte bassa c'è indicata la temperatura, la pressione, ecc.ecc.
```bash
 ./MI0283QT_lcd -n -s ./autunno_screen.rgb -m ./autunno2.bin -l 4 -w 320 -h 110
```

[1] www.watterott.com/de/MI0283QT-2-Adapter<br>
[2] https://code.google.com/p/foxg20-wonderland/wiki/PageName
