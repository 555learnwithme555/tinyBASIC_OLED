////////////////////////////////////////////////////////////////////////////////
// TinyBasic Plus
////////////////////////////////////////////////////////////////////////////////
//
// Authors: Mike Field <hamster@snap.net.nz>
//	    Scott Lawrence <yorgle@gmail.com>
//          Dan
//          kodera2t

#define kVersion "v0.15"
// v0.15bis-bis-bis-bis RTC activated. SSD1325 big OLED version
// v0.15bis-bis-bis 30/04/2015 Again using software serial for printer(D21:RX, D22:TX), camer support
// v0.15bis-bis-bis 21/04/2015 Thermal printer QR code printing support by "qprint"
// v0.15bis-bis 18/04/2015 Thermal printer support with command "llist" and "lprint"
// v0.15bis 23/02/2015
//     SSD1306 compatible OLED (128x64 dots) with SD card supported.

// v0.15  02/02/2015
//      SSD1306 compatible OLED/LCD (128x64 dots) support is added, using Adafruit_GFX lib. 
//      Frame buffer for 21x8 characters is added.
//      Along this support, TVout and PS2 keyboard support is subtracted, assuming
//      re-programmed Xbox360 or serial console usage for input device.
//      (you can add PS/2, if you need) I refered the source code by Ben Heck's
//      Retro Basic Computer. 
//      GRAPHIC related BASIC commands (currently supporting only direct command): 
//      CLS, LINE x0,y0,x1,y1 CIRCLE x,y,radius FILLCIRCLE x,y,radius ROUNDRECT x,y,w,h,r
//      FILLROUNDRECT x,y,w,h,r are added just for graphic test..
// v0.14: 19/04/2014
//      TVout and PS2uartKeyboard Libraries added.
//
// v0.13: 2013-03-04
//      Support for Arduino 1.5 (SPI.h included, additional changes for DUE support)
//
// v0.12: 2013-03-01
//      EEPROM load and save routines added: EFORMAT, ELIST, ELOAD, ESAVE, ECHAIN
//      added EAUTORUN option (chains to EEProm saved program on startup)
//      Bugfixes to build properly on non-arduino systems (PROGMEM #define workaround)
//      cleaned up a bit of the #define options wrt TONE
//
// v0.11: 2013-02-20
//      all display strings and tables moved to PROGMEM to save space
//      removed second serial
//      removed pinMode completely, autoconf is explicit
//      beginnings of EEPROM related functionality (new,load,save,list)
//
// v0.10: 2012-10-15
//      added kAutoConf, which eliminates the "PINMODE" statement.
//      now, DWRITE,DREAD,AWRITE,AREAD automatically set the PINMODE appropriately themselves.
//      should save a few bytes in your programs.
//
// v0.09: 2012-10-12
//      Fixed directory listings.  FILES now always works. (bug in the SD library)
//      ref: http://arduino.cc/forum/index.php/topic,124739.0.html
//      fixed filesize printouts (added printUnum for unsigned numbers)
//      #defineable baud rate for slow connection throttling
//e
// v0.08: 2012-10-02
//      Tone generation through piezo added (TONE, TONEW, NOTONE)
//
// v0.07: 2012-09-30
//      Autorun buildtime configuration feature
//
// v0.06: 2012-09-27
//      Added optional second serial input, used for an external keyboard
//
// v0.05: 2012-09-21
//      CHAIN to load and run a second file
//      RND,RSEED for random stuff
//      Added "!=" for "<>" synonym
//      Added "END" for "STOP" synonym (proper name for the functionality anyway)
//
// v0.04: 2012-09-20
//      DELAY ms   - for delaying
//      PINMODE <pin>, INPUT|IN|I|OUTPUT|OUT|O
//      DWRITE <pin>, HIGH|HI|1|LOW|LO|0
//      AWRITE <pin>, [0..255]
//      fixed "save" appending to existing files instead of overwriting
// 	Updated for building desktop command line app (incomplete)
//
// v0.03: 2012-09-19
//	Integrated Jurg Wullschleger whitespace,unary fix
//	Now available through github
//	Project renamed from "Tiny Basic in C" to "TinyBasic Plus"
//	   
// v0.02b: 2012-09-17  Scott Lawrence <yorgle@gmail.com>
// 	Better FILES listings
//
// v0.02a: 2012-09-17  Scott Lawrence <yorgle@gmail.com>
// 	Support for SD Library
// 	Added: SAVE, FILES (mostly works), LOAD (mostly works) (redirects IO)
// 	Added: MEM, ? (PRINT)
// 	Quirk:  "10 LET A=B+C" is ok "10 LET A = B + C" is not.
// 	Quirk:  INPUT seems broken?

/*
* Additional Libraries
*/
//// TVout
//#include <TVout.h>
//#include <video_gen.h>
//#include <font4x6.h>
//#include <font6x8.h>
//#include <font8x8.h>
//#include <font8x8ext.h>
//#include <fontALL.h>
//// PS2 Keyboard
//#include <PS2uartKeyboard.h>
//
//PS2uartKeyboard keyboard;
//TVout TV;

//#ifdef ENABLE_FILEIO
#include <Adafruit_VC0706.h>
#include <SD.h>
#include <Servo.h>
Servo servo;
int pos = 0;
int sounds[] = {500,500,500,1000};
int v = 0;
///// ver 1.0 12, ver 2.0 13
#define kSD_CS 13
#define kSD_Fail  0
#define kSD_OK    1
File fp;
//#endif
/////// QR code support/////////
/// This code is using software serial , thermalPrinter for printer output...
#include "qrprint.h"
#include <SoftwareSerial.h>
const byte pin = 22;                      // the pin that will be sending signals to the thermalPrinterPrinter printer (connected to printer's rx)
const byte printHeat = 8;                // 7 is the printer default. Controls number of heating dots, higher = hotter, darker, and more current draw
const byte printSpeed = 110;             // 80 is the printer default. Controls speed of printing (and darkness) higher = slower
SoftwareSerial thermalPrinter(90, pin);  // 
char mes[140];
int menum=0;

///// Adafruit serial JPEG camera is connected to hardware serial of ATmega1284P, 
//////D10(RX1) and D11(TX1)
Adafruit_VC0706 cam = Adafruit_VC0706(&Serial1);
boolean cameracheck;

//////////RTC (PCF2129)////////////////
#define PCF2129address 0xA2>>1
#include <string.h> 
#include <string.h> 
#define ASC8x16S 255968
#define ASC8x16N 257504
#define kanji_CS 4 
byte rawdata[32];
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
String days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };


///////jpeg decoder
#include "JPEGDecoder.h"
char filename[] = "TEST.JPG";
char  pic[80][60];
    uint8 *pImg;
    int x,y,bx,by,q=0;
    int pinum,val,valmax;
    int i,j,k;
//////////////// Newly added for graphic OLED(LCD) 128x64 support/////////////
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
///small OLED///
///#include <Adafruit_SSD1306.h>
//big OLED
#include <Adafruit_SSD1325.h>
// SPI PIN definition. Change to fit your board....
#define OLED_MOSI   5
#define OLED_CLK   7
#define OLED_DC    19
#define OLED_CS    18
#define OLED_RESET 20
//Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
//Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);
Adafruit_SSD1325 display(OLED_DC, OLED_RESET, OLED_CS);
//Adafruit_SSD1325 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
int screenMem[168]; //the implementation of frame buffer is referenced from Ben Heck's
int cursorX = 0;    //Retro BASIC computer's source
int checkChar = 0;

//////////////////////////////////////////////////////////////////////////////

// IF testing with Visual C, this needs to be the first thing in the file.
//#include "stdafx.h"


//char eliminateCompileErrors = 1;  // fix to suppress arduino build errors

// hack to let makefiles work with this file unchanged
#ifdef FORCE_DESKTOP 
#undef ARDUINO
#else
#define ARDUINO 1
#endif

////////////////////////////////////////////////////////////////////////////////
// Feature option configuration...

// This enables LOAD, SAVE, FILES commands through the Arduino SD Library
// it adds 9k of usage as well.
#define ENABLE_FILEIO 1
//#undef ENABLE_FILEIO

// this turns on "autorun".  if there's FileIO, and a file "autorun.bas",
// then it will load it and run it when starting up
//#define ENABLE_AUTORUN 1
#undef ENABLE_AUTORUN
// and this is the file that gets run
#define kAutorunFilename  "autorun.bas"

// this is the alternate autorun.  Autorun the program in the eeprom.
// it will load whatever is in the EEProm and run it
//#define ENABLE_EAUTORUN 1
#undef ENABLE_EAUTORUN

// this will enable the "TONE", "NOTONE" command using a piezo
// element on the specified pin.  Wire the red/positive/piezo to the kPiezoPin,
// and the black/negative/metal disc to ground.
// it adds 1.5k of usage as well.
//#define ENABLE_TONES 1
#undef ENABLE_TONES
#define kPiezoPin 15

// we can use the EEProm to store a program during powerdown.  This is 
// 1keyboardyte on the '328, and 512 bytes on the '168.  Enabling this here will
// allow for this funcitonality to work.  Note that this only works on AVR
// arduino.  Disable it for DUE/other devices.
#define ENABLE_EEPROM 1
//#undef ENABLE_EEPROM

// Sometimes, we connect with a slower device as the console.
// Set your console D0/D1 baud rate here (4800 baud default)
// 4800 bps is default speed of re-programed XBox360 chatpad
#define kConsoleBaud 4800

////////////////////////////////////////////////////////////////////////////////
#ifdef ARDUINO
#ifndef RAMEND
// okay, this is a hack for now
// if we're in here, we're a DUE probably (ARM instead of AVR)

#define RAMEND 4096-1

// turn off EEProm
#undef ENABLE_EEPROM
#undef ENABLE_TONES

#else
// we're an AVR!

// we're moving our data strings into progmem
#include <avr/pgmspace.h>
#endif

// includes, and settings for Arduino-specific functionality
#ifdef ENABLE_EEPROM
#include <EEPROM.h>  /* NOTE: case sensitive */
int eepos = 0;
#endif





/////define graphic////////
#define ENABLE_GRAPHIC 1

// set up our RAM buffer size for program and user input
// NOTE: This number will have to change if you include other libraries.
#ifdef ARDUINO
#ifdef ENABLE_FILEIO
#define kRamFileIO (1100) /* approximate */
#else
#define kRamFileIO (0)
#endif
#ifdef ENABLE_TONES
#define kRamTones (40)
#else
#define kRamTones (0)
#endif
#endif /* ARDUINO */
//#define kRamSize  (RAMEND - 8500 - kRamFileIO - kRamTones) 
#define kRamSize 3500
#ifndef ARDUINO
// Not arduino setup
#include <stdio.h>
#include <stdlib.h>
#undef ENABLE_TONES

// size of our program ram
#define kRamSize   4096 /* arbitrary */

#ifdef ENABLE_FILEIO
FILE * fp;
#endif
#endif

#ifdef ENABLE_FILEIO
// functions defined elsehwere
void cmd_Files( void );
#endif

////////////////////

#ifndef boolean 
#define boolean int
#define true 1
#define false 0
#endif
#endif

#ifndef byte
typedef unsigned char byte;
#endif

// some catches for AVR based text string stuff...
#ifndef PROGMEM
#define PROGMEM
#endif
#ifndef pgm_read_byte
#define pgm_read_byte( A ) *(A)
#endif

////////////////////

#ifdef ENABLE_FILEIO
unsigned char * filenameWord(void);
static boolean sd_is_initialized = false;
#endif

boolean inhibitOutput = false;
static boolean runAfterLoad = false;
static boolean triggerRun = false;

// these will select, at runtime, where IO happens through for load/save
enum {
  kStreamSerial = 0,
  kStreamEEProm,
  kStreamFile
};
static unsigned char inStream = kStreamSerial;
static unsigned char outStream = kStreamSerial;


////////////////////////////////////////////////////////////////////////////////
// ASCII Characters
#define CR	'\r'
#define NL	'\n'
#define LF      0x0a
#define TAB	'\t'
#define BELL	'\b'
#define SPACE   ' '
#define SQUOTE  '\''
#define DQUOTE  '\"'
#define CTRLC	0x1B  // Changed to ESC key (27 - 0x1B)
#define CTRLH	0x08
#define CTRLS	0x13
#define CTRLX	0x18

typedef short unsigned LINENUM;
#ifdef ARDUINO
#define ECHO_CHARS 1
#else
#define ECHO_CHARS 0
#endif


static unsigned char program[kRamSize];
static const char *  sentinel = "HELLO";
static unsigned char *txtpos,*list_line;
static unsigned char expression_error;
static unsigned char *tempsp;

/***********************************************************/
// Keyword table and constants - the last character has 0x80 added to it
const unsigned char keywords[] PROGMEM = {
  'L','I','S','T'+0x80,
  'L','L','I','S','T'+0x80,
  'L','O','A','D'+0x80,
  'N','E','W'+0x80,
  'R','U','N'+0x80,
  'S','A','V','E'+0x80,
  'N','E','X','T'+0x80,
  'L','E','T'+0x80,
  'I','F'+0x80,
  'G','O','T','O'+0x80,
  'G','O','S','U','B'+0x80,
  'R','E','T','U','R','N'+0x80,
  'R','E','M'+0x80,
  'F','O','R'+0x80,
  'I','N','P','U','T'+0x80,
  'P','R','I','N','T'+0x80,
  'L','P','R','I','N','T'+0x80,
  'Q','P','R','I','N','T'+0x80,
  'P','O','K','E'+0x80,
  'S','T','O','P'+0x80,
  'B','Y','E'+0x80,
  'F','I','L','E','S'+0x80,
  'M','E','M'+0x80,
  '?'+ 0x80,
  '\''+ 0x80,
  'A','W','R','I','T','E'+0x80,
  'D','W','R','I','T','E'+0x80,
  'D','E','L','A','Y'+0x80,
  'E','N','D'+0x80,
  'R','S','E','E','D'+0x80,
  'C','H','A','I','N'+0x80,
#ifdef ENABLE_TONES
  'T','O','N','E','W'+0x80,
  'T','O','N','E'+0x80,
  'N','O','T','O','N','E'+0x80,
#endif
#ifdef ARDUINO
#ifdef ENABLE_EEPROM
  'E','C','H','A','I','N'+0x80,
  'E','L','I','S','T'+0x80,
  'E','L','O','A','D'+0x80,
  'E','F','O','R','M','A','T'+0x80,
  'E','S','A','V','E'+0x80,
#endif

#ifdef ENABLE_GRAPHIC
  'C','L','S'+0x80,
  'L','I','N','E'+0x80,
  'C','I','R','C','L','E'+0x80,
  'F','I','L','L','C','I','R','C','L','E'+0x80,
  'R','O','U','N','D','R','E','C','T'+0x80,
  'F','I','L','L','R','O','U','N','D','R','E','C','T'+0x80,
  'T','R','I','A','N','G','L','E'+0x80,
  'F','I','L','L','T','R','I','A','N','G','L','E'+0x80,  
#endif

  'A','S','N','A','P'+0x80,
  'S','N','A','P'+0x80,
  'S','E','R','V','O'+0x80,
  'N','O','T','I','C','E'+0x80,
  
  ////RTC related commands
  'T','I','M','E'+0x80,
  'D','A','T','E'+0x80,
  'B','T','I','M','E'+0x80,
  'B','D','A','T','E'+0x80,
  'J','D','A','T','E'+0x80,

#endif
  0
};

// by moving the command list to an enum, we can easily remove sections 
// above and below simultaneously to selectively obliterate functionality.
enum {
  KW_LIST = 0,
  KW_LLIST,
  KW_LOAD, KW_NEW, KW_RUN, KW_SAVE,
  KW_NEXT, KW_LET, KW_IF,
  KW_GOTO, KW_GOSUB, KW_RETURN,
  KW_REM,
  KW_FOR,
  KW_INPUT, KW_PRINT, KW_LPRINT, KW_QPRINT,
  KW_POKE,
  KW_STOP, KW_BYE,
  KW_FILES,
  KW_MEM,
  KW_QMARK, KW_QUOTE,
  KW_AWRITE, KW_DWRITE,
  KW_DELAY,
  KW_END,
  KW_RSEED,
  KW_CHAIN,
#ifdef ENABLE_TONES
  KW_TONEW, KW_TONE, KW_NOTONE,
#endif
#ifdef ARDUINO
#ifdef ENABLE_EEPROM
  KW_ECHAIN, KW_ELIST, KW_ELOAD, KW_EFORMAT, KW_ESAVE,
 
#endif

#ifdef ENABLE_GRAPHIC
  KW_CLS, KW_LINE, KW_CIRCLE, KW_FILLCIRCLE, KW_ROUNDRECT, KW_FILLROUNDRECT,
  KW_TRIANGLE,KW_FILLTRIANGLE,
#endif

  KW_ASNAP,KW_SNAP,
  KW_SERVO,KW_NOTICE,
  
  ///RTC related commands
  KW_TIME,KW_DATE,KW_BTIME,KW_BDATE,KW_JDATE,
#endif
  KW_DEFAULT /* always the final one*/
};

struct stack_for_frame {
  char frame_type;
  char for_var;
  short int terminal;
  short int step;
  unsigned char *current_line;
  unsigned char *txtpos;
};

struct stack_gosub_frame {
  char frame_type;
  unsigned char *current_line;
  unsigned char *txtpos;
};

const unsigned char func_tab[] PROGMEM = {
  'P','E','E','K'+0x80,
  'A','B','S'+0x80,
  'A','R','E','A','D'+0x80,
  'D','R','E','A','D'+0x80,
  'R','N','D'+0x80,
  0
};
#define FUNC_PEEK    0
#define FUNC_ABS     1
#define FUNC_AREAD   2
#define FUNC_DREAD   3
#define FUNC_RND     4
#define FUNC_UNKNOWN 5

const unsigned char to_tab[] PROGMEM = {
  'T','O'+0x80,
  0
};

const unsigned char step_tab[] PROGMEM = {
  'S','T','E','P'+0x80,
  0
};

const unsigned char relop_tab[] PROGMEM = {
  '>','='+0x80,
  '<','>'+0x80,
  '>'+0x80,
  '='+0x80,
  '<','='+0x80,
  '<'+0x80,
  '!','='+0x80,
  0
};

#define RELOP_GE		0
#define RELOP_NE		1
#define RELOP_GT		2
#define RELOP_EQ		3
#define RELOP_LE		4
#define RELOP_LT		5
#define RELOP_NE_BANG		6
#define RELOP_UNKNOWN	7

const unsigned char highlow_tab[] PROGMEM = { 
  'H','I','G','H'+0x80,
  'H','I'+0x80,
  'L','O','W'+0x80,
  'L','O'+0x80,
  0
};
#define HIGHLOW_HIGH    1
#define HIGHLOW_UNKNOWN 4

#define STACK_SIZE (sizeof(struct stack_for_frame)*5)
#define VAR_SIZE sizeof(short int) // Size of variables in bytes

static unsigned char *stack_limit;
static unsigned char *program_start;
static unsigned char *program_end;
static unsigned char *stack; // Software stack for things that should go on the CPU stack
static unsigned char *variables_begin;
static unsigned char *current_line;
static unsigned char *sp1;
#define STACK_GOSUB_FLAG 'G'
#define STACK_FOR_FLAG 'F'
static unsigned char table_index;
static LINENUM linenum;

static const unsigned char okmsg[]            PROGMEM = "READY";
static const unsigned char whatmsg[]          PROGMEM = "What? ";
static const unsigned char howmsg[]           PROGMEM =	"How?";
static const unsigned char sorrymsg[]         PROGMEM = "Sorry!";
static const unsigned char initmsg[]          PROGMEM = "TinyBasic Plus " kVersion;
static const unsigned char memorymsg[]        PROGMEM = " bytes free.";
#ifdef ARDUINO
#ifdef ENABLE_EEPROM
static const unsigned char eeprommsg[]        PROGMEM = " bytes EEProm";
static const unsigned char eepromamsg[]       PROGMEM = " EEProm bytes available.";
#endif
#endif
static const unsigned char breakmsg[]         PROGMEM = "break!";
static const unsigned char unimplimentedmsg[] PROGMEM = "Unimplemented";
static const unsigned char backspacemsg[]     PROGMEM = "\b \b";
static const unsigned char indentmsg[]        PROGMEM = "    ";
static const unsigned char sderrormsg[]       PROGMEM = "SD card error.";
static const unsigned char sdfilemsg[]        PROGMEM = "SD file error.";
static const unsigned char dirextmsg[]        PROGMEM = "(dir)";
static const unsigned char slashmsg[]         PROGMEM = "/";
static const unsigned char spacemsg[]         PROGMEM = " ";

static const unsigned char cameraok[]         PROGMEM = "Adafruit Camera OK";
static const unsigned char camerang[]         PROGMEM = "Camera not found";
static const unsigned char picmsg[]         PROGMEM = "Snap in 2 secs...";
static const unsigned char pictaken[]         PROGMEM = "Picture taken!";
static const unsigned char picfail[]         PROGMEM = "Failed to snap!";
static const unsigned char picstore[]         PROGMEM = "Storing ";
static const unsigned char picstoremsg[]         PROGMEM = " byte image.";
static const unsigned char picdone[]         PROGMEM = "pic transfer done!";

static int inchar(void);
static void outchar(unsigned char c);
static void line_terminator(void);
static short int expression(void);
const unsigned char breakcheck(void);
/***************************************************************************/
static void ignore_blanks(void)
{
  while(*txtpos == SPACE || *txtpos == TAB)
    txtpos++;
}


/***************************************************************************/
static void scantable(const unsigned char *table)
{
  int i = 0;
  table_index = 0;
  while(1)
  {
    // Run out of table entries?
    if(pgm_read_byte( table ) == 0)
      return;

    // Do we match this character?
    if(txtpos[i] == pgm_read_byte( table ))
    {
      i++;
      table++;
    }
    else
    {
      // do we match the last character of keywork (with 0x80 added)? If so, return
      if(txtpos[i]+0x80 == pgm_read_byte( table ))
      {
        txtpos += i+1;  // Advance the pointer to following the keyword
        ignore_blanks();
        return;
      }

      // Forward to the end of this keyword
      while((pgm_read_byte( table ) & 0x80) == 0)
        table++;

      // Now move on to the first character of the next word, and reset the position index
      table++;
      table_index++;
      ignore_blanks();
      i = 0;
    }
  }
}

/***************************************************************************/
static void pushb(const unsigned char b)
{
  sp1--;
  *sp1 = b;
}

/***************************************************************************/
static unsigned char popb()
{
  unsigned char b;
  b = *sp1;
  sp1++;
  return b;
}

/***************************************************************************/
void printnum(int num)
{
  int digits = 0;

  if(num < 0)
  {
    num = -num;
    outchar('-');
  }
  do {
    pushb(num%10+'0');
    num = num/10;
    digits++;
  }
  while (num > 0);

  while(digits > 0)
  {
    outchar(popb());
    digits--;
  }
}

void lprintnum(int num)
{
  int digits = 0;

  if(num < 0)
  {
    num = -num;
    loutchar('-');
  }
  do {
    pushb(num%10+'0');
    num = num/10;
    digits++;
  }
  while (num > 0);

  while(digits > 0)
  {
    loutchar(popb());
    digits--;
  }
}

void printUnum(unsigned int num)
{
  int digits = 0;

  do {
    pushb(num%10+'0');
    num = num/10;
    digits++;
  }
  while (num > 0);

  while(digits > 0)
  {
    outchar(popb());
    digits--;
  }
}

/***************************************************************************/
static unsigned short testnum(void)
{
  unsigned short num = 0;
  ignore_blanks();

  while(*txtpos>= '0' && *txtpos <= '9' )
  {
    // Trap overflows
    if(num >= 0xFFFF/10)
    {
      num = 0xFFFF;
      break;
    }

    num = num *10 + *txtpos - '0';
    txtpos++;
  }
  return	num;
}

/***************************************************************************/
static unsigned char print_quoted_string(void)
{
  int i=0;
  unsigned char delim = *txtpos;
  if(delim != '"' && delim != '\'')
    return 0;
  txtpos++;

  // Check we have a closing delimiter
  while(txtpos[i] != delim)
  {
    if(txtpos[i] == NL)
      return 0;
    i++;
  }

  // Print the characters
  while(*txtpos != delim)
  {
    outchar(*txtpos);
    txtpos++;
  }
  txtpos++; // Skip over the last delimiter

  return 1;
}

static unsigned char lprint_quoted_string(void)
{
  int i=0;
  unsigned char delim = *txtpos;
  if(delim != '"' && delim != '\'')
    return 0;
  txtpos++;

  // Check we have a closing delimiter
  while(txtpos[i] != delim)
  {
    if(txtpos[i] == NL)
      return 0;
    i++;
  }

  // Print the characters
  while(*txtpos != delim)
  {
    loutchar(*txtpos);
    txtpos++;
  }
  txtpos++; // Skip over the last delimiter

  return 1;
}

/***************************************************************************/
void printmsgNoNL(const unsigned char *msg)
{
  while( pgm_read_byte( msg ) != 0 ) {
    outchar( pgm_read_byte( msg++ ) );
  };
}

/***************************************************************************/
void printmsg(const unsigned char *msg)
{
  printmsgNoNL(msg);
  line_terminator();
}

/***************************************************************************/
static void getln(char prompt)
{
  outchar(prompt);
  txtpos = program_end+sizeof(LINENUM);

  while(1)
  {
    char c = inchar();
    switch(c)
    {
    case NL:
      //break;
    case CR:
      line_terminator();
      // Terminate all strings with a NL
      txtpos[0] = NL;
      return;
    case CTRLH:
      if(txtpos == program_end)
        break;
      txtpos--;

      printmsg(backspacemsg);
      break;
    default:
      // We need to leave at least one space to allow us to shuffle the line into order
      if(txtpos == variables_begin-2)
        outchar(BELL);
      else
      {
        txtpos[0] = c;
        txtpos++;
        outchar(c);
      }
    }
  }
}

/***************************************************************************/
static unsigned char *findline(void)
{
  unsigned char *line = program_start;
  while(1)
  {
    if(line == program_end)
      return line;

    if(((LINENUM *)line)[0] >= linenum)
      return line;

    // Add the line length onto the current address, to get to the next line;
    line += line[sizeof(LINENUM)];
  }
}

/***************************************************************************/
static void toUppercaseBuffer(void)
{
  unsigned char *c = program_end+sizeof(LINENUM);
  unsigned char quote = 0;

  while(*c != NL)
  {
    // Are we in a quoted string?
    if(*c == quote)
      quote = 0;
    else if(*c == '"' || *c == '\'')
      quote = *c;
    else if(quote == 0 && *c >= 'a' && *c <= 'z')
      *c = *c + 'A' - 'a';
    c++;
  }
}

/***************************************************************************/
void printline()
{
  LINENUM line_num;

  line_num = *((LINENUM *)(list_line));
  list_line += sizeof(LINENUM) + sizeof(char);

  // Output the line */
  printnum(line_num);
  outchar(' ');
  while(*list_line != NL)
  {
    outchar(*list_line);
    list_line++;
  }
  list_line++;
  line_terminator();
}
/////////////////
void lprintline()
{
  LINENUM line_num;

  line_num = *((LINENUM *)(list_line));
  list_line += sizeof(LINENUM) + sizeof(char);

  // Output the line */
  lprintnum(line_num);
  loutchar(' ');
  while(*list_line != NL)
  {
    loutchar(*list_line);
    list_line++;
  }
  list_line++;
  l_line_terminator();
}
//////////////////
/***************************************************************************/
static short int expr4(void)
{
  // fix provided by Jurg Wullschleger wullschleger@gmail.com
  // fixes whitespace and unary operations
  ignore_blanks();

  if( *txtpos == '-' ) {
    txtpos++;
    return -expr4();
  }
  // end fix

  if(*txtpos == '0')
  {
    txtpos++;
    return 0;
  }

  if(*txtpos >= '1' && *txtpos <= '9')
  {
    short int a = 0;
    do 	{
      a = a*10 + *txtpos - '0';
      txtpos++;
    } 
    while(*txtpos >= '0' && *txtpos <= '9');
    return a;
  }

  // Is it a function or variable reference?
  if(txtpos[0] >= 'A' && txtpos[0] <= 'Z')
  {
    short int a;
    // Is it a variable reference (single alpha)
    if(txtpos[1] < 'A' || txtpos[1] > 'Z')
    {
      a = ((short int *)variables_begin)[*txtpos - 'A'];
      txtpos++;
      return a;
    }

    // Is it a function with a single parameter
    scantable(func_tab);
    if(table_index == FUNC_UNKNOWN)
      goto expr4_error;

    unsigned char f = table_index;

    if(*txtpos != '(')
      goto expr4_error;

    txtpos++;
    a = expression();
    if(*txtpos != ')')
      goto expr4_error;
    txtpos++;
    switch(f)
    {
    case FUNC_PEEK:
      return program[a];
      
    case FUNC_ABS:
      if(a < 0) 
        return -a;
      return a;

#ifdef ARDUINO
    case FUNC_AREAD:
      pinMode( a, INPUT );
      return analogRead( a );                        
    case FUNC_DREAD:
      pinMode( a, INPUT );
      return digitalRead( a );
#endif

    case FUNC_RND:
#ifdef ARDUINO
      return( random( a ));
#else
      return( rand() % a );
#endif
    }
  }

  if(*txtpos == '(')
  {
    short int a;
    txtpos++;
    a = expression();
    if(*txtpos != ')')
      goto expr4_error;

    txtpos++;
    return a;
  }

expr4_error:
  expression_error = 1;
  return 0;

}

/***************************************************************************/
static short int expr3(void)
{
  short int a,b;

  a = expr4();

  ignore_blanks(); // fix for eg:  100 a = a + 1

  while(1)
  {
    if(*txtpos == '*')
    {
      txtpos++;
      b = expr4();
      a *= b;
    }
    else if(*txtpos == '/')
    {
      txtpos++;
      b = expr4();
      if(b != 0)
        a /= b;
      else
        expression_error = 1;
    }
    else
      return a;
  }
}

/***************************************************************************/
static short int expr2(void)
{
  short int a,b;

  if(*txtpos == '-' || *txtpos == '+')
    a = 0;
  else
    a = expr3();

  while(1)
  {
    if(*txtpos == '-')
    {
      txtpos++;
      b = expr3();
      a -= b;
    }
    else if(*txtpos == '+')
    {
      txtpos++;
      b = expr3();
      a += b;
    }
    else
      return a;
  }
}
/***************************************************************************/
static short int expression(void)
{
  short int a,b;

  a = expr2();

  // Check if we have an error
  if(expression_error)	return a;

  scantable(relop_tab);
  if(table_index == RELOP_UNKNOWN)
    return a;

  switch(table_index)
  {
  case RELOP_GE:
    b = expr2();
    if(a >= b) return 1;
    break;
  case RELOP_NE:
  case RELOP_NE_BANG:
    b = expr2();
    if(a != b) return 1;
    break;
  case RELOP_GT:
    b = expr2();
    if(a > b) return 1;
    break;
  case RELOP_EQ:
    b = expr2();
    if(a == b) return 1;
    break;
  case RELOP_LE:
    b = expr2();
    if(a <= b) return 1;
    break;
  case RELOP_LT:
    b = expr2();
    if(a < b) return 1;
    break;
  }
  return 0;
}

/***************************************************************************/
void loop()
{
  unsigned char *start;
  unsigned char *newEnd;
  unsigned char linelen;
  boolean isDigital;
  boolean alsoWait = false;
  int val;

#ifdef ARDUINO
#ifdef ENABLE_TONES
  noTone( kPiezoPin );
#endif
#endif

  program_start = program;
  program_end = program_start;
  sp1 = program+sizeof(program);  // Needed for printnum
  stack_limit = program+sizeof(program)-STACK_SIZE;
  variables_begin = stack_limit - 27*VAR_SIZE;
  printmsg(initmsg);
  // memory free
  printnum(variables_begin-program_end);
  printmsg(memorymsg);
#ifdef ARDUINO
#ifdef ENABLE_EEPROM
  // eprom size
  printnum( E2END+1 );
  printmsg( eeprommsg );
#endif /* ENABLE_EEPROM */
#endif /* ARDUINO */

warmstart:
  // this signifies that it is running in 'direct' mode.
  current_line = 0;
  sp1 = program+sizeof(program);
  printmsg(okmsg);

prompt:
  if( triggerRun ){
    triggerRun = false;
    current_line = program_start;
    goto execline;
  }

  getln( '>' );
  toUppercaseBuffer();

  txtpos = program_end+sizeof(unsigned short);

  // Find the end of the freshly entered line
  while(*txtpos != NL)
    txtpos++;

  // Move it to the end of program_memory
  {
    unsigned char *dest;
    dest = variables_begin-1;
    while(1)
    {
      *dest = *txtpos;
      if(txtpos == program_end+sizeof(unsigned short))
        break;
      dest--;
      txtpos--;
    }
    txtpos = dest;
  }

  // Now see if we have a line number
  linenum = testnum();
  ignore_blanks();
  if(linenum == 0)
    goto direct;

  if(linenum == 0xFFFF)
    goto qhow;

  // Find the length of what is left, including the (yet-to-be-populated) line header
  linelen = 0;
  while(txtpos[linelen] != NL)
    linelen++;
  linelen++; // Include the NL in the line length
  linelen += sizeof(unsigned short)+sizeof(char); // Add space for the line number and line length

  // Now we have the number, add the line header.
  txtpos -= 3;
  *((unsigned short *)txtpos) = linenum;
  txtpos[sizeof(LINENUM)] = linelen;


  // Merge it into the rest of the program
  start = findline();

  // If a line with that number exists, then remove it
  if(start != program_end && *((LINENUM *)start) == linenum)
  {
    unsigned char *dest, *from;
    unsigned tomove;

    from = start + start[sizeof(LINENUM)];
    dest = start;

    tomove = program_end - from;
    while( tomove > 0)
    {
      *dest = *from;
      from++;
      dest++;
      tomove--;
    }	
    program_end = dest;
  }

  if(txtpos[sizeof(LINENUM)+sizeof(char)] == NL) // If the line has no txt, it was just a delete
    goto prompt;



  // Make room for the new line, either all in one hit or lots of little shuffles
  while(linelen > 0)
  {	
    unsigned int tomove;
    unsigned char *from,*dest;
    unsigned int space_to_make;

    space_to_make = txtpos - program_end;

    if(space_to_make > linelen)
      space_to_make = linelen;
    newEnd = program_end+space_to_make;
    tomove = program_end - start;


    // Source and destination - as these areas may overlap we need to move bottom up
    from = program_end;
    dest = newEnd;
    while(tomove > 0)
    {
      from--;
      dest--;
      *dest = *from;
      tomove--;
    }

    // Copy over the bytes into the new space
    for(tomove = 0; tomove < space_to_make; tomove++)
    {
      *start = *txtpos;
      txtpos++;
      start++;
      linelen--;
    }
    program_end = newEnd;
  }
  goto prompt;

unimplemented:
  printmsg(unimplimentedmsg);
  goto prompt;

qhow:	
  printmsg(howmsg);
  goto prompt;

qwhat:	
  printmsgNoNL(whatmsg);
  if(current_line != NULL)
  {
    unsigned char tmp = *txtpos;
    if(*txtpos != NL)
      *txtpos = ' ';
    list_line = current_line;
    printline();
    *txtpos = tmp;
  }
  line_terminator();
  goto prompt;

qsorry:	
  printmsg(sorrymsg);
  goto warmstart;

run_next_statement:
  while(*txtpos == ':')
    txtpos++;
  ignore_blanks();
  if(*txtpos == NL)
    goto execnextline;
  goto interperateAtTxtpos;

direct: 
  txtpos = program_end+sizeof(LINENUM);
  if(*txtpos == NL)
    goto prompt;

interperateAtTxtpos:
  if(breakcheck())
  {
    printmsg(breakmsg);
    goto warmstart;
  }

  scantable(keywords);

  switch(table_index)
  {
  case KW_DELAY:
    {
#ifdef ARDUINO
      expression_error = 0;
      val = expression();
      delay( val );
      goto execnextline;
#else
      goto unimplemented;
#endif
    }

  case KW_FILES:
    goto files;
  case KW_LIST:
    goto list;
  case KW_LLIST:
    goto llist;
  case KW_CHAIN:
    goto chain;
  case KW_LOAD:
    goto load;
  case KW_MEM:
    goto mem;
  case KW_NEW:
    if(txtpos[0] != NL)
      goto qwhat;
    program_end = program_start;
    goto prompt;
  case KW_RUN:
    current_line = program_start;
    goto execline;
  case KW_SAVE:
    goto save;
  case KW_NEXT:
    goto next;
  case KW_LET:
    goto assignment;
  case KW_IF:
    short int val;
    expression_error = 0;
    val = expression();
    if(expression_error || *txtpos == NL)
      goto qhow;
    if(val != 0)
      goto interperateAtTxtpos;
    goto execnextline;

  case KW_GOTO:
    expression_error = 0;
    linenum = expression();
    if(expression_error || *txtpos != NL)
      goto qhow;
    current_line = findline();
    goto execline;

  case KW_GOSUB:
    goto gosub;
  case KW_RETURN:
    goto gosub_return; 
  case KW_REM:
  case KW_QUOTE:
    goto execnextline;	// Ignore line completely
  case KW_FOR:
    goto forloop; 
  case KW_INPUT:
    goto input; 
  case KW_PRINT:
  case KW_QMARK:
    goto print;
  case KW_LPRINT:
    goto lprint;
  case KW_QPRINT:
    goto qprint;
  case KW_POKE:
    goto poke;
  case KW_END:
  case KW_STOP:
    // This is the easy way to end - set the current line to the end of program attempt to run it
    if(txtpos[0] != NL)
      goto qwhat;
    current_line = program_end;
    goto execline;
  case KW_BYE:
    // Leave the basic interperater
    return;

  case KW_AWRITE:  // AWRITE <pin>, HIGH|LOW
    isDigital = false;
    goto awrite;
  case KW_DWRITE:  // DWRITE <pin>, HIGH|LOW
    isDigital = true;
    goto dwrite;

  case KW_RSEED:
    goto rseed;

#ifdef ENABLE_TONES
  case KW_TONEW:
    alsoWait = true;
  case KW_TONE:
    goto tonegen;
  case KW_NOTONE:
    goto tonestop;
#endif

#ifdef ENABLE_GRAPHIC
  case KW_CLS:
    goto cls;
  case KW_LINE:
    goto line;
  case KW_CIRCLE:
    goto circle;
  case KW_FILLCIRCLE:
    goto FILLCIRCLE;
  case KW_ROUNDRECT:
    goto ROUNDRECT;
  case KW_FILLROUNDRECT:
    goto FILLROUNDRECT;
  case KW_TRIANGLE:
    goto TRIANGLE;  
  case KW_FILLTRIANGLE:
    goto FILLTRIANGLE;    
#endif

  case KW_ASNAP:
  goto asnap;

  case KW_SNAP:
  goto snap;

  case KW_SERVO:
  goto SERVO;
  case KW_NOTICE:
  goto NOTICE;
  
//////RTC related commands
  case KW_TIME:
  goto TIME;
  case KW_DATE:
  goto DATE;
  case KW_BTIME:
  goto BTIME;
  case KW_BDATE:
  goto BDATE;
  case KW_JDATE:
  goto JDATE;

#ifdef ARDUINO
#ifdef ENABLE_EEPROM
  case KW_EFORMAT:
    goto eformat;
  case KW_ESAVE:
    goto esave;
  case KW_ELOAD:
    goto eload;
  case KW_ELIST:
    goto elist;
  case KW_ECHAIN:
    goto echain;
#endif
#endif

  case KW_DEFAULT:
    goto assignment;
  default:
    break;
  }

execnextline:
  if(current_line == NULL)		// Processing direct commands?
    goto prompt;
  current_line +=	 current_line[sizeof(LINENUM)];

execline:
  if(current_line == program_end) // Out of lines to run
    goto warmstart;
  txtpos = current_line+sizeof(LINENUM)+sizeof(char);
  goto interperateAtTxtpos;

#ifdef ARDUINO
#ifdef ENABLE_EEPROM
elist:
  {
    int i;
    for( i = 0 ; i < (E2END +1) ; i++ )
    {
      val = EEPROM.read( i );

      if( val == '\0' ) {
        goto execnextline;
      }

      if( ((val < ' ') || (val  > '~')) && (val != NL) && (val != CR))  {
        outchar( '?' );
      } 
      else {
        outchar( val );
      }
    }
  }
  goto execnextline;

eformat:
  {
    for( int i = 0 ; i < E2END ; i++ )
    {
      if( (i & 0x03f) == 0x20 ) outchar( '.' );
      EEPROM.write( i, 0 );
    }
    outchar( LF );
  }
  goto execnextline;

esave:
  {
    outStream = kStreamEEProm;
    eepos = 0;

    // copied from "List"
    list_line = findline();
    while(list_line != program_end)
      printline();

    // go back to standard output, close the file
    outStream = kStreamSerial;
    
    goto warmstart;
  }
  
  
echain:
  runAfterLoad = true;

eload:
  // clear the program
  program_end = program_start;

  // load from a file into memory
  eepos = 0;
  inStream = kStreamEEProm;
  inhibitOutput = true;
  goto warmstart;
#endif /* ENABLE_EEPROM */
#endif

input:
  {
    unsigned char var;
    ignore_blanks();
    if(*txtpos < 'A' || *txtpos > 'Z')
      goto qwhat;
    var = *txtpos;
    txtpos++;
    ignore_blanks();
    if(*txtpos != NL && *txtpos != ':')
      goto qwhat;
    ((short int *)variables_begin)[var-'A'] = 99;

    goto run_next_statement;
  }

forloop:
  {
    unsigned char var;
    short int initial, step, terminal;
    ignore_blanks();
    if(*txtpos < 'A' || *txtpos > 'Z')
      goto qwhat;
    var = *txtpos;
    txtpos++;
    ignore_blanks();
    if(*txtpos != '=')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    expression_error = 0;
    initial = expression();
    if(expression_error)
      goto qwhat;

    scantable(to_tab);
    if(table_index != 0)
      goto qwhat;

    terminal = expression();
    if(expression_error)
      goto qwhat;

    scantable(step_tab);
    if(table_index == 0)
    {
      step = expression();
      if(expression_error)
        goto qwhat;
    }
    else
      step = 1;
    ignore_blanks();
    if(*txtpos != NL && *txtpos != ':')
      goto qwhat;


    if(!expression_error && *txtpos == NL)
    {
      struct stack_for_frame *f;
      if(sp1 + sizeof(struct stack_for_frame) < stack_limit)
        goto qsorry;

      sp1 -= sizeof(struct stack_for_frame);
      f = (struct stack_for_frame *)sp1;
      ((short int *)variables_begin)[var-'A'] = initial;
      f->frame_type = STACK_FOR_FLAG;
      f->for_var = var;
      f->terminal = terminal;
      f->step     = step;
      f->txtpos   = txtpos;
      f->current_line = current_line;
      goto run_next_statement;
    }
  }
  goto qhow;

gosub:
  expression_error = 0;
  linenum = expression();
  if(!expression_error && *txtpos == NL)
  {
    struct stack_gosub_frame *f;
    if(sp1 + sizeof(struct stack_gosub_frame) < stack_limit)
      goto qsorry;

    sp1 -= sizeof(struct stack_gosub_frame);
    f = (struct stack_gosub_frame *)sp1;
    f->frame_type = STACK_GOSUB_FLAG;
    f->txtpos = txtpos;
    f->current_line = current_line;
    current_line = findline();
    goto execline;
  }
  goto qhow;

next:
  // Fnd the variable name
  ignore_blanks();
  if(*txtpos < 'A' || *txtpos > 'Z')
    goto qhow;
  txtpos++;
  ignore_blanks();
  if(*txtpos != ':' && *txtpos != NL)
    goto qwhat;

gosub_return:
  // Now walk up the stack frames and find the frame we want, if present
  tempsp = sp1;
  while(tempsp < program+sizeof(program)-1)
  {
    switch(tempsp[0])
    {
    case STACK_GOSUB_FLAG:
      if(table_index == KW_RETURN)
      {
        struct stack_gosub_frame *f = (struct stack_gosub_frame *)tempsp;
        current_line	= f->current_line;
        txtpos			= f->txtpos;
        sp1 += sizeof(struct stack_gosub_frame);
        goto run_next_statement;
      }
      // This is not the loop you are looking for... so Walk back up the stack
      tempsp += sizeof(struct stack_gosub_frame);
      break;
    case STACK_FOR_FLAG:
      // Flag, Var, Final, Step
      if(table_index == KW_NEXT)
      {
        struct stack_for_frame *f = (struct stack_for_frame *)tempsp;
        // Is the the variable we are looking for?
        if(txtpos[-1] == f->for_var)
        {
          short int *varaddr = ((short int *)variables_begin) + txtpos[-1] - 'A'; 
          *varaddr = *varaddr + f->step;
          // Use a different test depending on the sign of the step increment
          if((f->step > 0 && *varaddr <= f->terminal) || (f->step < 0 && *varaddr >= f->terminal))
          {
            // We have to loop so don't pop the stack
            txtpos = f->txtpos;
            current_line = f->current_line;
            goto run_next_statement;
          }
          // We've run to the end of the loop. drop out of the loop, popping the stack
          sp1 = tempsp + sizeof(struct stack_for_frame);
          goto run_next_statement;
        }
      }
      // This is not the loop you are looking for... so Walk back up the stack
      tempsp += sizeof(struct stack_for_frame);
      break;
    default:
      //printf("Stack is stuffed!\n");
      goto warmstart;
    }
  }
  // Didn't find the variable we've been looking for
  goto qhow;

assignment:
  {
    short int value;
    short int *var;

    if(*txtpos < 'A' || *txtpos > 'Z')
      goto qhow;
    var = (short int *)variables_begin + *txtpos - 'A';
    txtpos++;

    ignore_blanks();

    if (*txtpos != '=')
      goto qwhat;
    txtpos++;
    ignore_blanks();
    expression_error = 0;
    value = expression();
    if(expression_error)
      goto qwhat;
    // Check that we are at the end of the statement
    if(*txtpos != NL && *txtpos != ':')
      goto qwhat;
    *var = value;
  }
  goto run_next_statement;
poke:
  {
    short int value;
    unsigned char *address;

    // Work out where to put it
    expression_error = 0;
    value = expression();
    if(expression_error)
      goto qwhat;
    address = (unsigned char *)value;

    // check for a comma
    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    // Now get the value to assign
    expression_error = 0;
    value = expression();
    if(expression_error)
      goto qwhat;
    //printf("Poke %p value %i\n",address, (unsigned char)value);
    // Check that we are at the end of the statement
    if(*txtpos != NL && *txtpos != ':')
      goto qwhat;
  }
  goto run_next_statement;

list:
  linenum = testnum(); // Retuns 0 if no line found.

  // Should be EOL
  if(txtpos[0] != NL)
    goto qwhat;

  // Find the line
  list_line = findline();
  while(list_line != program_end)
    printline();
  goto warmstart;
  
llist:
  linenum = testnum(); // Retuns 0 if no line found.

  // Should be EOL
  if(txtpos[0] != NL)
    goto qwhat;

  // Find the line
  list_line = findline();
  while(list_line != program_end)
  {
    lprintline();
  }
  loutchar(NL);
  loutchar(CR); 
  loutchar(NL);
  loutchar(CR);  
  loutchar(NL);
  loutchar(CR);  
  goto warmstart;

print:
  // If we have an empty list then just put out a NL
  if(*txtpos == ':' )
  {
    line_terminator();
    txtpos++;
    goto run_next_statement;
  }
  if(*txtpos == NL)
  {
    goto execnextline;
  }

  while(1)
  {
    ignore_blanks();
    if(print_quoted_string())
    {
      ;
    }
    else if(*txtpos == '"' || *txtpos == '\'')
      goto qwhat;
    else
    {
      short int e;
      expression_error = 0;
      e = expression();
      if(expression_error)
        goto qwhat;
      printnum(e);
    }

    // At this point we have three options, a comma or a new line
    if(*txtpos == ',')
      txtpos++;	// Skip the comma and move onto the next
    else if(txtpos[0] == ';' && (txtpos[1] == NL || txtpos[1] == ':'))
    {
      txtpos++; // This has to be the end of the print - no newline
      break;
    }
    else if(*txtpos == NL || *txtpos == ':')
    {
      line_terminator();	// The end of the print statement
      break;
    }
    else
      goto qwhat;	
  }
  goto run_next_statement;

lprint:
  // If we have an empty list then just put out a NL
  if(*txtpos == ':' )
  {
    l_line_terminator();
    txtpos++;
    goto run_next_statement;
  }
  if(*txtpos == NL)
  {
    goto execnextline;
  }

  while(1)
  {
    ignore_blanks();
    if(lprint_quoted_string())
    {
      ;
    }
    else if(*txtpos == '"' || *txtpos == '\'')
      goto qwhat;
    else
    {
      short int e;
      expression_error = 0;
      e = expression();
      if(expression_error)
        goto qwhat;
      lprintnum(e);
    }

    // At this point we have three options, a comma or a new line
    if(*txtpos == ',')
      txtpos++;	// Skip the comma and move onto the next
    else if(txtpos[0] == ';' && (txtpos[1] == NL || txtpos[1] == ':'))
    {
      txtpos++; // This has to be the end of the print - no newline
      break;
    }
    else if(*txtpos == NL || *txtpos == ':')
    {
      l_line_terminator();	// The end of the print statement
      break;
    }
    else
      goto qwhat;	
  }
  goto run_next_statement;

qprint:
{
    menum=0;
    char *c1;
        for(int i=0; i<80; i++){
          mes[i]=0;
        }
  // If we have an empty list then just put out a NL
  if(*txtpos == ':' )
  {
    line_terminator();
    txtpos++;
    goto run_next_statement;
  }
  if(*txtpos == NL)
  {
    goto execnextline;
  }

  while(1)
  {
    ignore_blanks();
    if(qr_quoted_string())
    {
      ;
    }
    else if(*txtpos == '"' || *txtpos == '\'')
      goto qwhat;
    else
    {
      short int e;
      expression_error = 0;
      e = expression();
      if(expression_error)
        goto qwhat;
      printnum(e);
    }

    // At this point we have three options, a comma or a new line
    if(*txtpos == ',')
      txtpos++;	// Skip the comma and move onto the next
    else if(txtpos[0] == ';' && (txtpos[1] == NL || txtpos[1] == ':'))
    {
      txtpos++; // This has to be the end of the print - no newline
      break;
    }
    else if(*txtpos == NL || *txtpos == ':')
    {
      line_terminator();	// The end of the print statement
      break;
    }
    else
      goto qwhat;	
  }
  for (int i=0; i<menum;i++){   
//  outchar(mes[i]);
  c1=mes;
  }
 qoutchar(c1);
  goto run_next_statement;
}

mem:
  // memory free
  printnum(variables_begin-program_end);
  printmsg(memorymsg);
#ifdef ARDUINO
#ifdef ENABLE_EEPROM
  {
    // eprom size
    printnum( E2END+1 );
    printmsg( eeprommsg );
    
    // figure out the memory usage;
    val = ' ';
    int i;   
    for( i=0 ; (i<(E2END+1)) && (val != '\0') ; i++ ) {
      val = EEPROM.read( i );    
    }
    printnum( (E2END +1) - (i-1) );
    
    printmsg( eepromamsg );
  }
#endif /* ENABLE_EEPROM */
#endif /* ARDUINO */
  goto run_next_statement;


  /*************************************************/

#ifdef ARDUINO
awrite: // AWRITE <pin>,val
dwrite:
  {
    short int pinNo;
    short int value;
    unsigned char *txtposBak;

    // Get the pin number
    expression_error = 0;
    pinNo = expression();
    if(expression_error)
      goto qwhat;

    // check for a comma
    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();


    txtposBak = txtpos; 
    scantable(highlow_tab);
    if(table_index != HIGHLOW_UNKNOWN)
    {
      if( table_index <= HIGHLOW_HIGH ) {
        value = 1;
      } 
      else {
        value = 0;
      }
    } 
    else {

      // and the value (numerical)
      expression_error = 0;
      value = expression();
      if(expression_error)
        goto qwhat;
    }
    pinMode( pinNo, OUTPUT );
    if( isDigital ) {
      digitalWrite( pinNo, value );
    } 
    else {
      analogWrite( pinNo, value );
    }
  }
  goto run_next_statement;
#else
pinmode: // PINMODE <pin>, I/O
awrite: // AWRITE <pin>,val
dwrite:
  goto unimplemented;
#endif

  /*************************************************/
files:
  // display a listing of files on the device.
  // version 1: no support for subdirectories

#ifdef ENABLE_FILEIO
    cmd_Files();
  goto warmstart;
#else
  goto unimplemented;
#endif // ENABLE_FILEIO


chain:
  runAfterLoad = true;

load:
  // clear the program
  program_end = program_start;

  // load from a file into memory
#ifdef ENABLE_FILEIO
  {
    unsigned char *filename;

    // Work out the filename
    expression_error = 0;
    filename = filenameWord();
    if(expression_error)
      goto qwhat;

#ifdef ARDUINO
    // Arduino specific
    if( !SD.exists( (char *)filename ))
    {
      printmsg( sdfilemsg );
    } 
    else {

      fp = SD.open( (const char *)filename );
      inStream = kStreamFile;
      inhibitOutput = true;
    }
#else // ARDUINO
    // Desktop specific
#endif // ARDUINO
    // this will kickstart a series of events to read in from the file.

  }
  goto warmstart;
#else // ENABLE_FILEIO
  goto unimplemented;
#endif // ENABLE_FILEIO



save:
  // save from memory out to a file
#ifdef ENABLE_FILEIO
  {
    unsigned char *filename;

    // Work out the filename
    expression_error = 0;
    filename = filenameWord();
    if(expression_error)
      goto qwhat;

#ifdef ARDUINO
    // remove the old file if it exists
    if( SD.exists( (char *)filename )) {
      SD.remove( (char *)filename );
    }

    // open the file, switch over to file output
    fp = SD.open( (const char *)filename, FILE_WRITE );
    outStream = kStreamFile;

    // copied from "List"
    list_line = findline();
    while(list_line != program_end)
      printline();

    // go back to standard output, close the file
    outStream = kStreamSerial;

    fp.close();
#else // ARDUINO
    // desktop
#endif // ARDUINO
    goto warmstart;
  }
#else // ENABLE_FILEIO
  goto unimplemented;
#endif // ENABLE_FILEIO

rseed:
  {
    short int value;

    //Get the pin number
    expression_error = 0;
    value = expression();
    if(expression_error)
      goto qwhat;

#ifdef ARDUINO
    randomSeed( value );
#else // ARDUINO
    srand( value );
#endif // ARDUINO
    goto run_next_statement;
  }

#ifdef ENABLE_TONES
tonestop:
  noTone( kPiezoPin );
  goto run_next_statement;

tonegen:
  {
    // TONE freq, duration
    // if either are 0, tones turned off
    short int freq;
    short int duration;

    //Get the frequency
    expression_error = 0;
    freq = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();


    //Get the duration
    expression_error = 0;
    duration = expression();
    if(expression_error)
      goto qwhat;

    if( freq == 0 || duration == 0 )
      goto tonestop;

    tone( kPiezoPin, freq, duration );
    if( alsoWait ) {
      delay( duration );
      alsoWait = false;
    }
    goto run_next_statement;
  }
#endif /* ENABLE_TONES */

#ifdef ENABLE_GRAPHIC
cls:
{
  for(int i=0; i<168; i++){
    screenMem[i]=32;/// 32=0x20 of ASCII code, flushing frame buffer
  }
  display.fillScreen(0);
  goto run_next_statement;
}


line:
  {
    //x0, y0, x1, y1
    short int x0;
    short int y0;
    short int x1;
    short int y1;

    //Get x0
    expression_error = 0;
    x0 = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get y0
    expression_error = 0;
    y0 = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get x1
    expression_error = 0;
    x1 = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get y1
    expression_error = 0;
    y1 = expression();
    if(expression_error)
          goto qwhat;
  display.fillScreen(0);          
  display.drawLine(x0, y0, x1, y1, 1);

    goto run_next_statement;
  }
  
  
circle:
  {
    //x, y, radius
    short int xpos;
    short int ypos;
    short int radius;

    //Get the xposition
    expression_error = 0;
    xpos = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get the y position
    expression_error = 0;
    ypos = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get the radius
    expression_error = 0;
    radius = expression();
    if(expression_error)
          goto qwhat;
//display.fillScreen(0);          
  display.drawCircle(xpos,ypos,radius,1);

    goto run_next_statement;
  }

FILLCIRCLE:
  {
    // x, y, radius
    short int xpos;
    short int ypos;
    short int radius;

    //Get the xposition
    expression_error = 0;
    xpos = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get the y position
    expression_error = 0;
    ypos = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get the radius
    expression_error = 0;
    radius = expression();
    if(expression_error)
          goto qwhat;
  display.fillScreen(0);          
  display.fillCircle(xpos,ypos,radius,1);

    goto run_next_statement;
  }

ROUNDRECT:
//example: roundrect 45,10,40,40,40
/// x y, w, h, t
  {
    // x y, w, h, r
    short int xpos;
    short int ypos;
    short int width;
    short int height;
    short int radius;

    //Get the x
    expression_error = 0;
    xpos = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get the y 
    expression_error = 0;
    ypos = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get the w length
    expression_error = 0;
    width = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get the height length
    expression_error = 0;
    height = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get the radius of round corner
    expression_error = 0;
    radius = expression();
    if(expression_error)
          goto qwhat;
  display.fillScreen(0);          
  display.drawRoundRect(xpos,ypos,width,height,radius,1);
  display.display();
    goto run_next_statement;
  }
  
  
FILLROUNDRECT:
/// x y, w, h, t
  {
    // x y, w, h, r
    short int xpos;
    short int ypos;
    short int width;
    short int height;
    short int radius;

    //Get the xposition
    expression_error = 0;
    xpos = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get the y position
    expression_error = 0;
    ypos = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get the w length
    expression_error = 0;
    width = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get the height length
    expression_error = 0;
    height = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get the radius of round corner
    expression_error = 0;
    radius = expression();
    if(expression_error)
          goto qwhat;
  display.fillScreen(0);          
  display.fillRoundRect(xpos,ypos,width,height,radius,1);

    goto run_next_statement;
  }

TRIANGLE:
/// x0, y0, x1, y1, x2, y2
  {
    // x y, w, h, r
    short int x0;
    short int y0;
    short int x1;
    short int y1;
    short int x2;
    short int y2;

    //Get x0
    expression_error = 0;
    x0 = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get y0
    expression_error = 0;
    y0 = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get x1
    expression_error = 0;
    x1 = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get y1
    expression_error = 0;
    y1 = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get x2
    expression_error = 0;
    x2 = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get y2
    expression_error = 0;
    y2 = expression();
    if(expression_error)
          goto qwhat;
  display.fillScreen(0);          
  display.drawTriangle(x0, y0, x1, y1, x2, y2,1); //last "1" is color white

    goto run_next_statement;
  }

FILLTRIANGLE:
/// x0, y0, x1, y1, x2, y2
  {
    short int x0;
    short int y0;
    short int x1;
    short int y1;
    short int x2;
    short int y2;

    //Get x0
    expression_error = 0;
    x0 = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get y0
    expression_error = 0;
    y0 = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get x1
    expression_error = 0;
    x1 = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get y1
    expression_error = 0;
    y1 = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get x2
    expression_error = 0;
    x2 = expression();
    if(expression_error)
      goto qwhat;

    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    //Get y2
    expression_error = 0;
    y2 = expression();
    if(expression_error)
          goto qwhat;
  display.fillScreen(0);        
  display.fillTriangle(x0, y0, x1, y1, x2, y2,1);

    goto run_next_statement;
  }

#endif /* ENABLE_GRAPHIC */


///////////////////////CAMERA PART/////////////////////////
asnap:
  {

  printmsg(picmsg);
  delay(500);
  //cam.reset();
      outStream = kStreamFile;
  if (! cam.takePicture())
  {
    printmsg(picfail);
  }
  else
   { 
    printmsg(pictaken);
   }

  // Open the file for writing
  if(SD.exists("TEST.JPG")){
    SD.remove("TEST.JPG");
  }
  File imgFile = SD.open("TEST.JPG", FILE_WRITE);

  // Get the size of the image (frame) taken  
  uint16_t jpglen = cam.frameLength();
  printmsg(picstore);
  //outchar(char(jpglen));
  printmsg(picstoremsg);
  int32_t time = millis();
 // pinMode(8, OUTPUT);
  // Read all the data up to # bytes!
  byte wCount = 0; // For counting # of writes
  while (jpglen > 0) {
    // read 32 bytes at a time;
    uint8_t *buffer;
    uint8_t bytesToRead = min(64, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
    buffer = cam.readPicture(bytesToRead);
    imgFile.write(buffer, bytesToRead);
    if(++wCount >= 64) { // Every 2K, give a little feedback so it doesn't appear locked up
      outchar('.');
      wCount = 0;
    }
    //Serial.print("Read ");  Serial.print(bytesToRead, DEC); Serial.println(" bytes");
    jpglen -= bytesToRead;
  }
  imgFile.close();

//  time = millis() - time;
  printmsg(picdone);
//  Serial.print(time); Serial.println(" ms elapsed");
///////
    outStream = kStreamSerial;
    cam.reset();
    SD.open("TEST.JPG", FILE_READ);
    
      // zero line spacing
  thermalPrinter.write(27);
  thermalPrinter.write(51);
  thermalPrinter.write((byte)0);
      JpegDec.decode("TEST.JPG",0);
    
        while(JpegDec.read()){

      pImg = JpegDec.pImage ;

        for(by=0; by<JpegDec.MCUHeight; by++){
        
            for(bx=0; bx<JpegDec.MCUWidth; bx++){
            
                x = JpegDec.MCUx * JpegDec.MCUWidth + bx;
                y = JpegDec.MCUy * JpegDec.MCUHeight + by;
                
                if(x<JpegDec.width && y<JpegDec.height){

                    if(JpegDec.comps == 1){ // Grayscale
                    
//                        sprintf(str,"#RGB,%d,%d,%u", x, y, pImg[0]);
                        //Serial.println(str);

                    }else{ // RGB

                        //sprintf(str,"#RGB,%d,%d,%u,%u,%u", x, y, pImg[0], pImg[1], pImg[2]);
                        val=0.299*pImg[0]+0.587*pImg[1]+0.114*pImg[2];
                        //sprintf(str,"#RGB,%d,%d,%d", x, y, val);
                        if(val>valmax){
                          valmax=val;
                        }
                        val=13-val/18;
                        switch(val){
                          case 0:
                          pic[x/2][y/2]=' ';
                          break;
                           case 1:
                          pic[x/2][y/2]='.';
                          break;
                           case 2:
                          pic[x/2][y/2]=',';
                          break;
                           case 3:
                          pic[x/2][y/2]=':';
                          break;    
                                case 4:
                          pic[x/2][y/2]=';';
                          break; 
                                case 5:
                          pic[x/2][y/2]='c';
                          break; 
                                case 6:
                          pic[x/2][y/2]='o';
                          break; 
                                case 7:
                          pic[x/2][y/2]='O';
                          break; 
                                case 8:
                          pic[x/2][y/2]='G';
                          break; 
                                case 9:
                          pic[x/2][y/2]='N';
                          break; 
                                case 10:
                          pic[x/2][y/2]='X';
                          break;     
                          case 11:
                          pic[x/2][y/2]='M';
                          break;
                          case 12:
                          pic[x/2][y/2]='W';
                          break;     
                          case 13:
                          pic[x/2][y/2]='@';
                          break;                 
                        }
                        if(val<0){
                          pic[x/2][y/2]=' ';
                        }
 //                       pic[x/2][y/2]=val;
                       // Serial.println(str);
                    }
                }
                pImg += JpegDec.comps ;
            }
        }
        q++;
    }/////reading loop end
    
        for(i=0;i<80;i=i+2)
    {
      for(j=0;j<60;j=j+2)
      {
        if(j==58){
          thermalPrinter.println(pic[79-i][j]);
        }else{
          thermalPrinter.print(pic[79-i][j]);
        }
      }      
    }
  // default line spacing
  thermalPrinter.write(27);
  thermalPrinter.write(50);   
  // new line
  thermalPrinter.print("\n");
  thermalPrinter.print("\n");  
  thermalPrinter.print("\n"); 
  //
  imgFile.close(); 
  cam.reset();
    
    
    
    
    
    goto run_next_statement;
    
    
    
    
    
  }//end camera part


snap:
  {

  printmsg(picmsg);
  delay(1000);
  //cam.reset();
      outStream = kStreamFile;
  if (! cam.takePicture())
  {
    printmsg(picfail);
  }
  else
   { 
    printmsg(pictaken);
   }

  // Create an image with the name IMAGExx.JPG
  char filename[13];
  strcpy(filename, "IMAGE00.JPG");
  for (int i = 0; i < 100; i++) {
    filename[5] = '0' + i/10;
    filename[6] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename)) {
      break;
    }
  }

  // Open the file for writing
  File imgFile = SD.open(filename, FILE_WRITE);

  // Get the size of the image (frame) taken  
  uint16_t jpglen = cam.frameLength();
  printmsg(picstore);
  //outchar(char(jpglen));
  printmsg(picstoremsg);
  int32_t time = millis();
 // pinMode(8, OUTPUT);
  // Read all the data up to # bytes!
  byte wCount = 0; // For counting # of writes
  while (jpglen > 0) {
    // read 32 bytes at a time;
    uint8_t *buffer;
    uint8_t bytesToRead = min(64, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
    buffer = cam.readPicture(bytesToRead);
    imgFile.write(buffer, bytesToRead);
    if(++wCount >= 64) { // Every 2K, give a little feedback so it doesn't appear locked up
      outchar('.');
      wCount = 0;
    }
    //Serial.print("Read ");  Serial.print(bytesToRead, DEC); Serial.println(" bytes");
    jpglen -= bytesToRead;
  }
  imgFile.close();

//  time = millis() - time;
  printmsg(picdone);
//  Serial.print(time); Serial.println(" ms elapsed");
///////
    outStream = kStreamSerial;
    cam.reset();
    goto run_next_statement;
  }
  
  
  
SERVO:
  {
    short int pos;
    //Get the xposition
    expression_error = 0;
    pos = expression();
    if(expression_error)
      goto qwhat;
    ignore_blanks();
  servo.write(pos);
    goto run_next_statement;
  }
  
NOTICE:
{
  notice();
  goto run_next_statement;
}

BTIME:
{
    readPCF2129();
    int xposi=0,yposi=10;
    display.setTextSize(4);
    display.fillScreen(0);  
    display.setCursor(xposi,yposi);
    display.print(hour);
    display.print(":");
    display.print(minute);
    display.display();
    display.setTextSize(1);    
    goto run_next_statement;    
}
TIME:
{
    readPCF2129();
    printnum(hour);
    outchar(':');
    printnum(minute);
    outchar(':');
    printnum(second);
    line_terminator();
    goto run_next_statement;
}

BDATE:
{
    int xposi=0,yposi=10;
    display.setTextSize(3);
    display.fillScreen(0);  
    display.setCursor(xposi,yposi);
    display.println(year+2000);
    //display.print(",");
    display.print(month);
    display.print(",");
    display.print(dayOfMonth);
    display.display();
    display.setTextSize(1);    
    goto run_next_statement;
}

DATE:
{
    readPCF2129();
    printnum(year);
    outchar(',');
    printnum(month);
    outchar(',');
    printnum(dayOfMonth);
    line_terminator();
    goto run_next_statement;
}

JDATE:
{
  readPCF2129();
  display.fillScreen(0);  
  Japanese_display();
  goto run_next_statement;  
}



}

// returns 1 if the character is valid in a filename
static int isValidFnChar( char c )
{
  if( c >= '0' && c <= '9' ) return 1; // number
  if( c >= 'A' && c <= 'Z' ) return 1; // LETTER
  if( c >= 'a' && c <= 'z' ) return 1; // letter (for completeness)
  if( c == '_' ) return 1;
  if( c == '+' ) return 1;
  if( c == '.' ) return 1;
  if( c == '~' ) return 1;  // Window~1.txt

  return 0;
}

unsigned char * filenameWord(void)
{
  // SDL - I wasn't sure if this functionality existed above, so I figured i'd put it here
  unsigned char * ret = txtpos;
  expression_error = 0;

  // make sure there are no quotes or spaces, search for valid characters
  //while(*txtpos == SPACE || *txtpos == TAB || *txtpos == SQUOTE || *txtpos == DQUOTE ) txtpos++;
  while( !isValidFnChar( *txtpos )) txtpos++;
  ret = txtpos;

  if( *ret == '\0' ) {
    expression_error = 1;
    return ret;
  }

  // now, find the next nonfnchar
  txtpos++;
  while( isValidFnChar( *txtpos )) txtpos++;
  if( txtpos != ret ) *txtpos = '\0';

  // set the error code if we've got no string
  if( *ret == '\0' ) {
    expression_error = 1;
  }

  return ret;
}

/***************************************************************************/
static void line_terminator(void)
{
  outchar(NL);
  outchar(CR);
}


static void l_line_terminator(void)
{
  loutchar(NL);
  loutchar(CR);
}

/***********************************************************/
void setup()
{
#ifdef ARDUINO



////////////////////////////////////
  Serial.begin(4800);	// opens serial port(keyboard), sets data rate to 4800 bps
  thermalPrinter.begin(19200); /// printer output thermalPrinter
    Serial1.begin(38400); // opens serial for Adafruit JPEG camera
  servo.attach(16);

//Modify the print speed and heat
  thermalPrinter.write(27);
  thermalPrinter.write(55);
  thermalPrinter.write(printHeat);
  thermalPrinter.write(printSpeed);
  thermalPrinter.write((byte)0);
//	lcd.begin(16, 4);
  display.begin();
  // init done
  display.display();
  //delay(2000);
  display.clearDisplay();
  // draw a single pixel
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
	for (int xg = 0 ; xg < 169 ; xg++) {
		screenMem[xg] = 32;

	}
/////////////////////////////////////
  printmsg(initmsg);

#ifdef ENABLE_FILEIO
  initSD();
#endif /* ENABLE_FILEIO */
  
#ifdef ENABLE_AUTORUN
  if( SD.exists( kAutorunFilename )) {
    program_end = program_start;
    fp = SD.open( kAutorunFilename );
    inStream = kStreamFile;
    inhibitOutput = true;
    runAfterLoad = true;
  }
#endif /* ENABLE_AUTORUN */

pinMode(14, OUTPUT);


#ifdef ENABLE_EEPROM
#ifdef ENABLE_EAUTORUN
  // read the first byte of the eeprom. if it's a number, assume it's a program we can load
  int val = EEPROM.read(0);
  if( val >= '0' && val <= '9' ) {
    program_end = program_start;
    inStream = kStreamEEProm;
    eepos = 0;
    inhibitOutput = true;
    runAfterLoad = true;
  }
#endif /* ENABLE_EAUTORUN */
#endif /* ENABLE_EEPROM */

#endif /* ARDUINO */

  // Try to locate the camera
  if (cam.begin()) {
  cameracheck=true;
  // Set the picture size - you can choose one of 640x480, 320x240 or 160x120 
  // Remember that bigger pictures take longer to transmit!
  
//  cam.setImageSize(VC0706_640x480);        // biggest
//  cam.setImageSize(VC0706_320x240);        // medium
  cam.setImageSize(VC0706_160x120);          // small
  valmax=0;
  // You can read the size back from the camera (optional, but maybe useful?)
  uint8_t imgsize = cam.getImageSize();    
  } else {
    cameracheck=false;
  }
///////////PCF2129 RTC time setting part//////////
//  // change the following to set your initial time
  Wire.begin();
  second = 30;
  minute = 54;
  hour = 9;
  dayOfWeek = 6;
  dayOfMonth = 17;
  month = 10;
  year = 15;
  //If time adjustment, comment out the next two lines and upload again to set and delete them in 
  //order to avoid resetting every reset
  //if(year!=2015){
  //setPCF2129();
  //}

}


/***********************************************************/
const unsigned char breakcheck(void)
{
#ifdef ARDUINO
  if(Serial.available())
    return Serial.read() == CTRLC;
  return 0;
#else
  if(kbhit())
    return getch() == CTRLC;
   else
     return 0;
#endif
}
/***********************************************************/
static int inchar()
{
  int v;
  int v2;
#ifdef ARDUINO
  
  switch( inStream ) {
  case( kStreamFile ):
#ifdef ENABLE_FILEIO
    v = fp.read();
    if( v == NL ) v=CR; // file translate
    if( !fp.available() ) {
      fp.close();
      goto inchar_loadfinish;
    }
    return v;    
#else
#endif
     break;
  case( kStreamEEProm ):
#ifdef ENABLE_EEPROM
#ifdef ARDUINO
    v = EEPROM.read( eepos++ );
    if( v == '\0' ) {
      goto inchar_loadfinish;
    }
    return v;
#endif
#else
    inStream = kStreamSerial;
    return NL;
#endif
     break;
  case( kStreamSerial ):
  default:
    while(1)
    {
      if(Serial.available())
      {
        v2=Serial.read();
        if(v2!=0x00){
          return v2;
        }
      }
    }
  }
  
inchar_loadfinish:
  inStream = kStreamSerial;
  inhibitOutput = false;

  if( runAfterLoad ) {
    runAfterLoad = false;
    triggerRun = true;
  }
  return NL; // trigger a prompt.
  
#else
  // otherwise. desktop!
  int got = getchar();

  // translation for desktop systems
  if( got == LF ) got = CR;

  return got;
#endif
}

/***********************************************************/
static void outchar(unsigned char c)
{
  if( inhibitOutput ) return;

#ifdef ARDUINO
  #ifdef ENABLE_FILEIO
    if( outStream == kStreamFile ) {
      // output to a file
      fp.write( c );
    } 
    else
  #endif
  #ifdef ARDUINO
  #ifdef ENABLE_EEPROM
    if( outStream == kStreamEEProm ) {
      EEPROM.write( eepos++, c );
    }
    else 
  #endif /* ENABLE_EEPROM */
  #endif /* ARDUINO */
    ///TV.print((char)c);
    //thermalPrinter.write(c);
    lcdChar(c);
#else
  putch(c);
#endif
}


static void loutchar(unsigned char c)
{
  if( inhibitOutput ) return;
  thermalPrinter.write(c);
}

void qoutchar(char* message){
  printQR(message);
}


static void lcdChar(byte c) {

	if (c == 8) {	//Backspace?
	
		if (cursorX > 0) {
	
			cursorX -= 1;	//Go back one
			screenMem[147 + cursorX] = 32;	//Erase it from memory
			doFrame(147 + cursorX); //Redraw screen up to that amount
			
		}
	
	}

	if (c != 13 and c != 10 and c != 8) {	//Not a backspace or return, just a normal character
	
		screenMem[147 + cursorX] = c;
		cursorX += 1;
		if (cursorX < 21) {
	        display.setCursor(cursorX*6,8*7);		
		display.write(c);
		display.display();
		}
		
	}
	
	if (cursorX == 21 or c == 10) {			//Did we hit Enter or go type past the end of a visible line?
	
		for (int xg = 21 ; xg > 0 ; xg--) {

			screenMem[0 + xg] = screenMem[21 + xg];
			screenMem[21 + xg] = screenMem[42 + xg];		
			screenMem[42 + xg] = screenMem[63 + xg];
			screenMem[63 + xg] = screenMem[84 + xg];
			screenMem[84 + xg] = screenMem[105 + xg];
			screenMem[105 + xg] = screenMem[126 + xg];		
			screenMem[126 + xg] = screenMem[147 + xg];



			screenMem[147 + xg] = 32;
		
		
		}
	
		cursorX = 0;
		
		doFrame(147);	
	}


}

static void doFrame(byte amount) {
        int xposi,yposi,yshift;
	//display.cleardisplay();
	//lcd.noCursor();
        display.clearDisplay();
	for (int xg = 0 ; xg < amount ; xg++) {
                yshift=int(xg/21.0);
                yposi=yshift*8;
                xposi=(xg-yshift*21)*6;
	        display.setCursor(xposi,yposi);
		display.write(screenMem[xg]);
	}
        display.display();
	//lcd.cursor();

}

/***********************************************************/
/* SD Card helpers */

#if ARDUINO && ENABLE_FILEIO

static int initSD( void )
{
  // if the card is already initialized, we just go with it.
  // there is no support (yet?) for hot-swap of SD Cards. if you need to 
  // swap, pop the card, reset the arduino.)

  if( sd_is_initialized == true ) return kSD_OK;

  // due to the way the SD Library works, pin 10 always needs to be 
  // an output, even when your shield uses another line for CS
  pinMode(4, OUTPUT); // change this to 53 on a mega
//  pinMode(11, OUTPUT);
  if( !SD.begin( kSD_CS )) {
    // failed
    printmsg( sderrormsg );
    return kSD_Fail;
  }
  // success - quietly return 0
  sd_is_initialized = true;

  // and our file redirection flags
  outStream = kStreamSerial;
  inStream = kStreamSerial;
  inhibitOutput = false;

  return kSD_OK;
}
#endif

#if ENABLE_FILEIO
void cmd_Files( void )
{
  File dir = SD.open( "/" );
  dir.seek(0);

  while( true ) {
    File entry = dir.openNextFile();
    if( !entry ) {
      entry.close();
      break;
    }

    // common header
    printmsgNoNL( indentmsg );
    printmsgNoNL( (const unsigned char *)entry.name() );
    if( entry.isDirectory() ) {
      printmsgNoNL( slashmsg );
    }

    if( entry.isDirectory() ) {
      // directory ending
      for( int i=strlen( entry.name()) ; i<16 ; i++ ) {
        printmsgNoNL( spacemsg );
      }
      printmsgNoNL( dirextmsg );
    }
    else {
      // file ending
      for( int i=strlen( entry.name()) ; i<17 ; i++ ) {
        printmsgNoNL( spacemsg );
      }
      printUnum( entry.size() );
    }
    line_terminator();
    entry.close();
  }
  dir.close();
}
#endif



static unsigned char qr_quoted_string(void)
{
  int i=0;


  unsigned char delim = *txtpos;
  if(delim != '"' && delim != '\'')
    return 0;
  txtpos++;

  // Check we have a closing delimiter
  while(txtpos[i] != delim)
  {
    if(txtpos[i] == NL)
      return 0;
    i++;
  }

  // Print the characters
  while(*txtpos != delim)
  {
    outchar(*txtpos);
    mes[menum]=*txtpos;
    txtpos++;
    menum++;
  }
  txtpos++; // Skip over the last delimiter
  
  //twit(a);
  
  return 1;
}

void notice()
{
  int i=0;
//  int v;
  while(i<4){
  tone(14,sounds[i],200);  
  delay(800);
  i ++;
  }
}

byte bcdToDec(byte value)
{
  return ((value / 16) * 10 + value % 16);
}

byte decToBcd(byte value){
  return (value / 10 * 16 + value % 10);
}

void setPCF2129()
// this part sets the time and date to the PCF2129
{
  Wire.beginTransmission(PCF2129address);
  Wire.write(0x03);
  Wire.write(decToBcd(second));  
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));     
  Wire.write(decToBcd(dayOfMonth));
  Wire.write(decToBcd(dayOfWeek));  
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year));
  Wire.endTransmission();
}

void readPCF2129()
// this gets the time and date from the PCF2129
{
  Wire.beginTransmission(PCF2129address);
  Wire.write(0x03);
  Wire.endTransmission();
  Wire.requestFrom(PCF2129address, 7);
  delay(10);
  second     = bcdToDec(Wire.read() & B01111111); // remove VL error bit
  minute     = bcdToDec(Wire.read() & B01111111); // remove unwanted bits from MSB
  hour       = bcdToDec(Wire.read() & B00111111); 
  dayOfMonth = bcdToDec(Wire.read() & B00111111);
  dayOfWeek  = bcdToDec(Wire.read() & B00000111);  
  month      = bcdToDec(Wire.read() & B00011111);  // remove century bit, 1999 is over
  year       = bcdToDec(Wire.read());
}

unsigned long calcAddr(unsigned short jiscode) {
  unsigned long MSB;   
  unsigned long LSB;
  unsigned long Address;
  MSB = (jiscode >> 8) - 0x20;
  LSB = (jiscode & 0xff) -0x20;
  if(MSB >=1 && MSB <= 15 && LSB >=1 && LSB <= 94)
    Address =( (MSB - 1) * 94 + (LSB - 01))*32;
  else if(MSB >=16 && MSB <= 47 && LSB >=1 && LSB <= 94)
    Address =( (MSB - 16) * 94 + (LSB - 1))*32+43584;
  else if(MSB >=48 && MSB <=84 && LSB >=1 && LSB <= 94)
    Address = ((MSB - 48) * 94 + (LSB - 1))*32+ 138464;
  else if(MSB ==85 && LSB >=0x01 && LSB <= 94)
    Address = ((MSB - 85) * 94 + (LSB - 1))*32+ 246944;
  else if(MSB >=88 && MSB <=89 && LSB >=1 && LSB <= 94)
    Address = ((MSB - 88) * 94 + (LSB - 1))*32+ 249952;
  
  return Address;
}

void getCharData(unsigned short code) {
    byte data;
    unsigned long addr=0;
    byte n;
    addr =calcAddr(code);
    n = 32;    
    digitalWrite(kanji_CS, HIGH);
    delayMicroseconds(4);   
    digitalWrite(kanji_CS, LOW);
    SPI.transfer(0x03);
    SPI.transfer((addr>>16) & 0xff);
    SPI.transfer((addr>>8) & 0xff);
    SPI.transfer(addr & 0xff);
    for(byte i = 0;i< n; i++)  {
      rawdata[i] = SPI.transfer(0x00);
    }
    digitalWrite(kanji_CS, HIGH);
}

void displayChar(unsigned short code, unsigned int offset_x,unsigned int offset_y) {
 offset_x=offset_x*16;
 offset_y=offset_y*16;
 getCharData(code);
      for(int x=0; x<32; x++)
    {
        for(int y=0; y<8; y++)
        {
        if (rawdata[x] & (1<<y))
        display.drawPixel(x%16 + offset_x, y+(8*(x>>4)) + offset_y, WHITE);
        else
        display.drawPixel(x%16 + offset_x, y+(8*(x>>4)) + offset_y, BLACK);
        }
    }
    display.display();
}

void Japanese_display(){
  {
  int second2,second1;
  int minute2,minute1;
  int hour2,hour1;
  int month2,month1;
  int day2,day1;
  int wareki2,wareki1;
  int heisei;
  int tmpsec;
  int tmpday;
  int tmphour;
  second2=second/10;
  second1=second-second2*10;
  
  switch(second1){
  case(0):
  displayChar(0x4E6D,6,3);
  break;
  case(1):
  displayChar(0x306C,6,3);
  break;
        case(2):
  displayChar(0x4673,6,3);
    break;
        case(3):
  displayChar(0x3B30,6,3);
    break;
        case(4):
  displayChar(0x3B4D,6,3);
    break;
        case(5):
  displayChar(0x385E,6,3);
    break;
        case(6):
  displayChar(0x4F3B,6,3);
    break;
        case(7):
  displayChar(0x3C37,6,3);
    break;
        case(8):
  displayChar(0x482C,6,3);
    break;
        case(9):
  displayChar(0x3665,6,3);
    break;
  }

  switch(second2){
  case(0):
  displayChar(0x2121,4,3);
  displayChar(0x2121,5,3);
  break;
  case(1):
  displayChar(0x2121,4,3);
  displayChar(0x3D3D,5,3);
  break;
        case(2):
  displayChar(0x4673,4,3);
  displayChar(0x3D3D,5,3);
    break;
        case(3):
  displayChar(0x3B30,4,3);
  displayChar(0x3D3D,5,3);
    break;
        case(4):
  displayChar(0x3B4D,4,3);
    displayChar(0x3D3D,5,3);
    break;
        case(5):
  displayChar(0x385E,4,3);
    displayChar(0x3D3D,5,3);
    break;
  }


  
  displayChar(0x4943,7,3);


  minute2=minute/10;
  minute1=minute-minute2*10;
  
  switch(minute1){
  case(0):
  displayChar(0x4E6D,6,2);
  break;
  case(1):
  displayChar(0x306C,6,2);
  break;
        case(2):
  displayChar(0x4673,6,2);
    break;
        case(3):
  displayChar(0x3B30,6,2);
    break;
        case(4):
  displayChar(0x3B4D,6,2);
    break;
        case(5):
  displayChar(0x385E,6,2);
    break;
        case(6):
  displayChar(0x4F3B,6,2);
    break;
        case(7):
  displayChar(0x3C37,6,2);
    break;
        case(8):
  displayChar(0x482C,6,2);
    break;
        case(9):
  displayChar(0x3665,6,2);
    break;
  }

  switch(minute2){
  case(0):
  displayChar(0x2121,4,2);
  displayChar(0x2121,5,2);
  break;
  case(1):
  displayChar(0x2121,4,2);
  displayChar(0x3D3D,5,2);
  break;
        case(2):
  displayChar(0x4673,4,2);
  displayChar(0x3D3D,5,2);
    break;
        case(3):
  displayChar(0x3B30,4,2);
  displayChar(0x3D3D,5,2);
    break;
        case(4):
  displayChar(0x3B4D,4,2);
    displayChar(0x3D3D,5,2);
    break;
        case(5):
  displayChar(0x385E,4,2);
    displayChar(0x3D3D,5,2);
    break;
  }
  displayChar(0x4A2C,7,2);
    


  hour2=hour/10;
  hour1=hour-hour2*10;
  
  switch(hour1){
  case(0):
  displayChar(0x4E6D,2,2);
  break;
  case(1):
  displayChar(0x306C,2,2);
  break;
        case(2):
  displayChar(0x4673,2,2);
    break;
        case(3):
  displayChar(0x3B30,2,2);
    break;
        case(4):
  displayChar(0x3B4D,2,2);
    break;
        case(5):
  displayChar(0x385E,2,2);
    break;
        case(6):
  displayChar(0x4F3B,2,2);
    break;
        case(7):
  displayChar(0x3C37,2,2);
    break;
        case(8):
  displayChar(0x482C,2,2);
    break;
        case(9):
  displayChar(0x3665,2,2);
    break;
  }

  switch(hour2){
  case(0):
  displayChar(0x2121,0,2);
  displayChar(0x2121,1,2);
  break;
  case(1):
  displayChar(0x2121,0,2);
  displayChar(0x3D3D,1,2);
  break;
        case(2):
  displayChar(0x4673,0,2);
  displayChar(0x3D3D,1,2);
    break;
        case(3):
  displayChar(0x3B30,0,2);
  displayChar(0x3D3D,1,2);
    break;
        case(4):
  displayChar(0x3B4D,0,2);
    displayChar(0x3D3D,1,2);
    break;
        case(5):
  displayChar(0x385E,0,2);
    displayChar(0x3D3D,1,2);
    break;
  }


  
  displayChar(0x3B7E,3,2);


 // if(tmpday!=dayOfMonth){
  month2=month/10;
  month1=month-month2*10;
  
  switch(month1){
  case(0):
  displayChar(0x4E6D,2,1);
  break;
  case(1):
  displayChar(0x306C,2,1);
  break;
        case(2):
  displayChar(0x4673,2,1);
    break;
        case(3):
  displayChar(0x3B30,2,1);
    break;
        case(4):
  displayChar(0x3B4D,2,1);
    break;
        case(5):
  displayChar(0x385E,2,1);
    break;
        case(6):
  displayChar(0x4F3B,2,1);
    break;
        case(7):
  displayChar(0x3C37,2,1);
    break;
        case(8):
  displayChar(0x482C,2,1);
    break;
        case(9):
  displayChar(0x3665,2,1);
    break;
  }

  switch(month2){
  case(0):
  displayChar(0x2121,0,1);
  displayChar(0x2121,1,1);
  break;
  case(1):
  displayChar(0x2121,0,1);
  displayChar(0x3D3D,1,1);
  break;
        case(2):
  displayChar(0x4673,0,1);
  displayChar(0x3D3D,1,1);
    break;
        case(3):
  displayChar(0x3B30,0,1);
  displayChar(0x3D3D,1,1);
    break;
        case(4):
  displayChar(0x3B4D,0,1);
    displayChar(0x3D3D,1,1);
    break;
        case(5):
  displayChar(0x385E,0,1);
    displayChar(0x3D3D,1,1);
    break;
  }
  displayChar(0x376E,3,1);


  day2=dayOfMonth/10;
  day1=dayOfMonth-day2*10;
  
  switch(day1){
  case(0):
  displayChar(0x4E6D,6,1);
  break;
  case(1):
  displayChar(0x306C,6,1);
  break;
        case(2):
  displayChar(0x4673,6,1);
    break;
        case(3):
  displayChar(0x3B30,6,1);
    break;
        case(4):
  displayChar(0x3B4D,6,1);
    break;
        case(5):
  displayChar(0x385E,6,1);
    break;
        case(6):
  displayChar(0x4F3B,6,1);
    break;
        case(7):
  displayChar(0x3C37,6,1);
    break;
        case(8):
  displayChar(0x482C,6,1);
    break;
        case(9):
  displayChar(0x3665,6,1);
    break;
  }

  switch(day2){
  case(0):
  displayChar(0x2121,4,1);
  displayChar(0x2121,5,1);
  break;
  case(1):
  displayChar(0x2121,4,1);
  displayChar(0x3D3D,5,1);
  break;
        case(2):
  displayChar(0x4673,4,1);
  displayChar(0x3D3D,5,1);
    break;
        case(3):
  displayChar(0x3B30,4,1);
  displayChar(0x3D3D,5,1);
    break;
  }
  displayChar(0x467C,7,1);


  
  
  displayChar(0x4A3F,2,0);
  displayChar(0x402E,3,0); 


  heisei=int(year)+12;
  wareki2=heisei/10;
  wareki1=heisei-wareki2*10;
  
  switch(wareki1){
  case(0):
  displayChar(0x4E6D,6,0);
  break;
  case(1):
  displayChar(0x306C,6,0);
  break;
        case(2):
  displayChar(0x4673,6,0);
    break;
        case(3):
  displayChar(0x3B30,6,0);
    break;
        case(4):
  displayChar(0x3B4D,6,0);
    break;
        case(5):
  displayChar(0x385E,6,0);
    break;
        case(6):
  displayChar(0x4F3B,6,0);
    break;
        case(7):
  displayChar(0x3C37,6,0);
    break;
        case(8):
  displayChar(0x482C,6,0);
    break;
        case(9):
  displayChar(0x3665,6,0);
    break;

  }

  switch(wareki2){
  case(0):
  displayChar(0x2121,4,0);
  displayChar(0x2121,5,0);
  break;
  case(1):
  displayChar(0x2121,4,0);
  displayChar(0x3D3D,5,0);
  break;
        case(2):
  displayChar(0x4673,4,0);
  displayChar(0x3D3D,5,0);
    break;
        case(3):
  displayChar(0x3B30,4,0);
  displayChar(0x3D3D,5,0);
  break;
  }
  displayChar(0x472F,7,0);

  
   switch(dayOfWeek){
  case(0):
  displayChar(0x467C,0,3);
  displayChar(0x4D4B,1,3);
  break;
  case(1):
  displayChar(0x376E,0,3);
  displayChar(0x4D4B,1,3);
  break;
        case(2):
  displayChar(0x3250,0,3);
  displayChar(0x4D4B,1,3);
    break;
        case(3):
  displayChar(0x3F65,0,3);
  displayChar(0x4D4B,1,3);
    break;
        case(4):
  displayChar(0x4C5A,0,3);
  displayChar(0x4D4B,1,3);
    break;
        case(5):
  displayChar(0x3662,0,3);
  displayChar(0x4D4B,1,3);
    break;
        case(6):
  displayChar(0x467C,0,3);
  displayChar(0x455A,1,3);
    break;
  } 
 // }
  }
  
}
