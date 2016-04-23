



void pause() {
  switch ((!ACCAvail) ? gb.menu(pausemenu, PAUSEMENULENGTH) : gb.menu(pausemenu2, PAUSEMENULENGTH + 1)) {
    case -1: break;
    case 0:  GAMESTATE = RUNNING; break;
    case 1:  levelStart(); break;
    case 2:  selectedTrack = 0; GAMESTATE = SELECT; break;
    case 3: ExitGame(); break;
    case 4: GAMESTATE = CALIBRATE; break;
  }

}

void restart() {
  sp = 0;
  dist = 0;
  pf_lseek(0);
  playerx = 0;
  for (byte i = 0; i < DISTANCE; i++) {
    segments[i] = getNextSegment();
    dist++;
  }
  for (byte i = 0; i < 10; i++) {
    opponents[i].d = i;
    opponents[i].sprite = oppsprite[i];
  }
  pos = 11;
  fc = 0;
  GAMESTATE = COUNTDOWN;
}

void ExitGame() {
  sp = 0;
  dist = 0;
  setup();
}

void selectTrack() {
  if (EEPROM.read(0) != EEPROMTOKEN1 || EEPROM.read(1) != EEPROMTOKEN2 || EEPROM.read(2) != EEPROMTOKEN3) {
    clearEEPROM();
    return;
  }

  byte offsetTrack = 0;
  if (selectedTrack > 5) offsetTrack = selectedTrack - 5;
  gb.display.println(F(" Track       Time Pos"));
  char trackBuffer[12];
  unsigned int sec;
  for (byte i = 0 + offsetTrack; i < 6 + offsetTrack; i++) {
    if (selectedTrack == i) gb.display.print(F("\20")); else gb.display.print(F(" "));
    strcpy_P(trackBuffer, (char*)pgm_read_word(&(trackmenu[i])));
    gb.display.print(trackBuffer);
    gb.display.cursorX = 51;
    sec = (((int)EEPROM.read(i * 4 + 7) * 256 + EEPROM.read(i * 4 + 8)));
    if (sec > 0) {
      if ((byte)(sec / 60) < 10)gb.display.print(F("0"));
      gb.display.print((byte)(sec / 60));
      gb.display.print(F(":"));
      if (sec % 60 < 10)gb.display.print(F("0"));
      gb.display.print((byte)(sec % 60));
      gb.display.print(F(" "));
      gb.display.println((byte)EEPROM.read(i * 4 + 6));
    } else {
      gb.display.println(F("--:-- --"));
    }
  }
  gb.display.println(F("\25select \26clear \27back"));
  if (gb.buttons.pressed(BTN_C)) {
    GAMESTATE = RUNNING;
  }
  if (gb.buttons.pressed(BTN_DOWN)) {
    if (selectedTrack < totalTrack - 1) selectedTrack++;
  }
  if (gb.buttons.pressed(BTN_UP)) {
    if (selectedTrack > 0) selectedTrack--;
  }

  if (gb.buttons.held(BTN_B, 40) ) {
    clearEEPROM();
  }

  if ( gb.buttons.pressed(BTN_A)) {
    loadLevel(selectedTrack);
  }
}

void calibrate() {
#ifdef WIREON
  gb.display.println(F("ADXL345 calibration"));
  gb.display.println(F("turn left+press LEFT"));
  gb.display.println(F("turn right+press RIGHT"));
  gb.display.println(F("press C when done"));
  int acc[3];
  getAccelerometerData(acc);
  if ( gb.buttons.pressed(BTN_C)) {
    GAMESTATE = RUNNING;
  }
  if ( gb.buttons.pressed(BTN_LEFT)) {
    ACCLeft = acc[1];
  }
  if ( gb.buttons.pressed(BTN_RIGHT)) {
    ACCRight = acc[1];
  }
  ACCInvert = (ACCLeft > ACCRight) ? true : false;
  gb.display.print(F("current Dir: "));
  if ((ACCInvert ? ACCRight - acc[1] : acc[1] - ACCRight) > 0)gb.display.print(F("RIGHT"));
  if ((ACCInvert ? ACCLeft - acc[1] : acc[1] - ACCLeft) < 0)  gb.display.print(F("LEFT"));
#endif
}

