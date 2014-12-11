#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>

#include "PWM_lib.h"

int SingleStep=50000;
int MinPos=500000;
int MaxPos=1000000;
int Period=16650000;
int Duty=0;

/**
 */
int main(int argc, char *argv[])
{
	if ( argc != 4) {
		fprintf(stderr, "Error: %s <channel> <period> <duty_cycle> [%d]\n", argv[0], argc);
		return 1;
	}

	Duty=MaxPos;

	PWMChannel_enable( argv[1]);

	PWMChannel_SetPeriod( argv[1], argv[2]);

	PWMChannel_SetDutyCycle( argv[1], argv[3]);

	printf("Hello World\n");

	return 0;
}

