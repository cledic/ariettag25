#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
//
#include "NokiaLCD_lib.h"

/* Macro di conversione RGB */
#define RGB(r,g,b)           (((r&0xF0)<<8)|(g&0xF0)|((b&0xF0)>>4)) //4 red | 4 green | 4 blue

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

struct FONT {
	int x;
	int y;
	int size;
	unsigned char fr;	// foreground color RGB
	unsigned char fg;
	unsigned char fb;
	unsigned char br;	// background color RGB
	unsigned char bg;
	unsigned char bb;
};

static void parse_opts(int argc, char *argv[]);
static void print_usage(const char *prog);
char* readLine( FILE* file );

/* Variabili alterabili da cmdline dall'utente */
static const char *device = "/dev/spidev32766.0";		// A default viene usata la spi1 CS0
static int noreset=0;						// A default viene eseguito il reset+init
static int pin_io=82;						// A default viene usato il pin 82
static const char *filename;
static const char *MoveFilename;
static const char *FullScreenFilename;
static const char *StringToDsiplay;
static int clearcmd=0;						// Comando di clear dello schermo.

static uint8_t mode;
struct RECT rect;
struct FONT font;
int isImage=0;
int isMovie=0;
int isFullScreen=0;
int isStringToDsiplay=0;
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
	fprintf( stdout, "  -n --noreset		no reset is performed\n");
	fprintf( stdout, "  -p --pinreset <val>		pin to use as reset\n");
	fprintf( stdout, "  -c --clear			redraw the screeen with color if supplied\n");
	fprintf( stdout, "				or WHITE. Execute first if with -f opt\n");
	fprintf( stdout, "  -l --loop <val>     	loop n times, used with -m option\n");
	fprintf( stdout, "  -S --string <val>     	string to display at x, y\n");
	fprintf( stdout, "  -F --font <val>     	Font: 0=SMALL 1=MEDIUM 2=LARGE\n");
	fprintf( stdout, "  -t --rate <val>     	frame rate in ms for -m option\n");
	fprintf( stdout, "  -x --xpos <val>     	x position to start to draw\n");
	fprintf( stdout, "  -y --ypos <val>     	y position to start to draw\n");
	fprintf( stdout, "  -w --width <val>     	width to draw\n");
	fprintf( stdout, "  -h --height <val>     	height to draw\n");
	fprintf( stdout, "  -r --red <val>     		red color\n");
	fprintf( stdout, "  -g --green <val>   		green color\n");
	fprintf( stdout, "  -b --blue <val>    		blue color\n");
	fprintf( stdout, "  -R --RED <val>     		red color to use as  backgound font\n");
	fprintf( stdout, "  -G --GREEN <val>   		green color to use as  backgound font\n");
	fprintf( stdout, "  -B --BLUE <val>    		blue color to use as  backgound font\n");
	//
	exit(1);
}

int main( int argc, char *argv[])
{
	// Imposto i valori iniziali della dimensione dell'LCD...
	rect.x=LCD_X_OFFSET;
	rect.y=LCD_Y_OFFSET;
	rect.w=LCD_WIDTH-1;
	rect.h=LCD_HEIGHT-1;
	rect.r=rect.g=rect.b=0;

	font.x=LCD_X_OFFSET;
	font.y=LCD_Y_OFFSET;
	font.size=SMALL;
	font.fr=font.fg=font.fb=250;	// foreground == WHITE
	font.br=font.bg=font.bb=0;	// background == BLACK

	parse_opts(argc, argv);
	
	// Inizializzo l'LCD della Nokia, controller compatibile al PCF8833
	InitLcd( PCF8833, device);
	
	if ( noreset==0) LCDReset();
	
	if ( clearcmd) {
		LCDDrawScreen( RGB( rect.r, rect.g, rect.b));
		usleep( 250000);
	}

	if ( isStringToDsiplay) {
		if ( strlen( StringToDsiplay)==1 && StringToDsiplay[0]=='-') {	// stdin
			StringToDsiplay=readLine( stdin );
		}

		LCDSetCurr_XY( rect.x, rect.y);
		LCDPutStr( StringToDsiplay, font.x, font.y, font.size, RGB(font.fr,font.fg, font.fb), RGB(font.br,font.bg, font.bb));
	}

	if ( (filename != (const char*)NULL) && isImage) LCDWritebmp( (const char*)filename, rect.x, rect.y, rect.w, rect.h);
	//if ( (FullScreenFilename != (const char*)NULL) && isFullScreen) LCDWriteRGB( (const char*)FullScreenFilename);
	if ( (FullScreenFilename != (const char*)NULL) && isFullScreen) LCDWriteFullScreenRGB_2( (const char*)FullScreenFilename);
	if ( (MoveFilename != (const char*)NULL) && isMovie) {
		if ( LoopFor == -1) {
			if (LCDMovieRGB( (const char*)MoveFilename)) {
				return 1;
			}
		} else {
			while( LoopFor--) {
				if (LCDMovieRGB( (const char*)MoveFilename)) {
					return 1;
				}
			}
		}
	}


	return 0;
}

static void parse_opts(int argc, char *argv[])
{

	static const struct option lopts[] = {
		{ "device",  	0, 0, 'D' },
		{ "noreset", 	0, 0, 'n' },
		{ "pinreset",	1, 0, 'p' },
		{ "file",	1, 0, 'f' },
		{ "screen",	1, 0, 's' },
		{ "string",	1, 0, 'S' },
		{ "font",	1, 0, 'F' },
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
		{ "RED",        1, 0, 'R' },
		{ "BLUE", 	1, 0, 'B' },
		{ "GREEN", 	1, 0, 'G' },
		{ NULL, 0, 0, 0 },
	};
	int c;

//	fprintf( stdout, "parse_opts\n");

	while (1) {
		c = getopt_long(argc, argv, "f:m:s:S:F:p:Dncx:l:t:y:w:h:r:g:b:R:G:B:", lopts, NULL);

//		fprintf( stdout, "opts: %c\n", c);

		if (c == -1)
			break;
		
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
		case 'S':
			isStringToDsiplay=1;
			StringToDsiplay = optarg;
			break;
		case 'F':
			font.size = atoi(optarg); 
			if ( font.size > LARGE)
				font.size=LARGE;
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
			font.x=rect.x;
			break;
		case 'y':
			rect.y = atoi(optarg);
			if ( rect.y > LCD_HEIGHT)
				rect.y=LCD_HEIGHT;
			font.y=rect.y;
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
			font.br = atoi(optarg);
			break;
		case 'g':
			rect.g = atoi(optarg);
			font.bg = atoi(optarg);
			break;
		case 'b':
			rect.b = atoi(optarg);
			font.bb = atoi(optarg);
			break;
		case 'R':
			font.fr = atoi(optarg);
			break;
		case 'G':
			font.fg = atoi(optarg);
			break;
		case 'B':
			font.fb = atoi(optarg);
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

char* readLine( FILE* file )
{
	char buffer[1024];
	char* result = 0;
	int length = 0;
	
	result = (char*)malloc(1024);

	while( read(STDIN_FILENO , buffer ,  sizeof( buffer )) )
	{
		int len = strlen(buffer);
		buffer[len] = 0;
	
		length += len;
		char* tmp = (char*)malloc(length+1);
		tmp[0] = 0;
	
		if( result )
		{
			strcpy( tmp, result );
			free( result );
			result = tmp;
		}
	
		strcat( result, buffer );
	
		// if( strstr( buffer, "\n" ) break;
	}
	
	return result;
}
