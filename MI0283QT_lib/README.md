LCD grafico 320x240 MI0283QT
============================

Questo LCD grafico a colori, della risoluzione di 320x240 pixel, si può trovare presso la Watterott [1]

Il dispositivo viene visto come periferica SPI usando il device spidev.
In questo caso, per poter visualizzare dei filmati, realizzati come sequenze di immagini, ho dovuto ricompilare il driver spidev per ampliare il buffer dai 4KB prefissati a 153600 Byte, che corrispondono ad una immagine video RGB565.

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
```
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

[1] www.watterott.com/de/MI0283QT-2-Adapter
