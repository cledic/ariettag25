#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

char get_gpio_Num( int gpioid)
{
	if ( gpioid >=64 && gpioid <= 95)
		return gpioid-64;

	if ( gpioid >=0 && gpioid <= 31)
		return gpioid;

	if ( gpioid >=43 && gpioid <= 46)
		return gpioid-32;
		
	return 64;
}

char get_gpio_Port( int gpioid)
{
	if ( gpioid >=64 && gpioid <= 95)
		return 'C';

	if ( gpioid >=0 && gpioid <= 31)
		return 'A';

	if ( gpioid >=43 && gpioid <= 46)
		return 'B';
		
	return 'Z';
}

int gpioexport(int gpioid) 
{
	FILE *filestream;

	if ((filestream=fopen("/sys/class/gpio/export","w"))==NULL) {
		printf("Error on export GPIO\n");
		return -1;
	}	
	fprintf(filestream,"%d",gpioid);
	fclose(filestream);
	return 0;
}

int gpiosetdir(int gpioid,char *mode) 
{
	FILE *filestream;
	char filename[50];

	sprintf(filename,"/sys/class/gpio/pio%c%d/direction",get_gpio_Port(gpioid), get_gpio_Num(gpioid));
	if ((filestream=fopen(filename,"w"))==NULL) {
		printf("Error on direction setup\n");
		return -1;
	}	
	fprintf(filestream,mode);
	fclose(filestream);
	return 0;
}

int gpiogetbits(int gpioid) 
{
	FILE *filestream;
	char filename[50];
	char retchar;

	gpioexport( gpioid);
	gpiosetdir( gpioid, "in");
	
	sprintf(filename,"/sys/class/gpio/pio%c%d/value",get_gpio_Port(gpioid), get_gpio_Num(gpioid));
	if ((filestream=fopen(filename,"r"))==NULL) {
		printf("Error on gpiogetbits %d\n",gpioid);
		return -1;
	}	
	retchar=fgetc(filestream);
	fclose(filestream);
	if (retchar=='0') return 0;
	else return 1;
}

int gpiosetbits(int gpioid) 
{
	FILE *filestream;
	char filename[50];

	gpioexport( gpioid);
	gpiosetdir( gpioid, "out");

	sprintf(filename,"/sys/class/gpio/pio%c%d/value",get_gpio_Port(gpioid), get_gpio_Num(gpioid));
	if ((filestream=fopen(filename,"w"))==NULL) {
		printf("Error on gpiosetbits %d\n",gpioid);
		return -1;
	}	
	fprintf(filestream,"1");
	fclose(filestream);
	return 0;
}

int gpioclearbits(int gpioid) 
{
	FILE *filestream;
	char filename[50];

	gpioexport( gpioid);
	gpiosetdir( gpioid, "out");

	sprintf(filename,"/sys/class/gpio/pio%c%d/value",get_gpio_Port(gpioid), get_gpio_Num(gpioid));
	if ((filestream=fopen(filename,"w"))==NULL) {
		printf("Error on gpioclearbits %d\n",gpioid);
		return -1;
	}	
	fprintf(filestream,"0");
	fclose(filestream);
	return 0;
}

#if 0
int main(void) 
{
	int led = 82;
	
	gpioexport(led);
	gpiosetdir(led,"out");
	for (;;) {
		gpiosetbits(led);
		sleep(1);
		gpioclearbits(led);
		sleep(1);
	}
}
#endif

