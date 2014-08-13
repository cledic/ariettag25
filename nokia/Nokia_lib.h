////////////////////////////////////////////////////////////////
///	Libreria para PCF8833 (Nokia 6100 Color Display)       ///
///			         Version: 0.5                            ///
///  Basado en: http://www.apetech.de/nokia6100.php	       ///
///  y librería GLC.C de CCS.                                ///
///  Usar GTP GCLCD de Lager para generar fuentes e imagenes ///
///  Ajustado para trabajar con PICs y Compilador CCS        ///
///  Por: Jaime Fernández-Caro Belmonte jim2k2@hotmail.com   ///
///  Prohibida su utilización tanto parcial como total       ///
///  en diseños comerciales                                  ///
////////////////////////////////////////////////////////////////

#define LCD_X_OFFSET    0
#define LCD_Y_OFFSET    0
#define LCD_WIDTH               (131)     ///< in pixels
#define LCD_HEIGHT              (131)     ///< in pixels
#define LCD_COLORS              (4096)    ///< 12 bits

#define LCD6100		1
#define LCD6610		2
#define PCF8833		3

// FONT6x8
// FONT8x8
// FONT8x16

// backlight control
#define BKLGHT_LCD_ON 1
#define BKLGHT_LCD_OFF 2

// Booleans
#define NOFILL 0
#define FILL 1

// 12-bit color definitions
#define WHITE 0xFFF
#define BLACK 0x000
#define LCD_COLOR_BLACK	BLACK
#define LCD_COLOR_WHITE WHITE
#define RED 0xF00
#define GREEN 0x0F0
#define BLUE 0x00F
#define CYAN 0x0FF
#define MAGENTA 0xF0F
#define YELLOW 0xFF0
#define BROWN 0xB22
#define ORANGE 0xFA0
#define PINK 0xF6A

#define LCD_DEFAULT_FG_COLOR    LCD_COLOR_BLACK   // black
#define LCD_DEFAULT_BG_COLOR    LCD_COLOR_WHITE   // white

// Fonts sizes
#define SMALL 0
#define MEDIUM 1
#define LARGE 2

//
void InitLcd(unsigned char type, const unsigned char* device);
void WriteSpiCommand(unsigned char cmd);
void WriteSpiData( unsigned char data);
//
void LCDClearScreen(void);
void LCDSetPixel(unsigned char x, unsigned char y, unsigned int color);
void LCDDrawScreen(unsigned int color);
void LCDPutStr(const char *pString, unsigned char x, unsigned char y, unsigned char Size, unsigned int fColor, unsigned int bColor);
void LCDPutChar(char c, unsigned char x, unsigned char y, unsigned char size, unsigned int fColor, unsigned int bColor);
void LCDWrite130x130bmp (const unsigned char *bmp);
int LCDWritebmp (const char *filename, unsigned char x, unsigned char y, unsigned char w, unsigned char h);
int LCDMovieRGB (const char *filename);
int LCDWriteFullScreenRGB (const char *filename);
int LCDWriteFullScreenRGB_2 (const char *filename);
int LCDWriteRGB (const char *filename);
void LCDWrite64x64bmp (unsigned char x, unsigned char y, const unsigned char *bmp);
void LCDWrite32x32bmp (unsigned char x, unsigned char y, const unsigned char *bmp);
//
void LCDSetCurr_XY( unsigned char x, unsigned char y);
unsigned char LCDGetCurr_X( void);
unsigned char LCDGetCurr_Y( void);
void LCDPrint(const char *pString, unsigned char Size, unsigned int fColor, unsigned int bColor);

