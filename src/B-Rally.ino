#define WIREON
#include <diskio.h>
#include <integer.h>
#include <petit_fatfs.h>
#include <pff.h>

//imports the SPI library (needed to communicate with Gamebuino's screen)
#include <SPI.h>
#include <EEPROM.h>

#ifdef WIREON
#include <Wire.h>       //accel?
#endif
#include <Gamebuino.h>

#include "Bitmaps.h"
#include "menu.h"

Gamebuino gb;

//declare all the variables needed for the game :
#define DISTANCE 70
#define MAXSPEED 150
#define ROADWIDTH 30
#define HALFLCDHEIGHT (LCDHEIGHT/2)
#define HALFLCDWIDTH  (LCDWIDTH/2)
#define MAXOBST 20
#define PAUSE 0
#define RUNNING 1
#define GAMEOVER 2
#define COUNTDOWN 3
#define LOADSAVE 4
#define SELECT 5
#define CALIBRATE 6
//#define CALLEFT   7
//#define CALRIGHT  8
#define EEPROMTOKEN1 0xb2
#define EEPROMTOKEN2 0xac
#define EEPROMTOKEN3 0xe2

extern const byte font3x5[];
byte * dbuffer = gb.display.getBuffer();
byte miny, minheight;
struct segment {
  char x;
  char y;
  char height;
  byte lanes;
  byte signs;
  byte opp;
};

struct segment segments[DISTANCE];

struct obst {
  const byte* sprite;
  char x;
  char z;
  char y;
  //char cb; //clipbottom
  //char cl;
  //char cr;
  //char ct;
  boolean renderWhite;
};

struct obst o[MAXOBST];

struct opp {
  const byte * sprite;
  unsigned int d;
  char x;
  float s;    //speed
  byte r;     //cumul speed
};

struct opp opponents[10];

char pf_buf[4];
WORD pf_rcount;
FRESULT pf_res;
byte numObst;
int dx, dy;
int playerx;
unsigned int  dist;
byte s = 2;
byte sp = 2;
int lastl;
int lastr;
int l;
int r;
byte dir = 3;
char hill_x;
char offx;
byte pos;
unsigned int fc;
byte GAMESTATE;
byte countdown;
byte curTrack = 0;
byte maxTrack = 3;
byte totalTrack = TRACKMENULENGTH;
byte nextTrack;
unsigned int totalLength = 10000;
unsigned int lapLength = 10000;
byte selectedTrack;
boolean ACCAvail = false;
int ACCCenter;
int ACCLeft;
int ACCRight;
boolean ACCInvert;

// the setup routine runs once when Gamebuino starts up
void setup() {
  gb.begin();
  gb.titleScreen(F("B-Rally"));
  gb.battery.show = false;
#ifdef WIREON
  Wire.begin();
  testACC();
  if (ACCAvail) {
    initAcc();
  }
#endif /* WIREON */

  //open track file
  openTrackFile(0);

  loadEEPROM(); //load lastTrack etc
  levelStart();

}

// the loop routine runs over and over again forever
void loop() {
  //updates the gamebuino (the display, the sound, the auto backlight... everything)
  //returns true when it's time to render a new frame (20 times/second)
  if (gb.update()) {
    switch (GAMESTATE) {
      case RUNNING: running(); break;
      case GAMEOVER: gameover(); break;
      case PAUSE: pause(); break;
      case COUNTDOWN: running(); break;
      case SELECT: selectTrack(); break;
      case CALIBRATE: calibrate(); break;
    }
  }
}

//Mockup-Track for before petitfatfs
/*
  segment getNextSegment() {
  struct segment s;
  s.x = (5.5 * sin(dist / 150) + 1.0 * sin(dist / 70));
  // s.x=0; //STRAIGHT!!!!
  s.y = (3.5 * sin(dist / 130));
  // s.y=0;
  s.lanes = 1;
  s.height = 0;
  s.signs = 0;
  s.opp = 0;
  if (((5.5 * sin((dist + 20) / 150) + 1.0 * sin((dist + 20) / 70)) < -4) && ((dist % 5) == 0))s.signs = 1;
  if (((5.5 * sin((dist + 20) / 150) + 1.0 * sin((dist + 20) / 70)) > 4) && ((dist % 5) == 0))s.signs = 2;
  return s;
  }
*/
segment getNextSegment() {
  struct segment s;
  pf_res = pf_read((void*)&pf_buf, 4, &pf_rcount);
  if (pf_res) {
    // retry laoding
    openTrackFile(curTrack * 131072 + dist * 4 + 4); //trackoffset + distoffset (4bytes per segment) + track/lap length
    pf_res = pf_read((void*)&pf_buf, 4, &pf_rcount);
  }
  s.x = pf_buf[0];
  s.y = pf_buf[1];
  s.lanes = pf_buf[2] + 1;
  s.signs = pf_buf[3];
  s.height = 0;
  s.opp = 0;
  return s;
}

void openTrackFile(long filePos) {
  PFFS.begin(10, rx, tx); // initialize petit_fatfs
  if (!pf_open("B-RALLY.DAT")) {
    pf_lseek(filePos);
  } else {
    gb.popup(F("could not open DAT file"), 200);
  }

}

byte rx() { // needed by petit_fatfs
  SPDR = 0xFF;
  loop_until_bit_is_set(SPSR, SPIF);
  return SPDR;
}

void tx(byte d) { // needed by petit_fatfs
  SPDR = d;
  loop_until_bit_is_set(SPSR, SPIF);
}
