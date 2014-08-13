/*

*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <math.h>
 
#include "BMP180_lib.h"

#define wait_ms( x)		usleep( x*1000);

/** Perform temperature measurement
 *  
 * @returns
 *   temperature (C)
 */    
int BMP180_ReadRawTemperature(long* pUt);

/** Perform pressure measurement 
 *  
 * @returns
 *   temperature (C)
 */    
int BMP180_ReadRawPressure(long* pUp);

/** Calculation of the temperature from the digital output
 */    
float BMP180_TrueTemperature(long ut);

/** Calculation of the pressure from the digital output
 */    
float BMP180_TruePressure(long up);

int m_oss;
float m_temperature;     
float m_pressure;
float m_altitude;

short ac1, ac2, ac3; 
unsigned short ac4, ac5, ac6;
short b1, b2;
short mb, mc, md;
long x1, x2, x3, b3, b5, b6;
unsigned long b4, b7;

// #define BMP180_TEST_FORMULA

/* file handle for I2C device */
static int fd;
/* flag for device succesfully opened */
static int deviceReady=0;
   
int BMP180_Init( const char *device, float altitude, int overSamplingSetting)
{
    char data[22];
    int errors = 0;
	
    m_altitude = altitude;
    m_oss = overSamplingSetting; 
    m_temperature = UNSET_BMP180_TEMPERATURE_VALUE;
    m_pressure = UNSET_BMP180_PRESSURE_VALUE;  

    fd = open( device, O_RDWR);

    if ( fd < 0) {
        printf("Error opening file: %s\n", strerror(errno));
        return 1;
    }

    if ( ioctl(fd, I2C_SLAVE, I2C_ADDR) < 0) {
        printf("ioctl error: %s\n", strerror(errno));
        return 1;
    }

    deviceReady=1;

    
    // read calibration data
    errors = BMP180_Read(0xAA, 22, data); // read 11 x 16 bits at this position 
    wait_ms(10);
    
    // store calibration data for further calculus  
    ac1 = data[0]  << 8 | data[1];
    ac2 = data[2]  << 8 | data[3];
    ac3 = data[4]  << 8 | data[5];
    ac4 = data[6]  << 8 | data[7];
    ac5 = data[8]  << 8 | data[9];
    ac6 = data[10] << 8 | data[11];
    b1  = data[12] << 8 | data[13];
    b2  = data[14] << 8 | data[15];
    mb  = data[16] << 8 | data[17];
    mc  = data[18] << 8 | data[19];
    md  = data[20] << 8 | data[21];
 
#ifdef BMP180_TEST_FORMULA
    ac1 = 408;
    ac2 = -72;
    ac3 = -14383;
    ac4 = 32741;
    ac5 = 32757;
    ac6 = 23153;
    b1 = 6190;
    b2 = 4;
    mb = -32768;
    mc = -8711;
    md = 2868;
    m_oss = 0;
    errors = 0;
#endif // #ifdef BMP180_TEST_FORMULA
 
    return errors? 0 : 1;
}
 
int BMP180_ReadValues(float* pTemperature, float* pPressure)
{
    long t, p;
 
    if (!BMP180_ReadRawTemperature(&t) || !BMP180_ReadRawPressure(&p))
    {
        m_temperature = UNSET_BMP180_TEMPERATURE_VALUE;
        m_pressure = UNSET_BMP180_PRESSURE_VALUE;  
        return 0;
    }
 
    m_temperature = BMP180_TrueTemperature(t);
    m_pressure = BMP180_TruePressure(p);
 
    if (pPressure)
        *pPressure = m_pressure;
    if (pTemperature)
        *pTemperature = m_temperature;
 
    return 1;
}
 
int BMP180_ReadRawTemperature(long* pUt)
{
    int errors = 0;
    char val;
    char data[2];
    
    // request temperature measurement
    val = 0x2E;
    errors = BMP180_Write(0xF4, val); // write 0XF2 into reg 0XF4
 
    wait_ms( 5);
 
    // read raw temperature data
    errors += BMP180_Read(0xF6, 2, data);  // get 16 bits at this position 
    
#ifdef BMP180_TEST_FORMULA
    errors = 0;
#endif // #ifdef BMP180_TEST_FORMULA
 
    if (errors)
        return 0;
    else
        *pUt = data[0] << 8 | data[1];
 
#ifdef BMP180_TEST_FORMULA
    *pUt = 27898;
#endif // #ifdef BMP180_TEST_FORMULA
    
    return 1;
}
 
int BMP180_ReadRawPressure(long* pUp)
{
    int errors = 0;
    char val;
    char data[2];
	
    // request pressure measurement
    val = 0x34 + (m_oss << 6);
    errors = BMP180_Write( 0xF4, val); // write 0x34 + (m_oss << 6) into reg 0XF4
 
    switch (m_oss)
    {
        case BMP180_OSS_ULTRA_LOW_POWER:        wait_ms(4.5); break;
        case BMP180_OSS_NORMAL:                 wait_ms(7.5); break;
        case BMP180_OSS_HIGH_RESOLUTION:        wait_ms(13.5); break;
        case BMP180_OSS_ULTRA_HIGH_RESOLUTION:  wait_ms(25.5); break;
    }
 
    // read raw pressure data
    errors += BMP180_Read( 0xF6, 2, data);  // get 16 bits at this position     
    
#ifdef BMP180_TEST_FORMULA
    errors = 0;
#endif // #ifdef BMP180_TEST_FORMULA
 
    if (errors)
        return 0;
    else
        *pUp = (data[0] << 16 | data[1] << 8) >> (8 - m_oss);
#ifdef BMP180_TEST_FORMULA
        *pUp = 23843;
#endif // #ifdef BMP180_TEST_FORMULA
 
    return 1;
}
 
float BMP180_TrueTemperature(long ut)
{
    long t;
    
    // straight out from the documentation
    x1 = ((ut - ac6) * ac5) >> 15;
    x2 = ((long)mc << 11) / (x1 + md);
    b5 = x1 + x2;
    t = (b5 + 8) >> 4;
 
    // convert to celcius
    return t / 10.F;
}
 
float BMP180_TruePressure(long up)
{
    long p;
    
    // straight out from the documentation
    b6 = b5 - 4000;
    x1 = (b2 * (b6 * b6 >> 12)) >> 11;
    x2 = ac2 * b6 >> 11;
    x3 = x1 + x2;
    b3 = (((ac1 * 4 + x3) << m_oss) + 2) >> 2;
    x1 = (ac3 * b6) >> 13;
    x2 = (b1 * ((b6 * b6) >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    b4 = ac4 * (unsigned long)(x3 + 32768) >> 15;
    b7 = ((unsigned long)up - b3)* (50000 >> m_oss);
    if (b7 < 0x80000000)
        p = (b7 << 1) / b4;
    else
        p = (b7 / b4) << 1;
    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    p = p + ((x1 + x2 + 3791) >> 4);
 
    // convert to hPa and, if altitude has been initialized, to sea level pressure  
    if (m_altitude == 0.F)
        return p / 100.F;
    else
        return  p / (100.F * pow((1.F - m_altitude / 44330.0L), 5.255L)); 
}

int BMP180_Write(int address, int data)
{
	char _buff[2];

	if (!deviceReady)
		return 1;

	_buff[0] = address;
	_buff[1] = data;

	if ( write(fd, _buff, 2) != 2)
		return 1;

	return 0;
}

int BMP180_Read(int address, int length, unsigned char buffer[])
{
	char _buff[1];

	if (!deviceReady)
		return 1;

	_buff[0] = address;

	if ( write(fd, _buff, 1) != 1)
		return 1;

	if ( read( fd, buffer, length) != length)
		return 1;

	return 0;
}

