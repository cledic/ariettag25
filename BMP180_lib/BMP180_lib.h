#ifndef BMP180_H
#define BMP180_H
  
///  default address is (0xEF >> 1)
#define I2C_ADDR 0x77
 
// Oversampling settings
#define BMP180_OSS_ULTRA_LOW_POWER 0        // 1 sample  and  4.5ms for conversion
#define BMP180_OSS_NORMAL          1        // 2 samples and  7.5ms for conversion
#define BMP180_OSS_HIGH_RESOLUTION 2        // 4 samples and 13.5ms for conversion
#define BMP180_OSS_ULTRA_HIGH_RESOLUTION 3  // 8 samples and 25.5ms for conversion
 
#define UNSET_BMP180_PRESSURE_VALUE 0.0f
#define UNSET_BMP180_TEMPERATURE_VALUE -273.15f // absolute zero
  
int BMP180_Init( const char *device, float altitude, int overSamplingSetting);

int BMP180_ReadValues(float* pTemperature, float* pPressure);
  
#endif

