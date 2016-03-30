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

/* GLCD definition and functions. */
#define LCD_MODE_65K        5          // 5=65K, 6=262K colors.

/* Bit di controllo dell'LCD */
#define LCD_ID               (0)
#define LCD_DATA             ((0x72)|(LCD_ID<<2))
#define LCD_REGISTER         ((0x70)|(LCD_ID<<2))
/* Dimensioni schermo */
#define LCD_WIDTH            (320)
#define LCD_HEIGHT           (240)
/* Macro di conversione RGB */
#define RGB(r,g,b)           (((r&0xF8)<<8)|((g&0xFC)<<3)|((b&0xF8)>>3)) //5 red | 6 green | 5 blue

/* Macro */
#define wait_ms(x)			usleep(x * 1000)
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
//
#define cSPI_BUFF	(LCD_WIDTH*LCD_HEIGHT*2)

/* */
static void pabort(const char *s)
{
	perror(s);
//	abort();
}
 
/* Definizione funzioni. */
void lcd_area(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1);
void lcd_cmd(unsigned char reg, unsigned char param);
void lcd_data(unsigned short c);
void lcd_init(void);
int lcd_reset( void);
void lcd_drawstart(void);
unsigned int lcd_drawimage( char *filename);
unsigned int lcd_drawRGBimage( const char *filename);
unsigned int lcd_drawmovie( char *filename);
unsigned int lcd_drawmoviefullres( char *filename);
void lcd_clear( void);
void lcd_fillscreen( unsigned short color);
void lcd_fillarea( unsigned short x, unsigned short y, unsigned short w, unsigned short h, unsigned short color);
//
static void parse_opts(int argc, char *argv[]);
static void print_usage(const char *prog);

/* Variabili alterabili da cmdline dall'utente */
static const char *device = "/dev/spidev32766.0";		// A default viene usata la spi1 CS0
static int noreset=0;						// A default viene eseguito il reset+init
static int pin_io=82;						// A default viene usato il pin 82
static const char *filename;
static const char *MoveFilename;
static const char *FullScreenFilename;
static int clearcmd=0;						// Comando di clear dello schermo.

/* Geometria dell'immagine impostata dall'utente. */
struct RECT {
		int x;
		int y;
		int w;
		int h;
	unsigned char r;
	unsigned char g;
	unsigned char b;
};
/* Funzioni che tengono conto della geometria ipmostata dall'utente. */
unsigned int lcd_drawRGBimageRect( const char *filename, struct RECT rect);
void lcd_fillareaRect( struct RECT rect);
unsigned int lcd_drawmovieRect( const char *filename, struct RECT rect);

int fd;
static uint8_t mode;
struct RECT rect;
int isImage=0;
int isMovie=0;
int isFullScreen=0;
int FrameRate=60;		// frame rate used for movie playing in ms
int LoopFor=-1;

/**
 */
static void print_usage(const char *prog)
{
	fprintf( stdout, "Usage: %s [-Dnpf]\n", prog);
	fprintf( stdout, "  -D --device <val>	  	device to use (default %s)\n", device);
	fprintf( stdout, "  -f --file <name>    	file RGB to draw\n");
	fprintf( stdout, "  -s --screen <name>    	full screen file RGB to draw\n");
	fprintf( stdout, "  -m --movie <name>    	movie file RGB 565 to play\n");
	fprintf( stdout, "  -n --noreset			no reset is performed\n");
	fprintf( stdout, "  -p --pinreset <val>		pin to use as reset\n");
	fprintf( stdout, "  -c --clear				redraw the screeen with color if supplied\n");
	fprintf( stdout, "							or WHITE. Execute first if with -f opt\n");
	fprintf( stdout, "  -l --loop <val>     	loop n times, used with -m option\n");
	fprintf( stdout, "  -t --rate <val>     	frame rate in ms for -m option\n");
	fprintf( stdout, "  -x --xpos <val>     	x position to start to draw\n");
	fprintf( stdout, "  -y --ypos <val>     	y position to start to draw\n");
	fprintf( stdout, "  -w --width <val>     	width to draw\n");
	fprintf( stdout, "  -h --height <val>     	height to draw\n");
	fprintf( stdout, "  -r --red <val>     		red color\n");
	fprintf( stdout, "  -g --green <val>   		green color\n");
	fprintf( stdout, "  -b --blue <val>    		blue color\n");
	//
	exit(1);
}

/** 
 */
int main(int argc, char *argv[])
{
	int ret;

	// Imposto i valori iniziali della dimensione dell'LCD...
	rect.x=0;
	rect.y=0;
	rect.w=LCD_WIDTH;
	rect.h=LCD_HEIGHT;
	rect.r=rect.g=rect.b=0;

	parse_opts(argc, argv);

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

	// Verifico se l'utente ha richiesto il reset e l'init dell'LCD
	if ( noreset==0) lcd_init();
	
	//
	if ( clearcmd) {
		lcd_fillareaRect( rect);
		usleep( 250000);
	}
	//
	if ( (filename != (const char*)NULL) && isImage) lcd_drawRGBimageRect( (const char*)filename, rect);
	if ( (FullScreenFilename != (const char*)NULL) && isFullScreen) lcd_drawRGBimage( (const char*)FullScreenFilename);
	if ( (MoveFilename != (const char*)NULL) && isMovie) {
		if ( LoopFor == -1) {
			if (lcd_drawmovieRect( (const char*)MoveFilename, rect)) {
				return 1;
			}
		} else {
			while( LoopFor--) {
				if ( lcd_drawmovieRect( (const char*)MoveFilename, rect)) {
					return 1;
				}
			}
		}
	}

	close( fd);
	return 0;
}

void lcd_cmd(unsigned char reg, unsigned char param)
{
	int ret;
	unsigned char cmd[2];

	cmd[0]=LCD_REGISTER;
	cmd[1]=reg;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)cmd,
		.rx_buf = (unsigned long)NULL,
		.len = ARRAY_SIZE( cmd),
		.delay_usecs = 0,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	usleep( 50);

	if (ret < 1)
		pabort("can't send lcd_cmd::register message");

	
	cmd[0]=LCD_DATA;
	cmd[1]=param;
	struct spi_ioc_transfer tr2 = {
		.tx_buf = (unsigned long)cmd,
		.rx_buf = (unsigned long)NULL,
		.len = ARRAY_SIZE( cmd),
		.delay_usecs = 0,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr2);
	usleep( 50);

	if (ret < 1)
		pabort("can't send lcd_cmd::data message");

  return;
}


void lcd_data(unsigned short c)
{
	int ret;
	unsigned char cmd[3];

	cmd[0]=LCD_DATA;
	cmd[1]=(unsigned char)(c>>8);
	cmd[2]=(unsigned char)(c&0xFF);
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)cmd,
		.rx_buf = (unsigned long)NULL,
		.len = ARRAY_SIZE( cmd),
		.delay_usecs = 0,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	usleep( 50);

	if (ret < 1)
		pabort("can't send lcd_data::data message");

  return;
}

void lcd_init(void)
{

  //
  lcd_reset();

  //driving ability
  lcd_cmd(0xEA, 0x00);
  lcd_cmd(0xEB, 0x20);
  lcd_cmd(0xEC, 0x0C);
  lcd_cmd(0xED, 0xC4);
  lcd_cmd(0xE8, 0x40);
  lcd_cmd(0xE9, 0x38);
  lcd_cmd(0xF1, 0x01);
  lcd_cmd(0xF2, 0x10);
  lcd_cmd(0x27, 0xA3);

  //power voltage
  lcd_cmd(0x1B, 0x1B);
  lcd_cmd(0x1A, 0x01);
  lcd_cmd(0x24, 0x2F);
  lcd_cmd(0x25, 0x57);

  //VCOM offset
  lcd_cmd(0x23, 0x8D); //for flicker adjust

  //power on
  lcd_cmd(0x18, 0x36);
  lcd_cmd(0x19, 0x01); //start osc
  lcd_cmd(0x01, 0x00); //wakeup
  lcd_cmd(0x1F, 0x88);
  wait_ms(5);
  lcd_cmd(0x1F, 0x80);
  wait_ms(5);
  lcd_cmd(0x1F, 0x90);
  wait_ms(5);
  lcd_cmd(0x1F, 0xD0);
  wait_ms(5);

  //color selection
  lcd_cmd(0x17, LCD_MODE_65K); //0x0005=65k, 0x0006=262k 

  //panel characteristic
  lcd_cmd(0x36, 0x00);

  //display on
  lcd_cmd(0x28, 0x38);
  wait_ms(40);
  lcd_cmd(0x28, 0x3C);

  //display options
#ifdef LCD_MIRROR
  lcd_cmd(0x16, 0x68); //MY=0 MX=1 MV=1 ML=0 BGR=1
#else
  lcd_cmd(0x16, 0xA8); //MY=1 MX=0 MV=1 ML=0 BGR=1
#endif

  lcd_area(0, 0, (LCD_WIDTH-1), (LCD_HEIGHT-1));

  return;
}


void lcd_area(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1)
{

  lcd_cmd(0x03, (x0>>0)); //set x0
  lcd_cmd(0x02, (x0>>8)); //set x0
  lcd_cmd(0x05, (x1>>0)); //set x1
  lcd_cmd(0x04, (x1>>8)); //set x1
  lcd_cmd(0x07, (y0>>0)); //set y0
  lcd_cmd(0x06, (y0>>8)); //set y0
  lcd_cmd(0x09, (y1>>0)); //set y1
  lcd_cmd(0x08, (y1>>8)); //set y1

  return;
}

void lcd_drawstart(void)
{
	int ret;
	unsigned char cmd[2];
	
	cmd[0]=LCD_REGISTER;
	cmd[1]=0x22;

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)cmd,
		.rx_buf = (unsigned long)NULL,
		.len = ARRAY_SIZE( cmd),
		.delay_usecs = 0,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	usleep( 50);

	if (ret < 1)
		pabort("can't send lcd_drawstart::register message");

}

/** Ripulisce lo schermo disegnandolo di nero.
 */
void lcd_clear( void)
{
	int ret;
	int i;	
	unsigned char buff[ cSPI_BUFF+1];

	// metto a zero l'intero array
	memset( buff, 0x00, cSPI_BUFF+1);

	// Fisso la dimensione dell'area di schermo.
    lcd_area(0, 0, (LCD_WIDTH-1), (LCD_HEIGHT-1));

	// Invio il comando di start.. 
	lcd_drawstart();
	buff[0]=LCD_DATA;
	struct spi_ioc_transfer tr2 = {
		.tx_buf = (unsigned long)buff,
		.rx_buf = (unsigned long)NULL,
		.len = ARRAY_SIZE( buff),
		.delay_usecs = 0,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr2);

	if (ret < 1) {
		// pabort("can't send lcd_clear::nocs message");
		fprintf( stderr, "ERRORE: clear::ics_message\n");
	}

}

/** Ripulisce lo schermo disegnandolo con il colore specificato.
 *
 * @param	r, g, b
 * @return	none
 */
void lcd_fillscreen( unsigned short color)
{
	int ret;
	int i;	
	unsigned char buff[ cSPI_BUFF+1];

	// metto a zero l'intero array
	memset( buff, 0, cSPI_BUFF+1);

	i=1;
	while( i<(LCD_WIDTH*LCD_HEIGHT*2)) {
		buff[i++] = ( color>>8);
		buff[i++] = ( color&0xFF);
	}
	
	// Fisso la dimensione dell'area di schermo.
    lcd_area(0, 0, (LCD_WIDTH-1), (LCD_HEIGHT-1));

	// Invio il comando di start.. 
	lcd_drawstart();
	buff[0]=LCD_DATA;
	struct spi_ioc_transfer tr2 = {
		.tx_buf = (unsigned long)buff,
		.rx_buf = (unsigned long)NULL,
		.len = ARRAY_SIZE( buff),
		.delay_usecs = 0,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr2);

	if (ret < 1) {
		// pabort("can't send lcd_clear::nocs message");
		fprintf( stderr, "ERRORE: fillscreen::ioc_messages\n");
	}

}

/** Ripulisce un'area dello schermo disegnandolo con il colore specificato.
 *
 * @param	x, y, w, h, r, g, b
 * @return	none
 */
void lcd_fillareaRect( struct RECT rect)
{
	int ret;
	int i;	
	unsigned short color;

	unsigned char *buff = (unsigned char*)malloc( (rect.w*rect.h*2)+1);

	// metto a zero l'intero array
	memset( buff, 0, (rect.w*rect.h*2)+1);

	color=RGB( rect.r, rect.g, rect.b);

	i=1;
	while( i<(rect.w*rect.h*2)) {
		buff[i++] = ( color>>8);
		buff[i++] = ( color&0xFF);
	}
	
	// Fisso la dimensione dell'area di schermo.
    	lcd_area(rect.x, rect.y, (rect.w-1)+rect.x, (rect.h-1)+rect.y);

	// Invio il comando di start.. 
	lcd_drawstart();
	buff[0]=LCD_DATA;
	struct spi_ioc_transfer tr2 = {
		.tx_buf = (unsigned long)buff,
		.rx_buf = (unsigned long)NULL,
		.len = (rect.w*rect.h*2)+1,
		.delay_usecs = 0,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr2);

	free( buff);

	if (ret < 1) {
		// pabort("can't send lcd_clear::nocs message");
		fprintf( stderr, "ERRORE: fillarea::ioc_message\n");
	}

}

/** Ripulisce un'area dello schermo disegnandolo con il colore specificato.
 *
 * @param	x, y, w, h, r, g, b
 * @return	none
 */
void lcd_fillarea( unsigned short x, unsigned short y, unsigned short w, unsigned short h, unsigned short color)
{
	int ret;
	int i;	
	//unsigned char buff[ w*h*2+1];

	unsigned char *buff=(unsigned char*)malloc( (w*h*2)+1);
	// metto a zero l'intero array
	memset( buff, 0, (w*h*2)+1);

	i=1;
	while( i<(w*h*2)) {
		buff[i++] = ( color>>8);
		buff[i++] = ( color&0xFF);
	}
	
	// Fisso la dimensione dell'area di schermo.
    lcd_area(x, y, (w-1)+x, (h-1)+y);

	// Invio il comando di start.. 
	lcd_drawstart();
	buff[0]=LCD_DATA;
	struct spi_ioc_transfer tr2 = {
		.tx_buf = (unsigned long)buff,
		.rx_buf = (unsigned long)NULL,
		.len = ARRAY_SIZE( buff),
		.delay_usecs = 0,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr2);

	free(buff);

	if (ret < 1) {
		// pabort("can't send lcd_clear::nocs message");
		fprintf( stderr, "ERRORE: fillarea::ioc_message\n");
	}

}


/** Legge il file specificato e lo visualizza sullo schermo.
 * L'immagine deve essere 320x240 e composta da due byte per pixel nel formato
 * RGB 565
 *
 * @param	char *filename	
 *			il nome del file da visualizzare.
 * @return	0 -> OK, 1 -> errore.
 */
unsigned int lcd_drawimage( char *filename)
{
	int ret;

	FILE *f;
	
	f=fopen( filename, "rb");
	if (f==NULL)
		return 1;
	
	unsigned char *buff=(unsigned char*)malloc( cSPI_BUFF+1);

	// metto a zero l'intero array
	memset( buff, 0x00, cSPI_BUFF+1);

	// Posiziono il comando LCD_DATA come primo byte del buffer
	buff[0]=LCD_DATA;
	// Poi leggo il buffer dell'immagine ( -1 byte)
	fread( &buff[1], 1, cSPI_BUFF, f);
	fclose( f);

	// Fisso la dimensione dell'area di schermo.
    lcd_area(0, 0, (LCD_WIDTH-1), (LCD_HEIGHT-1));
	// Invio il comando di start.. 
	lcd_drawstart();			

	// il primo byte del buffer contiene il comando LCD_DATA a seguire i byte RGB già impacchettati per
	// essere visualizzati.
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)buff,
		.rx_buf = (unsigned long)NULL,
		.len = ARRAY_SIZE( buff),
		.delay_usecs = 0,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

	free(buff);

	if (ret < 1) {
		// pabort("can't send lcd_drawstart::register message");
		fprintf( stdout, "ERRORE: lcd_drawstart::register message\n");
	}
	
	return 0;
}

/** Legge il file specificato e lo visualizza sullo schermo.
 * L'immagine deve essere 320x240 e composta da tre byte per pixel nel formato
 * RGB 888
 *
 * @param	char *filename	
 *			il nome del file da visualizzare.
 * @return	0 -> OK, 1 -> errore.
 */
unsigned int lcd_drawRGBimage( const char *filename)
{
	int ret;
	int i;
	unsigned short rgb_val;
	unsigned char r, g, b;
	FILE *f;

	unsigned char *buff=(unsigned char*)malloc( (320*240*2)+1);

	// metto a zero l'intero array
	memset( buff, 0x00, (320*240*2)+1);
	// Posiziono il comando LCD_DATA come primo byte del buffer
	buff[0]=LCD_DATA;
	

	f=fopen( filename, "rb");
	if (f==NULL) {
		free(buff);
		return 1;
	}
	
	i=1;
	// Leggo l'intero file convertendo il formato RGB a 8 bit in 565
	while( !feof( f)) {
		//
		r=fgetc( f);
		g=fgetc( f);
		b=fgetc( f);
		//
		rgb_val = RGB( r, g, b);
		buff[i++]=(rgb_val>>8);
		buff[i++]=(rgb_val&0xFF);
		//
		if ( i>(320*240*2))
			break;
	}
	//
	fclose( f);

	// Fisso la dimensione dell'area di schermo.
    lcd_area(0, 0, (320-1), (240-1));
	// Invio il comando di start.. 
	lcd_drawstart();			

	// il primo byte del buffer contiene il comando LCD_DATA a seguire i byte RGB già impacchettati per
	// essere visualizzati.
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)buff,
		.rx_buf = (unsigned long)NULL,
		.len = (320*240*2)+1,
		.delay_usecs = 0,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	free( buff);

	if (ret < 1) {
		// pabort("can't send lcd_drawstart::register message");
		fprintf( stdout, "ERRORE:lcd_drawRGBimage::register message\n");
	}
	
	return 0;
}

/** Legge il file specificato e lo visualizza sullo schermo.
 * L'immagine deve essere in formato RGB. Tiene conto della dimensione
 * usando la struttura RECT.
 *
 * @param	char *filename	
 *			il nome del file da visualizzare.
 * @param	RECT rect
 * @return	0 -> OK, 1 -> errore.
 */
unsigned int lcd_drawRGBimageRect( const char *filename, struct RECT rect)
{
	int ret;
	int i;
	unsigned short rgb_val;
	unsigned char r, g, b;
	FILE *f;

	// 
	unsigned char *buff=(unsigned char*)malloc( (rect.w*rect.h*2)+1);

	// metto a zero l'intero array
	memset( buff, 0x00, (rect.w*rect.h*2)+1);
	// Posiziono il comando LCD_DATA come primo byte del buffer
	buff[0]=LCD_DATA;
	
	f=fopen( filename, "rb");
	if (f==NULL) {
		free(buff);
		return 1;
	}
	
	i=1;
	// Leggo l'intero file convertendo il formato RGB a 8 bit in 565
	while( !feof( f)) {
		//
		r=fgetc( f);
		g=fgetc( f);
		b=fgetc( f);
		//
		rgb_val = RGB( r, g, b);
		buff[i++]=(rgb_val>>8);
		buff[i++]=(rgb_val&0xFF);
		//
		if ( i > (rect.w*rect.h*2) )
			break;
	}
	//
	fclose( f);

	// Fisso la dimensione dell'area di schermo.
    lcd_area(rect.x, rect.y, (rect.w-1)+rect.x, (rect.h-1)+rect.y);
	// Invio il comando di start.. 
	lcd_drawstart();			

	// il primo byte del buffer contiene il comando LCD_DATA a seguire i byte RGB già impacchettati per
	// essere visualizzati.
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)buff,
		.rx_buf = (unsigned long)NULL,
		.len = (rect.w*rect.h*2)+1,
		.delay_usecs = 0,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

	free( buff);

	if (ret < 1) {
		// pabort("can't send lcd_drawstart::register message");
		fprintf( stdout, "ERRORE:lcd_drawRGBimage::register message\n");
	}
	
	return 0;
}

/** Visualizza il contenuto di un file precedentemente realizzato come sequenza di immagini in movimento
 *  in formato RGB 565, di dimensione 320x240
 */
unsigned int lcd_drawmovieRect( const char *filename, struct RECT rect)
{
	int ret, i, ii;
	unsigned short rgb_val;

	FILE *f;
	
	unsigned char *buff=(unsigned char*)malloc( (rect.w*rect.h*2)+1);

	f=fopen( filename, "rb");
	if (f==NULL) {
		free(buff);
		return 1;
	}
	
	// metto a zero l'intero array
	memset( buff, 0x00, (rect.w*rect.h*2)+1);
	
	// Fisso la dimensione dell'area di schermo.
    lcd_area(rect.x, rect.y, rect.w-1, rect.h-1);

	while( !feof( f)) {
		// Posiziono il comando LCD_DATA come primo byte del buffer
		buff[0]=LCD_DATA;
		// Poi leggo il buffer dell'immagine ( -1 byte)
		fread( &buff[1], 1, (rect.w*rect.h*2), f);

		// Invio il comando di start.. 
		lcd_drawstart();			

		// il primo byte del buffer contiene il comando LCD_DATA a seguire i byte RGB già impacchettati per
		// essere visualizzati.
		struct spi_ioc_transfer tr = {
			.tx_buf = (unsigned long)buff,
			.rx_buf = (unsigned long)NULL,
			.len = (rect.w*rect.h*2)+1,
			.delay_usecs = 0,
		};

		ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

		if (ret < 1) {
			//pabort("can't send lcd_drawstartfullres::register message");
			fprintf( stdout, "ERRORE: fullres ioc_message\n");
			free(buff);
			fclose( f);	
			return 1;
		}

		usleep( FrameRate*1000);
	}
	//
	fclose( f);	
	free(buff);

	return 0;
}


/** Visualizza il contenuto di un file precedentemente realizzato come sequenza di immagini in movimento
 *  in formato RGB 565, di dimensione 160x120
 */
unsigned int lcd_drawmovie( char *filename)
{
	int ret;
	unsigned char buff[(160*120*2)+1];

	FILE *f;
	
	f=fopen( filename, "rb");
	if (f==NULL)
		return 1;
	
	// Fisso la dimensione dell'area di schermo.
	lcd_area(0, 0, ((LCD_WIDTH/2)-1), ((LCD_HEIGHT/2)-1));

	while( !feof( f)) {
		// metto a zero l'intero array
		memset( buff, 0x00, (160*120*2)+1);

		// Posiziono il comando LCD_DATA come primo byte del buffer
		buff[0]=LCD_DATA;
		// Poi leggo il buffer dell'immagine ( -1 byte)
		fread( &buff[1], 1, (160*120*2), f);
	
		// Invio il comando di start.. 
		lcd_drawstart();			

		// il primo byte del buffer contiene il comando LCD_DATA a seguire i byte RGB già impacchettati per
		// essere visualizzati.
		struct spi_ioc_transfer tr = {
			.tx_buf = (unsigned long)buff,
			.rx_buf = (unsigned long)NULL,
			.len = ARRAY_SIZE( buff),
			.delay_usecs = 0,
		};

		ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

		if (ret < 1) {
			// pabort("can't send lcd_drawstart::register message");
			fprintf( stdout, "ERRORE: lcd_drawstart::register message\n");
		}

		usleep( FrameRate*1000);
	}
	//
	fclose( f);	

	return 0;
}

/** Visualizza il contenuto di un file precedentemente realizzato come sequenza di immagini in movimento
 *  in formato RGB 565, di dimensione 320x240
 */
unsigned int lcd_drawmovieRGBRect( char *filename, struct RECT rect)
{
	int ret, i, ii;
	unsigned short rgb_val;

	FILE *f;
	
	unsigned char *buff=(unsigned char*)malloc( (rect.w*rect.h*3)+1);

	f=fopen( filename, "rb");
	if (f==NULL) {
		free(buff);
		return 1;
	}
	
	while( !feof( f)) {
		// metto a zero l'intero array
		memset( buff, 0x00, (rect.w*rect.h*3)+1);

		// Posiziono il comando LCD_DATA come primo byte del buffer
		buff[0]=LCD_DATA;
		// Poi leggo il buffer dell'immagine ( -1 byte)
		fread( &buff[1], 1, (rect.w*rect.h*3), f);

		// Copio inplace l'immagine letta RGB 3 byte in RGB 2 byte
		ii=1;
		for ( i=1; i<(rect.w*rect.h*3); i+3) {
			rgb_val = RGB( buff[i], buff[i+1], buff[i+2]);
			buff[ii++]=(rgb_val>>8);
			buff[ii++]=(rgb_val&0xFF);
		}

		// Fisso la dimensione dell'area di schermo.
	    lcd_area(rect.x, rect.y, rect.w-1, rect.h-1);
		// Invio il comando di start.. 
		lcd_drawstart();			

		// il primo byte del buffer contiene il comando LCD_DATA a seguire i byte RGB già impacchettati per
		// essere visualizzati.
		struct spi_ioc_transfer tr = {
			.tx_buf = (unsigned long)buff,
			.rx_buf = (unsigned long)NULL,
			.len = (rect.w*rect.h*2)+1,
			.delay_usecs = 0,
		};

		ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

		if (ret < 1) {
			// pabort("can't send lcd_drawstartfullres::register message");
			fprintf( stdout, "ERRORE: fullres ioc_message\n");
			fclose( f);	
			free(buff);
			return 1;
		}
		usleep( FrameRate*1000);
	}
	//
	fclose( f);	
	free(buff);

	return 0;
}

/** Visualizza il contenuto di un file precedentemente realizzato come sequenza di immagini in movimento
 *  in formato RGB 565, di dimensione 320x240
 */
unsigned int lcd_drawmoviefullres( char *filename)
{
	int ret;

	FILE *f;
	
	f=fopen( filename, "rb");
	if (f==NULL)
		return 1;
	
	unsigned char *buff=(unsigned char*)malloc( (320*240*2)+1);
	// metto a zero l'intero array
	memset( buff, 0x00, (320*240*2)+1);
	// Fisso la dimensione dell'area di schermo.
	lcd_area(0, 0, (320-1), (240-1));

	while( !feof( f)) {
		// Posiziono il comando LCD_DATA come primo byte del buffer
		buff[0]=LCD_DATA;
		// Poi leggo il buffer dell'immagine ( -1 byte)
		fread( &buff[1], 1, (320*240*2), f);
	
		// Invio il comando di start.. 
		lcd_drawstart();			

		// il primo byte del buffer contiene il comando LCD_DATA a seguire i byte RGB già impacchettati per
		// essere visualizzati.
		struct spi_ioc_transfer tr = {
			.tx_buf = (unsigned long)buff,
			.rx_buf = (unsigned long)NULL,
			.len = ARRAY_SIZE( buff),
			.delay_usecs = 0,
		};

		ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

		if (ret < 1) {
			// pabort("can't send lcd_drawstartfullres::register message");
			fprintf( stdout, "ERRORE: fullres ioc_message\n");
			fclose( f);	
			free( buff);
			return 1;
		}

		usleep( FrameRate*1000);
	}
	//
	fclose( f);	
	free( buff);

	return 0;
}

/** Esegue il reset dell'LCD. L'lcd rimane in reset per circa 50ms
 * Gestisco il gpio usando il sys.
 */
int lcd_reset( void)
{
    int exportfd;
    exportfd = open("/sys/class/gpio/export", O_WRONLY);
    if (exportfd < 0)
    {
    fprintf( stderr, "Cannot open GPIO to export it \n");
    return -1;
    }
	// Write the GPIO you want to export and close the export interface.
    write(exportfd, "82", 4);
    close(exportfd);
	
	// Next, You will need to configure the direction of the GPIO using the direction interface of GPIOLIB.
    int directionfd;
    directionfd = open("/sys/class/gpio/gpio82/direction", O_RDWR);
    if (directionfd < 0)
    {
    fprintf( stderr, "Cannot open GPIO direction for 82\n");
    return -1;
    }

	// Write "in" if the GPIO is input or "out" if the GPIO is output and close the interface.
    write(directionfd, "out", 4);
    close(directionfd);

	// You can now use the value interface of GPIOLIB to set/clear the GPIO pin.
	// To make the GPIO line LOW
    int valuefd;
    valuefd = open("/sys/class/gpio/gpio82/value", O_RDWR);
    if (valuefd < 0)
    {
    fprintf( stderr, "Cannot open GPIO value for 82\n");
    return -1;
    }
    write(valuefd, "0", 2);
    close(valuefd);

	//
	usleep( 50*1000);
	
	// To make the GPIO line HIGH
    valuefd = open("/sys/class/gpio/gpio82/value", O_RDWR);
    if (valuefd < 0)
    {
    fprintf( stderr, "Cannot open GPIO value for 82\n");
    return -1;
    }
    write(valuefd, "1", 2);
    close(valuefd);

    // Ritardo prima di iniziare la parte di init dell'LCD...
    usleep( 250*1000);

//    printf("fine reset\n");	
}


static void parse_opts(int argc, char *argv[])
{

	static const struct option lopts[] = {
		{ "device",  	0, 0, 'D' },
		{ "noreset", 	0, 0, 'n' },
		{ "pinreset",	1, 0, 'p' },
		{ "file",	1, 0, 'f' },
		{ "screen",	1, 0, 's' },
		{ "movie",	1, 0, 'm' },
		{ "loop",	1, 0, 'l' },
		{ "rate", 	1, 0, 't' },
		{ "xpos", 	1, 0, 'x' },
		{ "ypos", 	1, 0, 'y' },
		{ "width", 	1, 0, 'w' },
		{ "height", 	1, 0, 'h' },
		{ "clear", 	0, 0, 'c' },
		{ "red",        1, 0, 'r' },
		{ "blue", 	1, 0, 'b' },
		{ "green", 	1, 0, 'g' },
		{ NULL, 0, 0, 0 },
	};
	int c;

//	fprintf( stdout, "parse_opts\n");

	while (1) {
		c = getopt_long(argc, argv, "f:m:s:p:Dncx:l:t:y:w:h:r:g:b:", lopts, NULL);

		if (c == -1)
			break;

//		fprintf( stdout, "opts: %c\n", c);

		switch (c) {
		case 'D':
			device = optarg;
			break;
		case 'n':
			noreset=1;
			break;
		case 'p':
			pin_io = atoi(optarg);
			break;
		case 's':
			isFullScreen=1;
			FullScreenFilename = optarg;
			break;
		case 'f':
			isImage=1;
			filename = optarg;
			break;
		case 'm':
			isMovie=1;
			MoveFilename = optarg;
			break;
		case 'l':
			LoopFor = atoi(optarg);
			break;
		case 't':
			FrameRate = atoi(optarg);
			break;
		case 'x':
			rect.x = atoi(optarg);
			if ( rect.x > LCD_WIDTH)
				rect.x=LCD_WIDTH;
			break;
		case 'y':
			rect.y = atoi(optarg);
			if ( rect.y > LCD_HEIGHT)
				rect.y=LCD_HEIGHT;
			break;
		case 'w':
			rect.w = atoi(optarg);
			if ( rect.w > LCD_WIDTH)
				rect.w=LCD_WIDTH;
			break;
		case 'h':
			rect.h = atoi(optarg);
			if ( rect.y > LCD_HEIGHT)
				rect.y=LCD_HEIGHT;
			break;
		case 'r':
			rect.r = atoi(optarg);
			break;
		case 'g':
			rect.g = atoi(optarg);
			break;
		case 'b':
			rect.b = atoi(optarg);
			break;
		case 'c':
			clearcmd=1;
			break;
		default:
			print_usage(argv[0]);
			break;
		}
	}
}




