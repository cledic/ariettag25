AriettaNews
===========

Questo programma visualizza, sullo schermo Nokia6110, in un ciclo continuo, le news dell'ANSA e le indicazioni meteo 
del sito WeatherChannel.

Il testo viene convertito in immagine e l'immagine corrispondete al meteo viene anchessa montata su di una immagine 130x130
con in calce le indicazioni di temperatura e altre previsioni.

Questo display è pilotato in SPI, usando il device spidev. Ma, ha la prticolarietà di usare una lunghezza di 9bit di frame.
Il primo bit infatti viene usato per indicare all'LCD se a seguire sono comandi o dati.

```C++
//
static int fd;
static unsigned char bits = 9;
static unsigned int mode;
static unsigned int speed = 9*1000*1000;

void InitLcd(unsigned char type, const unsigned char* device)
{
  unsigned char i;
  int ret;
  _type = type;
  
  LCDReset();
  
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
  
  ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
  if (ret == -1)
    pabort("can't set bits per word");
  
  ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
  if (ret == -1)
    pabort("can't get bits per word");
  
  ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
  if (ret == -1)
    pabort("can't set speed");
  
  ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
  if (ret == -1)
    pabort("can't get speed");

```
