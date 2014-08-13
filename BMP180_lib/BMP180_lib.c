/*
BMP180.h - Header file for the BMP180 Barometric Pressure Sensor Arduino Library.
Copyright (C) 2012 Love Electronics Ltd (loveelectronics.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the version 3 GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

 Datasheet for BMP180:
 http://www.bosch-sensortec.com/content/language1/downloads/BST-BMP180-DS000-07.pdf

*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <math.h>

#include "BMP180_lib.h"

/* file handle for I2C device */
static int fd;
/* flag for device succesfully opened */
static int deviceReady=0;

#define delay( x)	usleep( x*1000);

void BMP180_BMP180( void)
{
  ConversionWaitTimeMs = 5;
  OversamplingSetting = 0;
  Oversample = false;

  LastTemperatureTime = -1000;
  LastTemperatureData = 0;

  AcceptableTemperatureLatencyForPressure = 1000;

  BMP180_SetResolution( BMP180_Mode_Standard, false);
}

uint8_t BMP180_EnsureConnected( void)
{
	uint8_t data;
	
	BMP180_Read(Reg_ChipId, 1, &data);

	if(data == ChipIdData)
		IsConnected = 1;
	else
		IsConnected = 0;

	return IsConnected;
}

int BMP180_Initialize( const char *device)
{
	fd = open( device, O_RDWR);

	if (fd < 0) {
		printf("Error opening file: %s\n", strerror(errno));
		return 1;
	}

	if (ioctl(fd, I2C_SLAVE, I2C_ADDR) < 0) {
		printf("ioctl error: %s\n", strerror(errno));
		return 1;
	}

	deviceReady=1;

	BMP180_BMP180();
	
	uint8_t buffer[ Reg_CalibrationEnd - Reg_CalibrationStart + 2];
	
	BMP180_Read(Reg_CalibrationStart, Reg_CalibrationEnd - Reg_CalibrationStart + 2, &buffer);
	// This data is in Big Endian format from the BMP180.
    Calibration_AC1 = (buffer[0] << 8) | buffer[1];
    Calibration_AC2 = (buffer[2] << 8) | buffer[3];
    Calibration_AC3 = (buffer[4] << 8) | buffer[5];
    Calibration_AC4 = (buffer[6] << 8) | buffer[7];
    Calibration_AC5 = (buffer[8] << 8) | buffer[9];
    Calibration_AC6 = (buffer[10] << 8) | buffer[11];
    Calibration_B1 = (buffer[12] << 8) | buffer[13];
    Calibration_B2 = (buffer[14] << 8) | buffer[15];
    Calibration_MB = (buffer[16] << 8) | buffer[17];
    Calibration_MC = (buffer[18] << 8) | buffer[19];
    Calibration_MD = (buffer[20] << 8) | buffer[21];
	
	return 0;;
}

void BMP180_PrintCalibrationData( void)
{
	printf( "AC1:\t %d\n" Calibration_AC1);
	printf( "AC2:\t %d\n" Calibration_AC2);
	printf( "AC3:\t %d\n" Calibration_AC3);
	printf( "AC4:\t %d\n" Calibration_AC4);
	printf( "AC5:\t %d\n" Calibration_AC5);
	printf( "AC6:\t %d\n" Calibration_AC6);
	printf( "B1:\t %d\n" Calibration_B1);
	printf( "B2:\t %d\n" Calibration_B2);
	printf( "MB:\t %d\n" Calibration_MB);
	printf( "MC:\t %d\n" Calibration_MC);
	printf( "MD:\t %d\n" Calibration_MD);
}

int BMP180_GetUncompensatedTemperature( void)
{
    // Instruct device to perform a conversion.
    BMP180_Write(Reg_Control, ControlInstruction_MeasureTemperature);
    // Wait for the conversion to complete.
    delay(5);
    uint8_t* data[2];
	BMP180_Read(Reg_AnalogConverterOutMSB, 2, &data);
    int value = (data[0] << 8) | data[1];
    return value;
}

long BMP180_GetUncompensatedPressure( void)
{
    long pressure = 0;
    int loops = Oversample ? 3 : 1;

    for (int i = 0; i < loops; i++)
    {
        // Instruct device to perform a conversion, including the oversampling data.
        uint8_t CtrlByte = ControlInstruction_MeasurePressure + (OversamplingSetting << 6);
        BMP180_Write(Reg_Control, CtrlByte);
        // Wait for the conversion
        delay(ConversionWaitTimeMs);
        // Read the conversion data.
        uint8_t buffer[3];
		BMP180_Read(Reg_AnalogConverterOutMSB, 3, buffer);

        // Collect the data (and push back the LSB if we are not sampling them).
        pressure = ((((long)buffer[0] <<16) | ((long)buffer[1] <<8) | ((long)buffer[2])) >> (8-OversamplingSetting));
    }
    return pressure / loops;
}

float BMP180_CompensateTemperature(int uncompensatedTemperature)
{
    int temperature;
    int x2;
	long x1;
	x1 = (((long)uncompensatedTemperature - (long)Calibration_AC6) * (long)Calibration_AC5) >> 15;
    x2 = ((long)Calibration_MC << 11) / (x1 + Calibration_MD);
    int param_b5 = x1 + x2;
    temperature = (int)((param_b5 + 8) >> 4);  /* temperature in 0.1 deg C*/
    float fTemperature = temperature;
	fTemperature /= 10.0;

    // Record this data because it is required by the pressure algorithem.
    LastTemperatureData = param_b5;
    LastTemperatureTime = millis();

    return fTemperature;
}

long BMP180_CompensatePressure(long uncompensatedPressure)
{
	int msSinceLastTempReading = millis() - LastTemperatureTime;
    // Check to see if we have old temperature data.
    if (msSinceLastTempReading > AcceptableTemperatureLatencyForPressure)
        GetTemperature(); // Refresh the temperature.

    // Data from the BMP180 datasheet to test algorithm.
    /*OversamplingSetting = 0;
    uncompensatedPressure = 23843;
    LastTemperatureData = 2399;
    Calibration_AC1 = 408;
    Calibration_AC2 = -72;
    Calibration_AC3 = -14383;
    Calibration_AC4 = 32741;
    Calibration_AC5 = 32757;
    Calibration_AC6 = 23153;
    Calibration_B1 = 6190;
    Calibration_B2 = 4;
    Calibration_MB = -32767;
    Calibration_MC = -8711;
    Calibration_MD = 2868;*/

    // Algorithm taken from BMP180 datasheet.
    long b6 = LastTemperatureData - 4000;
    long x1 = (Calibration_B2 * (b6 * b6 >> 12)) >> 11;
    long x2 = Calibration_AC2 * b6 >> 11;
    long x3 = x1 + x2;
    long b3 = ((Calibration_AC1 * 4 + x3) << OversamplingSetting) + 2;
    b3 = b3 >> 2;
    x1 = Calibration_AC3 * b6 >> 13;
    x2 = (Calibration_B1 * (b6 * b6 >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    long b4 = Calibration_AC4 * (x3 + 32768) >> 15;
    unsigned long b7 = (((uncompensatedPressure - b3)) * (50000 >> OversamplingSetting));
    long p;
    if (b7 < 0x80000000)
	{
		p = ((b7 * 2) / b4);    
	}
    else
	{
        p = ((b7 / b4) * 2);
	}
		
    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    p = p + ((x1 + x2 + 3791) >> 4);

    return p;
}

void BMP180_SoftReset( void)
{
    BMP180_Write(Reg_SoftReset, SoftResetInstruction);
    delay(100);
}

float BMP180_GetTemperature( void)
{
    return BMP180_CompensateTemperature( BMP180_GetUncompensatedTemperature());
}

long BMP180_GetPressure( void)
{
    return BMP180_CompensatePressure( BMP180_GetUncompensatedPressure());
}

float BMP180_GetAltitude(float currentSeaLevelPressureInPa)
{
    // Get pressure in Pascals (Pa).
    float pressure = BMP180_GetPressure();
    // Calculate altitude from sea level.
    float altitude = 44330.0 * (1.0 - pow(pressure / currentSeaLevelPressureInPa, 0.1902949571836346));
    return altitude;
}

uint8_t BMP180_SetResolution(uint8_t sampleResolution, bool oversample)
{
    OversamplingSetting = sampleResolution;
    Oversample = oversample;
    switch (sampleResolution)
    {
        case 0:
            ConversionWaitTimeMs = 5;
            break;
        case 1:
            ConversionWaitTimeMs = 8;
            break;
        case 2:
            ConversionWaitTimeMs = 14;
            break;
        case 3:
            ConversionWaitTimeMs = 26;
            break;
        default:
            return ErrorCode_1_Num;
    }
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

int BMP180_Read(int address, int length, uint8_t buffer[])
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

char* BMP180_GetErrorText(int errorCode)
{
	if(ErrorCode_1_Num == 1)
		return ErrorCode_1;
	
	return "Error not defined.";
}
