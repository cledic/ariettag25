#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <string.h>

#include "PWM_lib.h"

#define cBUFFER_SIZE    255

/*
 echo $PwmChn > /sys/class/pwm/pwmchip0/export

 echo 0 > /sys/class/pwm/pwmchip0/pwm$PwmChn/enable
 echo $Period > /sys/class/pwm/pwmchip0/pwm$PwmChn/period
 echo $Duty > /sys/class/pwm/pwmchip0/pwm$PwmChn/duty_cycle
 echo 1 > /sys/class/pwm/pwmchip0/pwm$PwmChn/enable

*/

int PWMChannel_enable( char* chnl)
{
    int fchnl;

    fchnl=open("/sys/class/pwm/pwmchip0/export", O_WRONLY);
    if (fchnl < 0)
    {
        fprintf( stderr, "Cannot open /sys/class/pwm/pwmchip0/export to export it \n");
        return -1;
    }

    // Write the channel you want to export and close the export interface.
    write(fchnl, chnl, 4);
    close(fchnl);

    return 0;
}

int PWMChannel_SetPeriod( char* chnl, char* period)
{
    int fchnl;

    char *buff=(char*)malloc( cBUFFER_SIZE);


    if ( buff==(char*)NULL)
    {
        fprintf( stderr, "Cannot allocate memory\n");
        return -1;
    }

    snprintf( buff, cBUFFER_SIZE, "/sys/class/pwm/pwmchip0/pwm%s/enable", chnl);

    fchnl=open(buff, O_WRONLY);
    if (fchnl < 0)
    {
        fprintf( stderr, "Cannot open %s to disable the channel \n", buff);
        free(buff);
        return -1;
    }

    // Disable the channel
    write(fchnl, "0", 2);
    close(fchnl);

    snprintf( buff, cBUFFER_SIZE, "/sys/class/pwm/pwmchip0/pwm%s/period", chnl);

    fchnl=open(buff, O_WRONLY);
    if (fchnl < 0)
    {
        fprintf( stderr, "Cannot open %s to set period \n", buff);
        free(buff);
        return -1;
    }
    //
    write(fchnl, period, strlen( period));
    close(fchnl);

    snprintf( buff, cBUFFER_SIZE, "/sys/class/pwm/pwmchip0/pwm%s/enable", chnl);

    fchnl=open(buff, O_WRONLY);
    if (fchnl < 0)
    {
        fprintf( stderr, "Cannot open %s to re-enable the channel \n", buff);
        free(buff);
        return -1;
    }

    // Disable the channel
    write(fchnl, "1", 2);
    close(fchnl);
    free(buff);

    return 0;
}


int PWMChannel_SetDutyCycle( char* chnl, char* duty)
{
    int fchnl;

    char *buff=(char*)malloc( cBUFFER_SIZE);

    if ( buff==(char*)NULL)
    {
        fprintf( stderr, "Cannot allocate memory\n");
        return -1;
    }

    snprintf( buff, cBUFFER_SIZE, "/sys/class/pwm/pwmchip0/pwm%s/enable", chnl);

    fchnl=open(buff, O_WRONLY);
    if (fchnl < 0)
    {
        fprintf( stderr, "Cannot open %s to disable the channel \n", buff);
        free(buff);
        return -1;
    }

    // Disable the channel
    write(fchnl, "0", 2);
    close(fchnl);

    snprintf( buff, cBUFFER_SIZE, "/sys/class/pwm/pwmchip0/pwm%s/duty_cycle", chnl);

    fchnl=open(buff, O_WRONLY);
    if (fchnl < 0)
    {
        fprintf( stderr, "Cannot open %s to set duty_cycle \n", buff);
        free(buff);
        return -1;
    }
    //
    write(fchnl, duty, strlen( duty));
    close(fchnl);

    snprintf( buff, cBUFFER_SIZE, "/sys/class/pwm/pwmchip0/pwm%s/enable", chnl);

    fchnl=open(buff, O_WRONLY);
    if (fchnl < 0)
    {
        fprintf( stderr, "Cannot open %s to re-enable the channel \n", buff);
        free(buff);
        return -1;
    }

    // Disable the channel
    write(fchnl, "1", 2);
    close(fchnl);
    free(buff);

    return 0;
}
