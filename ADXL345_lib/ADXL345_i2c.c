#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <errno.h>

/*
		http://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/tree/Documentation/i2c/dev-interface
        apt-get install i2c-tools
        apt-get install libi2c-dev
*/

/* ------- Register names ------- */
#define ADXL345_DEVID 0x00
#define ADXL345_RESERVED1 0x01
#define ADXL345_THRESH_TAP 0x1d
#define ADXL345_OFSX 0x1e
#define ADXL345_OFSY 0x1f
#define ADXL345_OFSZ 0x20
#define ADXL345_DUR 0x21
#define ADXL345_LATENT 0x22
#define ADXL345_WINDOW 0x23
#define ADXL345_THRESH_ACT 0x24
#define ADXL345_THRESH_INACT 0x25
#define ADXL345_TIME_INACT 0x26
#define ADXL345_ACT_INACT_CTL 0x27
#define ADXL345_THRESH_FF 0x28
#define ADXL345_TIME_FF 0x29
#define ADXL345_TAP_AXES 0x2a
#define ADXL345_ACT_TAP_STATUS 0x2b
#define ADXL345_BW_RATE 0x2c
#define ADXL345_POWER_CTL 0x2d
#define ADXL345_INT_ENABLE 0x2e
#define ADXL345_INT_MAP 0x2f
#define ADXL345_INT_SOURCE 0x30
#define ADXL345_DATA_FORMAT 0x31
#define ADXL345_DATAX0 0x32
#define ADXL345_DATAX1 0x33
#define ADXL345_DATAY0 0x34
#define ADXL345_DATAY1 0x35
#define ADXL345_DATAZ0 0x36
#define ADXL345_DATAZ1 0x37
#define ADXL345_FIFO_CTL 0x38
#define ADXL345_FIFO_STATUS 0x39

#define ADXL345_BW_1600 0xF // 1111
#define ADXL345_BW_800  0xE // 1110
#define ADXL345_BW_400  0xD // 1101
#define ADXL345_BW_200  0xC // 1100
#define ADXL345_BW_100  0xB // 1011
#define ADXL345_BW_50   0xA // 1010
#define ADXL345_BW_25   0x9 // 1001
#define ADXL345_BW_12   0x8 // 1000
#define ADXL345_BW_6    0x7 // 0111
#define ADXL345_BW_3    0x6 // 0110

/*
 Interrupt bit position
 */
#define ADXL345_INT_DATA_READY_BIT 0x07
#define ADXL345_INT_SINGLE_TAP_BIT 0x06
#define ADXL345_INT_DOUBLE_TAP_BIT 0x05
#define ADXL345_INT_ACTIVITY_BIT   0x04
#define ADXL345_INT_INACTIVITY_BIT 0x03
#define ADXL345_INT_FREE_FALL_BIT  0x02
#define ADXL345_INT_WATERMARK_BIT  0x01
#define ADXL345_INT_OVERRUNY_BIT   0x00

#define ADXL345_OK    1 // no error
#define ADXL345_ERROR 0 // indicates error is predent

#define ADXL345_NO_ERROR   0 // initial state
#define ADXL345_READ_ERROR 1 // problem reading accel
#define ADXL345_BAD_ARG    2 // bad method argument

#define I2C_ADDR (0x53)    // ADXL345 device address
#define TO_READ (6)      // num of bytes we are going to read each time (two bytes for each axis)

int ADXL345_writeTo( int file, char address, char val);
int ADXL345_readFrom( int file, char address, int num, char buff[]);
void ADXL345_powerOn( int file);
void ADXL345_readAccel( int file, int *x, int *y, int *z);

// #define I2C_ADDR 0x53

int main (void) {
        char buffer[1];
        int fd;
        int x, y, z;

        fd = open("/dev/i2c-1", O_RDWR);

        if (fd < 0) {
                printf("Error opening file: %s\n", strerror(errno));
                return 1;
        }

        if (ioctl(fd, I2C_SLAVE, I2C_ADDR) < 0) {
                printf("ioctl error: %s\n", strerror(errno));
                return 1;
        }

        ADXL345_powerOn( fd);
        ADXL345_readAccel( fd, &x, &y, &z);

        printf("Values: X: %d\tY: %d\tZ: %d\n", x, y, z);

        return 0;
}

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

void ADXL345_powerOn( int file) 
{

  //Turning on the ADXL345
  ADXL345_writeTo( file, ADXL345_POWER_CTL, 0);
  ADXL345_writeTo( file, ADXL345_POWER_CTL, 16);
  ADXL345_writeTo( file, ADXL345_POWER_CTL, 8);
}

void ADXL345_readAccel(int file, int *x, int *y, int *z) {
  char buff[6];

  ADXL345_readFrom( file, ADXL345_DATAX0, TO_READ, buff); //read the acceleration data from the ADXL345

  // each axis reading comes in 10 bit resolution, ie 2 bytes.  Least Significat Byte first!!
  // thus we are converting both bytes in to one int
  *x = (((int)buff[1]) << 8) | buff[0];
  *y = (((int)buff[3]) << 8) | buff[2];
  *z = (((int)buff[5]) << 8) | buff[4];
}


