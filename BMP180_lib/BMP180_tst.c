#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <math.h>

/*
        http://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/tree/Documentation/i2c/dev-interface
        apt-get install i2c-tools
        apt-get install libi2c-dev
*/

// Include the Love Electronics BMP180 library.
#include "BMP180_lib.h"

int main( void)
{

	float temperatura, pressione;

	BMP180_Init( "/dev/i2c-0", 0, BMP180_OSS_NORMAL);

	BMP180_ReadValues( &temperatura, &pressione);

	printf("t: %f\tp:%f\n", temperatura, pressione);
}
