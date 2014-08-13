#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
//
#include "NokiaLCD_lib.h"
#include "NokiaLCD_lib_fonts.h"

void ShowWiFiSingleLogo( unsigned char idx);

#define LCD_PRINTF_BUF_SIZE 256

/* Macro */
#define DelayMs(x)			usleep(x * 1000)
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* */
static void pabort(const char *s)
{
	perror(s);
//	abort();
}

static unsigned char xBase = 0;
static unsigned char gx = 0;
static unsigned char gy = 0;
static unsigned int fgColor = LCD_DEFAULT_FG_COLOR;
static unsigned int bgColor = LCD_DEFAULT_BG_COLOR;
static unsigned char CurrFont = SMALL;
const unsigned char FontHeight[3] = {8,8,16};
const unsigned char FontWidth[3] = {6,8,8};
//
static unsigned char curr_x = 0;
static unsigned char curr_y = 0;

unsigned char *FontTable[] = {(unsigned char *)FONT6x8, (unsigned char *)FONT8x8, (unsigned char *)FONT8x16};
static unsigned char _type;

//
static int fd;
static unsigned char bits = 9;
static unsigned int mode;
static unsigned int speed = 9*1000*1000;

void LCDSetCurr_XY( unsigned char x, unsigned char y)
{
	curr_x = 131-x;
	curr_y = y;
}	

unsigned char LCDGetCurr_X( void) 
{
	return curr_x;
}	

unsigned char LCDGetCurr_Y( void) 
{
	return curr_y;
}	

// *************************************************************************************************
// LCDPrint
//
// Draws a null-terminates character string at current position
//
// Inputs: pString = pointer to character string to be displayed
// Size = font pitch (SMALL, MEDIUM, LARGE)
// fColor = 12-bit foreground color value rrrrggggbbbb
// bColor = 12-bit background color value rrrrggggbbbb
//
//
// Returns: nothing
//
// Notes: Here's an example to display "Hello World!" at address (20,20)
//
// LCDPrint("Hello World!", LARGE, WHITE, BLACK);
//
//
// *************************************************************************************************
void LCDPrint(const char *pString, unsigned char Size, unsigned int fColor, unsigned int bColor) {
	char c;
	unsigned char ytmp;
	unsigned char x, y;
	
	//
	x = curr_x;
	ytmp = y = curr_y;
	
	// loop until null-terminator is seen
	while (*pString != 0x00) {
		//
		c=*pString++;
		// Ciclo sulla coppia CRLF incrementando x ed y
		while ( c == 0x0D && *pString == 0x0A) {
			pString++;
			c=*pString++;
			ytmp = 0;
			x -= FontHeight[Size];
			if ( x > 131)
				x=0;
		}
		// Visualizzo solo caratteri printabili...
		if ( c < 32)
			continue;
			
		// visualizzo il carattere...
		LCDPutChar(c, x, ytmp, Size, fColor, bColor);
		// advance the y position
		ytmp += FontWidth[Size];
		// verifico se sono fuori schermo
		if (ytmp > (LCD_WIDTH-FontWidth[Size])) {
			ytmp = 0;
			x -= FontHeight[Size];
			if ( x > 131)
				x=0;
		}		
	}
	// Memorizzo l'attuale posizione.
	curr_x=x;
	curr_y=ytmp;
}

// 
void InitLcd(unsigned char type, const unsigned char* device)
{
	unsigned char i;
	int ret;
	
	_type = type;
	
	LCDReset();
	
	fd = open(device, O_FSYNC|O_RDWR);
	if (fd < 0)
		pabort("can't open device");
	//
	mode |= SPI_CPHA;  
	mode |= SPI_CPOL;

	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");
	
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");
	
	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set speed");
	
	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get speed");

	//fprintf( stdout, "bits: %d\n", bits);

    switch ( type) {
        case LCD6100:
            WriteSpiCommand(0xCA); // display control
            WriteSpiData(0);
            WriteSpiData(32);
            WriteSpiData(0);
            WriteSpiCommand(0xBB);
            WriteSpiData(1);
            WriteSpiCommand(0xD1); // oscillator on
            WriteSpiCommand(0x94); // sleep out
            WriteSpiCommand(0x20); // power control
            WriteSpiData(0x0F);
            WriteSpiCommand(0xA7); // invert display
            WriteSpiCommand(0x81); // Voltage control
            WriteSpiData(39);      // contrast setting: 0..63
            WriteSpiData(3);       // resistance ratio
            DelayMs(1);
            WriteSpiCommand(0xBC);
            WriteSpiData(0);
            WriteSpiData(1);
            WriteSpiData(4);
            WriteSpiCommand(0xAF);  // turn on the display
            break;
            
        case LCD6610:
            WriteSpiCommand(0xCA);    // display control
            WriteSpiData(0);
            WriteSpiData(31);
            WriteSpiData(0);
            WriteSpiCommand(0xBB);
            WriteSpiData(1);
            WriteSpiCommand(0xD1); // oscillator on
            WriteSpiCommand(0x94); // sleep out
            WriteSpiCommand(0x20); // power control
            WriteSpiData(0x0F);
            WriteSpiCommand(0xA7); // invert display
            WriteSpiCommand(0x81); // Voltage control
            WriteSpiData(39);      // contrast setting: 0..63
            WriteSpiData(3);       // resistance ratio
            DelayMs(1);
            WriteSpiCommand(0xBC);
            WriteSpiData(0);
            WriteSpiData(0);
            WriteSpiData(2);
            WriteSpiCommand(0xAF);  // turn on the display
            break;
            
        case PCF8833:
            WriteSpiCommand(0x11);  // sleep out
            WriteSpiCommand(0x3A);  // column mode
            WriteSpiData(0x03);		// 12bit pixel
            WriteSpiCommand(0x36);  // madctl
//            WriteSpiData(0x40);     // mirror X, RGB
//            WriteSpiData(0x00);     // no mirror , RGB
            WriteSpiData(0x80);     // mirror Y, RGB
            WriteSpiCommand(0x25);  // setcon
            WriteSpiData(0x40);		// contrast 0x30
            DelayMs(100);
            WriteSpiCommand(0x29);	//DISPON
            DelayMs(250);
            WriteSpiCommand(0x03);	//BSTRON
            DelayMs(250);
            break;
    }
	//
	LCDSetCurr_XY( 0, 0);
}

// *************************************************************************************************
// LCDPutStr.c
//
// Draws a null-terminates character string at the specified (x,y) address, size and color
//
// Inputs: pString = pointer to character string to be displayed
// x = row address (0 .. 131)
// y = column address (0 .. 131)
// Size = font pitch (SMALL, MEDIUM, LARGE)
// fColor = 12-bit foreground color value rrrrggggbbbb
// bColor = 12-bit background color value rrrrggggbbbb
//
//
// Returns: nothing
//
// Notes: Here's an example to display "Hello World!" at address (20,20)
//
// LCDPutChar("Hello World!", 20, 20, LARGE, WHITE, BLACK);
//
//
// Author: James P Lynch August 30, 2007
// *************************************************************************************************
void LCDPutStr(const char *pString, unsigned char x, unsigned char y, unsigned char Size, unsigned int fColor, unsigned int bColor) {
        char c;
        unsigned char xtmp, ytmp;

	// ^ Y
	// |
	// |
	// +----> X
        //
        y=131-y-FontHeight[Size];
        xtmp=x;
        // loop until null-terminator is seen
        while (*pString != 0x00) {
                //
                c=*pString++;
                //
                if ( c == 0x0D && *pString == 0x0A) {
                        pString++;
                        c=*pString++;
                        xtmp = x;
                        y -= FontHeight[Size];
                        if ( y <= 0)
                                y=131-FontHeight[Size];
                }
                if ( c < 32)
                        continue;

                // draw the character
                LCDPutChar(c, xtmp, y, Size, fColor, bColor);
                // advance the x position
                xtmp += FontWidth[Size];
                // bail out if x exceeds 131
                if (xtmp >= (LCD_WIDTH-FontWidth[Size])) {
                        xtmp = x;
                        y -= FontHeight[Size];
                        if ( y <= 0)
                                y=131-FontHeight[Size];
                }
        }
}

/**
**
*/
void LCDPutChar(char c, unsigned char x, unsigned char y, unsigned char size, unsigned int fColor, unsigned int bColor) {
	char i, j;
	unsigned char nCols;
	unsigned char nRows;
	unsigned char nBytes;
	unsigned char PixelRow;
	unsigned char Mask;
	unsigned int Word0;
	unsigned int Word1;
	unsigned int CharIdx;
	const unsigned char *pFont;
	const unsigned char *pChar;

	// get pointer to the beginning of the selected font table
	pFont = (const unsigned char *)FontTable[size];
	//pFont = (const unsigned char *)FONT6x8;
	// get the nColumns, nRows and nBytes
	nCols = *pFont;
	nRows = *(pFont + 1);
	nBytes = *(pFont + 2);
	// get pointer to the last byte of the desired character
	// pChar = pFont + (unsigned int)((nBytes * (c - 0x1F)) + nBytes - 1);
	CharIdx = (unsigned int)((unsigned int)((unsigned int)nBytes * (unsigned int)(c - 0x1F)) + nBytes - 1);

	//printf("char:%c, nCol:%d, nRows:%d, nBytes:%d, CharIdx: %d\n", c, nCols, nRows, nBytes, CharIdx);
	//printf("X:%d, Y:%d\n", x, y);

	// Column address set (command 0x2A)
	WriteSpiCommand( 0x2A);
	WriteSpiData(x);
	WriteSpiData(x + nCols - 1);
	// Row address set (command 0x2B)
	WriteSpiCommand( 0x2B);
	WriteSpiData(y);
	WriteSpiData(y + nRows - 1);
	// WRITE MEMORY
	WriteSpiCommand( 0x2C);
	// loop on each row, working backwards from the bottom to the top
	//for (i=nRows-1; i >= 0; i--) {
	for (i=0; i <= (nRows-1); i++) {
		// copy pixel row from font table and then decrement row
		// PixelRow = *pChar--;
		PixelRow = pFont[ CharIdx--];
		// loop on each pixel in the row (left to right)
		// Note: we do two pixels each loop
		Mask = 0x80;
		//printf("idx:%d, Mask:0x%X, piexlRow:0x%X\n", i, Mask, PixelRow);
		for (j = 0; j < nCols; j += 2) {
			// if pixel bit set, use foreground color; else use the background color
			// now get the pixel color for two successive pixels
			if ((PixelRow & Mask) == 0)
				Word0 = bColor;
			else
				Word0 = fColor;
			
			Mask = Mask >> 1;
			if ((PixelRow & Mask) == 0)
				Word1 = bColor;
			else
				Word1 = fColor;
			
			Mask = Mask >> 1;
			// use this information to output three data bytes
			WriteSpiData((Word0 >> 4) & 0xFF);
			WriteSpiData(((Word0 & 0xF) << 4) | ((Word1 >> 8) & 0xF));
			WriteSpiData(Word1 & 0xFF);
		}
	}
	// terminate the Write Memory command
	WriteSpiCommand( 0x00);
}

void WriteSpiCommand(unsigned char cmd)
{
	int ret;
	uint16_t buff;
	
	buff = cmd;
	
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)&buff,
		.rx_buf = (unsigned long)NULL,
		.len = 2,
		.bits_per_word = 9,
		.delay_usecs = 0,
	};
	
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1) {
		fprintf( stdout, "ERRORE: SpiCmd ioc_message\n");
		close( fd);	
	}
}
void WriteSpiData(unsigned char data)
{
	int ret;
	uint16_t buff;
	
	buff = 0x0100 | data;
	
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)&buff,
		.rx_buf = (unsigned long)NULL,
		.len = 2,
		.bits_per_word = 9,
		.delay_usecs = 0,
	};
	
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1) {
		fprintf( stdout, "ERRORE: SpiData ioc_message\n");
		close( fd);	
	}

}

void WritePixelColor( unsigned int color);
void WritePixelColor( unsigned int color)
{
	int ret;
	uint16_t buff[]={0x0100,0x0100,0x0100};
	
	buff[0] |= (color >> 4) & 0xFF;
	buff[1] |= ((color & 0xF) << 4) | ((color >> 8) & 0xF);
	buff[2] |= color & 0xFF;
	
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)&buff,
		.rx_buf = (unsigned long)NULL,
		.len = 6,
		.bits_per_word = 9,
		.delay_usecs = 0,
	};
	
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1) {
		fprintf( stdout, "ERRORE: Write2PixelColor ioc_message\n");
		close( fd);	
	}

}

void WritePixelColorArray( unsigned char *data);
void WritePixelColorArray( unsigned char *data)
{
	int ret;
	uint16_t buff[]={0x0100,0x0100,0x0100,0x0100,0x0100,0x0100};
	
	buff[0] |= (data[0]&0xF0 ) | ((data[1]&0xF0) >> 4);
	buff[1] |= (data[2]&0xF0 ) | ((data[3]&0xF0) >> 4);
	buff[2] |= (data[4]&0xF0 ) | ((data[5]&0xF0) >> 4);
	buff[3] |= (data[6]&0xF0 ) | ((data[7]&0xF0) >> 4);
	buff[4] |= (data[8]&0xF0 ) | ((data[9]&0xF0) >> 4);
	buff[5] |= (data[10]&0xF0 ) | ((data[11]&0xF0) >> 4);
	
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)&buff,
		.rx_buf = (unsigned long)NULL,
		.len = 12,
		.bits_per_word = 9,
		.delay_usecs = 0,
	};
	
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1) {
		fprintf( stdout, "ERRORE: Write2PixelColor ioc_message\n");
	}
}

// *****************************************************************************
//
// Clears the LCD screen to single color (BLACK)
//
// Inputs: none
//
// Author: James P Lynch August 30, 2007
// *****************************************************************************
void LCDClearScreen(void) {
	unsigned int i; // loop counter
	
    unsigned char x1 = 0 + 0;
    unsigned char y1 = 0 + 0;
    unsigned char x2 = x1 + LCD_WIDTH - 1;
    unsigned char y2 = y1 + LCD_HEIGHT - 1;

    switch (_type) {
        case LCD6100:
        case LCD6610:
            WriteSpiCommand(0x15); // column
            WriteSpiData(x1);
            WriteSpiData(x2);
            WriteSpiCommand(0x75); // row
            WriteSpiData(y1);
            WriteSpiData(y2);
            WriteSpiCommand(0x5C); // start write to ram
            break;
        case PCF8833:
            WriteSpiCommand(0x2A);  // column
            WriteSpiData(x1);
            WriteSpiData(x2);
            WriteSpiCommand(0x2B); // row
            WriteSpiData(y1);
            WriteSpiData(y2);
            WriteSpiCommand(0x2C); // start write to ram
            break;
    }

	for(i = 0; i < (unsigned int)((LCD_WIDTH * LCD_HEIGHT) / 2); i++) {
		WriteSpiData((WHITE >> 4) & 0xFF);
		WriteSpiData(((WHITE & 0xF) << 4) | ((WHITE >> 8) & 0xF));
		WriteSpiData(WHITE & 0xFF);
	}
}

// *****************************************************************************
//
// Clears the LCD screen to single color (BLACK)
//
// Inputs: none
//
// Author: James P Lynch August 30, 2007
// *****************************************************************************
void LCDDrawScreen(unsigned int color) {
	unsigned int i; // loop counter
	
    unsigned char x1 = 0 + 0;
    unsigned char y1 = 0 + 0;
    unsigned char x2 = x1 + LCD_WIDTH - 1;
    unsigned char y2 = y1 + LCD_HEIGHT - 1;

    switch (_type) {
        case LCD6100:
        case LCD6610:
            WriteSpiCommand(0x15); // column
            WriteSpiData(x1);
            WriteSpiData(x2);
            WriteSpiCommand(0x75); // row
            WriteSpiData(y1);
            WriteSpiData(y2);
            WriteSpiCommand(0x5C); // start write to ram
            break;
        case PCF8833:
            WriteSpiCommand(0x2A);  // column
            WriteSpiData(x1);
            WriteSpiData(x2);
            WriteSpiCommand(0x2B); // row
            WriteSpiData(y1);
            WriteSpiData(y2);
            WriteSpiCommand(0x2C); // start write to ram
            break;
    }

	for(i = 0; i < (unsigned int)((LCD_WIDTH * LCD_HEIGHT) / 2); i++) {
#if 0
		WriteSpiData((color >> 4) & 0xFF);
		WriteSpiData(((color & 0xF) << 4) | ((color >> 8) & 0xF));
		WriteSpiData(color & 0xFF);
#else
		WritePixelColor( color);
#endif
	}
}

// *************************************************************************************
// LCDSetPixel.c
//
// Lights a single pixel in the specified color at the specified x and y addresses
//
// Inputs: x = row address (0 .. 131)
// y = column address (0 .. 131)
// color = 12-bit color value rrrrggggbbbb
// rrrr = 1111 full red
// :
// 0000 red is off
//
// gggg = 1111 full green
// :
// 0000 green is off
//
// bbbb = 1111 full blue
// :
// 0000 blue is off
//
// Returns: nothing
//
// Note: see lcd.h for some sample color settings
//
// Author: James P Lynch August 30, 2007
// *************************************************************************************
void LCDSetPixel(unsigned char x, unsigned char y, unsigned int color) {

    switch (_type) {
        case LCD6100:
        case LCD6610:
            WriteSpiCommand(0x15); // column
            WriteSpiData(x);
            WriteSpiData(x);
            WriteSpiCommand(0x75); // row
            WriteSpiData(y);
            WriteSpiData(y);
            WriteSpiCommand(0x5C); // start write to ram
            break;
        case PCF8833:
            WriteSpiCommand(0x2A);  // column
            WriteSpiData(x);
            WriteSpiData(x);
            WriteSpiCommand(0x2B); // row
            WriteSpiData(y);
            WriteSpiData(y);
            WriteSpiCommand(0x2C); // start write to ram
            break;
    }

	WriteSpiData((color >> 4) & 0xFF);
	WriteSpiData(((color & 0xF) << 4) | ((color >> 8) & 0xF));
	WriteSpiData(color & 0xFF);
}

void LCDWrite130x130bmp (const unsigned char *bmp)
{
    unsigned int j;

	WriteSpiCommand(0x2A);  // column
    WriteSpiData (LCD_X_OFFSET);
    WriteSpiData (LCD_X_OFFSET + LCD_WIDTH - 1); // 130 pixel viewable
	//
	WriteSpiCommand(0x2B); // row
    WriteSpiData (LCD_Y_OFFSET);
    WriteSpiData (LCD_Y_OFFSET + LCD_HEIGHT - 1); // 130 pixel viewable
	//
	WriteSpiCommand(0x2C); // start write to ram

    // write to memory
    for (j = 0; j < 25350; j++) 
    {
        WriteSpiData (bmp[j]);
    }

}

int LCDWriteFullScreenRGB (const char *filename)
{
    unsigned int j, i, ii;
	unsigned char r, g, b;
	FILE *f;
	int ret;
	unsigned short lb[25742];
	
	unsigned char *buff=(unsigned char*)malloc( LCD_WIDTH*LCD_HEIGHT*3);

	// metto a zero l'intero array
	memset( buff, 0x00, LCD_WIDTH*LCD_HEIGHT*3);

	f=fopen( filename, "rb");
	if (f==NULL)
		return 1;

	fread( &buff[0], 1, LCD_WIDTH*LCD_HEIGHT*3, f);
	fclose( f);

	// Alloco un array di short
	//uint16_t *lb = (uint16_t*) malloc(sizeof(uint16_t)*25350);
	//if (lb==NULL)
		//return 1;

    // write to buffer
	j=0;
	ii=0;
    for (i = 0; i < (unsigned int)((LCD_WIDTH * LCD_HEIGHT) / 2); i++) 
    {
        	//WriteSpiData ( (buff[j+0]&0xF0 ) | ((buff[j+1]&0xF0) >> 4));
		//WriteSpiData ( (buff[j+2]&0xF0 ) | ((buff[j+3]&0xF0) >> 4));
		//WriteSpiData ( (buff[j+4]&0xF0 ) | ((buff[j+5]&0xF0) >> 4));
		//*(lb++)=0x0100 | (buff[j+0]&0xF0 ) | ((buff[j+1]&0xF0) >> 4);
		//*(lb++)=0x0100 | (buff[j+2]&0xF0 ) | ((buff[j+3]&0xF0) >> 4);
		//*(lb++)=0x0100 | (buff[j+4]&0xF0 ) | ((buff[j+5]&0xF0) >> 4);
		lb[ii]=0x0100 | (buff[j+0]&0xF0 ) | ((buff[j+1]&0xF0) >> 4);
		lb[ii+1]=0x0100 | (buff[j+2]&0xF0 ) | ((buff[j+3]&0xF0) >> 4);
		lb[ii+2]=0x0100 | (buff[j+4]&0xF0 ) | ((buff[j+5]&0xF0) >> 4);
		j+=6;
		ii+=3;
    }

	WriteSpiCommand(0x2A);  // column
    WriteSpiData (LCD_X_OFFSET);
    WriteSpiData (LCD_X_OFFSET + LCD_WIDTH - 1); // 130 pixel viewable
	//
	WriteSpiCommand(0x2B); // row
    WriteSpiData (LCD_Y_OFFSET);
    WriteSpiData (LCD_Y_OFFSET + LCD_HEIGHT - 1); // 130 pixel viewable
	//
	WriteSpiCommand(0x2C); // start write to ram

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)&lb,
		.rx_buf = (unsigned long)NULL,
		.len = 25742*2,
		.bits_per_word = 9,
		.delay_usecs = 0,
	};
	
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1) {
		fprintf( stdout, "ERRORE: LCDWriteFullScreenRGB ioc_message\n");
		close( fd);	
	}
	
	free( buff);
	//free( lb);
	return 0;
}

int LCDWriteFullScreenRGB_2 (const char *filename)
{
    unsigned int j, i;
	FILE *f;
	
	unsigned char *buff=(unsigned char*)malloc( LCD_WIDTH*LCD_HEIGHT*3);

	// metto a zero l'intero array
	memset( buff, 0x00, LCD_WIDTH*LCD_HEIGHT*3);

	f=fopen( filename, "rb");
	if (f==NULL)
		return 1;

	fread( &buff[0], 1, LCD_WIDTH*LCD_HEIGHT*3, f);
	fclose( f);

	WriteSpiCommand(0x2A);  // column
    WriteSpiData (LCD_X_OFFSET);
    WriteSpiData (LCD_X_OFFSET + LCD_WIDTH - 1); // 130 pixel viewable
	//
	WriteSpiCommand(0x2B); // row
    WriteSpiData (LCD_Y_OFFSET);
    WriteSpiData (LCD_Y_OFFSET + LCD_HEIGHT - 1); // 130 pixel viewable
	//
	WriteSpiCommand(0x2C); // start write to ram

    // write to buffer
	j=0;
    for (i = 0; i < (unsigned int)((LCD_WIDTH * LCD_HEIGHT) / 4); i++) 
    {
		WritePixelColorArray( &buff[j]);
		j+=12;
	}

	free( buff);
	return 0;
}



int LCDWriteRGB (const char *filename)
{
    unsigned int j, i;
	unsigned char r, g, b;
	FILE *f;
	
	unsigned char *buff=(unsigned char*)malloc( LCD_WIDTH*LCD_HEIGHT*3);

	// metto a zero l'intero array
	memset( buff, 0x00, LCD_WIDTH*LCD_HEIGHT*3);

	f=fopen( filename, "rb");
	if (f==NULL)
		return 1;

	fread( &buff[0], 1, LCD_WIDTH*LCD_HEIGHT*3, f);
	fclose( f);


	WriteSpiCommand(0x2A);  // column
    WriteSpiData (LCD_X_OFFSET);
    WriteSpiData (LCD_X_OFFSET + LCD_WIDTH - 1); // 130 pixel viewable
	//
	WriteSpiCommand(0x2B); // row
    WriteSpiData (LCD_Y_OFFSET);
    WriteSpiData (LCD_Y_OFFSET + LCD_HEIGHT - 1); // 130 pixel viewable
	//
	WriteSpiCommand(0x2C); // start write to ram

    // write to memory
	j=0;
    for (i = 0; i < (unsigned int)((LCD_WIDTH * LCD_HEIGHT) / 2); i++) 
    {
        	WriteSpiData ( (buff[j+0]&0xF0 ) | ((buff[j+1]&0xF0) >> 4));
		WriteSpiData ( (buff[j+2]&0xF0 ) | ((buff[j+3]&0xF0) >> 4));
		WriteSpiData ( (buff[j+4]&0xF0 ) | ((buff[j+5]&0xF0) >> 4));
		j+=6;
    }
	
	free( buff);
	return 0;
}

int LCDMovieRGB (const char *filename)
{
	unsigned int j, i;
	unsigned char r, g, b;
	FILE *f;

	unsigned char *buff=(unsigned char*)malloc( LCD_WIDTH*LCD_HEIGHT*3);

	// metto a zero l'intero array
	memset( buff, 0x00, LCD_WIDTH*LCD_HEIGHT*3);

	f=fopen( filename, "rb");
	if (f==NULL)
		return 1;

	WriteSpiCommand(0x2A);  // column
    WriteSpiData (LCD_X_OFFSET);
    WriteSpiData (LCD_X_OFFSET + LCD_WIDTH - 1); // 130 pixel viewable
    //
    WriteSpiCommand(0x2B); // row
    WriteSpiData (LCD_Y_OFFSET);
    WriteSpiData (LCD_Y_OFFSET + LCD_HEIGHT - 1); // 130 pixel viewable
	//
	WriteSpiCommand(0x2C); // start write to ram

	while( !feof( f)) {
		fread( &buff[0], 1, LCD_WIDTH*LCD_HEIGHT*3, f);
		// write to memory
		j=0;
		for (i = 0; i < (unsigned int)((LCD_WIDTH * LCD_HEIGHT) / 2); i++)
		{
			WriteSpiData ( (buff[j+0]&0xF0 ) | ((buff[j+1]&0xF0) >> 4));
			WriteSpiData ( (buff[j+2]&0xF0 ) | ((buff[j+3]&0xF0) >> 4));
			WriteSpiData ( (buff[j+4]&0xF0 ) | ((buff[j+5]&0xF0) >> 4));
			j+=6;
		}
	}

	fclose( f);
    free( buff);
    return 0;
}


int LCDWritebmp (const char *filename, unsigned char x, unsigned char y, unsigned char w, unsigned char h)
{
    unsigned int j;
	FILE *f;
	
	unsigned char *buff=(unsigned char*)malloc( ((w*h)/2)*3);

	// metto a zero l'intero array
	memset( buff, 0x00, ((w*h)/2)*3);

	f=fopen( filename, "rb");
	if (f==NULL)
		return 1;

	fread( &buff[0], 1, ((w*h)/2)*3, f);
	fclose( f);
	
	WriteSpiCommand(0x2A);  			// column
    WriteSpiData (x);
    WriteSpiData (x + w ); 			
	//
	WriteSpiCommand(0x2B); 				// row
    WriteSpiData (y);
    WriteSpiData (y + h ); 			
	//
	WriteSpiCommand(0x2C); // start write to ram

    // write to memory
    for (j = 0; j < (((w*h)/2)*3); j++) 
    {
        WriteSpiData (buff[j]);
    }
	
	free(buff);
	
	return 0;
}

void LCDWrite64x64bmp (unsigned char x, unsigned char y, const unsigned char *bmp)
{
    unsigned int j;

	x=130-x;
	WriteSpiCommand(0x2A);  			// column
    WriteSpiData (x);
    WriteSpiData (x + 64 - 1); 			// 
	//
	WriteSpiCommand(0x2B); // row
    WriteSpiData (y);
    WriteSpiData (y + 64 - 1); // 130 pixel viewable
	//
	WriteSpiCommand(0x2C); // start write to ram

    // write to memory
    for (j = 0; j < 6144; j++) 
    {
        WriteSpiData (bmp[j]);
    }

}

void LCDWrite32x32bmp (unsigned char x, unsigned char y, const unsigned char *bmp)
{
    unsigned int j;

	x=130-x;
	x-=32;
	WriteSpiCommand(0x2B);  			// column
    WriteSpiData (x);
    WriteSpiData (x + 32 - 1); 			// 
	//
	WriteSpiCommand(0x2A); // row
    WriteSpiData (y);
    WriteSpiData (y + 32 - 1); // 130 pixel viewable
	//
	WriteSpiCommand(0x2C); // start write to ram

    // write to memory
    for (j = 0; j < 1536; j++) 
    {
        WriteSpiData (bmp[j]);
    }

}

/** Esegue il reset dell'LCD. L'lcd rimane in reset per circa 50ms
 * Gestisco il gpio usando il sys.
 */
int LCDReset( void)
{
    int exportfd;
    exportfd = open("/sys/class/gpio/export", O_WRONLY);
    if (exportfd < 0)
    {
    fprintf( stderr, "Cannot open GPIO to export it \n");
    return -1;
    }
	// Write the GPIO you want to export and close the export interface.
    write(exportfd, "68", 4);
    close(exportfd);
	
	// Next, You will need to configure the direction of the GPIO using the direction interface of GPIOLIB.
    int directionfd;
    directionfd = open("/sys/class/gpio/gpio68/direction", O_RDWR);
    if (directionfd < 0)
    {
    fprintf( stderr, "Cannot open GPIO direction for 68\n");
    return -1;
    }

	// Write "in" if the GPIO is input or "out" if the GPIO is output and close the interface.
    write(directionfd, "out", 4);
    close(directionfd);

	// You can now use the value interface of GPIOLIB to set/clear the GPIO pin.
	// To make the GPIO line LOW
    int valuefd;
    valuefd = open("/sys/class/gpio/gpio68/value", O_RDWR);
    if (valuefd < 0)
    {
    fprintf( stderr, "Cannot open GPIO value for 68\n");
    return -1;
    }
    write(valuefd, "0", 2);
    close(valuefd);

	//
	usleep( 50*1000);
	
	// To make the GPIO line HIGH
    valuefd = open("/sys/class/gpio/gpio68/value", O_RDWR);
    if (valuefd < 0)
    {
    fprintf( stderr, "Cannot open GPIO value for 68\n");
    return -1;
    }
    write(valuefd, "1", 2);
    close(valuefd);

    // Ritardo prima di iniziare la parte di init dell'LCD...
    usleep( 250*1000);

//    printf("fine reset\n");	
}


