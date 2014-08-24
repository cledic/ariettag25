Libreria ADXL345
================
Il dispositivo è un accelerometro I2C disponibile da tempo sul mercato.

In testa al sorgente ho inserito questo commento che può tornare utile:
<code>
/*
http://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/tree/Documentation/i2c/dev-interface
apt-get install i2c-tools
apt-get install libi2c-dev
*/
</code>

Il dispositivo risponde all'indirizzo:
<code>
#define I2C_ADDR (0x53) // ADXL345 device address
</code>

e sulla mia AriettaG25 è collegato alla I2C-0
<code>
  fd = open("/dev/i2c-1", O_RDWR);
  if (fd < 0) {
    printf("Error opening file: %s\n", strerror(errno));
    return 1;
  }
  if (ioctl(fd, I2C_SLAVE, I2C_ADDR) < 0) {
    printf("ioctl error: %s\n", strerror(errno));
    return 1;
  }
</code>
