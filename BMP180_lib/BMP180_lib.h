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

#ifndef BMP180_h
#define BMP180_h

#include <inttypes.h>

#define I2C_ADDR 0x77

#define ChipIdData 0x55
#define ControlInstruction_MeasureTemperature 0x2E
#define ControlInstruction_MeasurePressure 0x34

#define Reg_ChipId 0xD0
#define Reg_Control 0xF4
#define Reg_CalibrationStart 0xAA
#define Reg_CalibrationEnd 0xBE
#define Reg_AnalogConverterOutMSB 0xF6
#define Reg_SoftReset 0xE0
#define SoftResetInstruction 0xB6

#define ErrorCode_1 "Entered sample resolution was invalid. See datasheet for details."
#define ErrorCode_1_Num 1

#define BMP180_Mode_UltraLowPower		0
#define BMP180_Mode_Standard			1
#define BMP180_Mode_HighResolution		2
#define BMP180_Mode_UltraHighResolution	3

void BMP180_BMP180( void);

int BMP180_Initialize(const char *device);

int BMP180_GetUncompensatedTemperature( void);
float BMP180_CompensateTemperature(int uncompensatedTemperature);

long BMP180_GetUncompensatedPressure( void);
long BMP180_CompensatePressure(long uncompensatedPressure);

float BMP180_GetTemperature( void);
long BMP180_GetPressure( void);

float BMP180_GetAltitude(float currentSeaLevelPressureInPa);

void BMP180_SoftReset( void);
uint8_t BMP180_SetResolution(uint8_t sampleResolution, bool oversample);

void BMP180_PrintCalibrationData( void);

uint8_t BMP180_EnsureConnected( void);
uint8_t BMP180_IsConnected;
char* BMP180_GetErrorText(int errorCode);

int BMP180_Write(int address, int byte);
int BMP180_Read(int address, int length, uint8_t buffer[]);

uint8_t OversamplingSetting;
bool Oversample;
int ConversionWaitTimeMs;
int LastTemperatureData;
int LastTemperatureTime;
int AcceptableTemperatureLatencyForPressure;

int Calibration_AC1;
int Calibration_AC2;
int Calibration_AC3;
unsigned int Calibration_AC4;
unsigned int Calibration_AC5;
unsigned int Calibration_AC6;
int Calibration_B1;
int Calibration_B2;
int Calibration_MB;
int Calibration_MC;
int Calibration_MD;

#endif
