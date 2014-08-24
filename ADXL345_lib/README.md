Libreria ADXL345
================
Il dispositivo è un accelerometro I2C disponibile da tempo sul mercato.

In testa al sorgente ho inserito questo commento che può tornare utile:
```
/*
http://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/tree/Documentation/i2c/dev-interface
apt-get install i2c-tools
apt-get install libi2c-dev
*/
```

Il dispositivo risponde all'indirizzo:
```
#define I2C_ADDR (0x53) // ADXL345 device address
```


e sulla mia AriettaG25 è collegato alla I2C-1
```
  fd = open("/dev/i2c-1", O_RDWR);
  if (fd < 0) {
    printf("Error opening file: %s\n", strerror(errno));
    return 1;
  }
  if (ioctl(fd, I2C_SLAVE, I2C_ADDR) < 0) {
    printf("ioctl error: %s\n", strerror(errno));
    return 1;
  }
```

A questo punto il dispositivo si può <i>leggere e scrivere</i> con questo codice:
```
// Writes val to address register on device
int ADXL345_writeTo( int file, char address, char val)
{
char _buff[2];
_buff[0] = address;
_buff[1] = val;
if ( write(file, _buff, 2) != 2)
return 1;
return 0;
}
// Reads num bytes starting from address register on device in to _buff array
int ADXL345_readFrom(int file, char address, int num, char buff[])
{
char _buff[1];
_buff[0] = address;
if ( write(file, _buff, 1) != 1)
return 1;
if ( read( file, buff, num) != num)
return 1;
return 0;
}
```

