void drawFLine(int x1, int x2, byte y, byte color) {
  if ((y < 0) || (y > LCDHEIGHT) || (x1 > LCDWIDTH) || (x2 < 0)) return;
  byte mask = _BV(y % 8);
  byte nmsk = ~mask;
  if (x1 < 0)x1 = 0;
  if (x2 > LCDWIDTH)x2 = LCDWIDTH;
  int bufpos = (byte)x1 + (y / 8) * LCDWIDTH;
  int bufend = (byte)x2 + (y / 8) * LCDWIDTH;
  if (color == WHITE) {
    while (bufpos < bufend) {
      dbuffer[bufpos++] &= nmsk;
    }
  }  else if (color == BLACK) {
    while (bufpos < bufend) {
      dbuffer[bufpos++] |= mask;
    }
  }  else if (color == GRAY) {
    if (((gb.frameCount + y) % 2) == 1) {
      while (bufpos < bufend - 1) {
        dbuffer[bufpos++] |= mask;
        dbuffer[bufpos++] &= nmsk;
      }
    } else {
      while (bufpos < bufend - 1) {
        dbuffer[bufpos++] &= nmsk;
        dbuffer[bufpos++] |= mask;
      }
    }
  }
}

//without clipping
void renderScaledSprite16(byte xPos, byte yPos, byte f, boolean renderWhite, const byte* sprite) {
  if ((f < 1) || (f > 15)) return;
  if (xPos > LCDWIDTH || yPos > LCDHEIGHT) return;

  sprite += 2; //skip size
  f--; //match array of pre scales
  for (byte y = 0 ; y < (15 - f ); y++) {
    const byte* srcLine = sprite + (pgm_read_byte(factors[f] + y)) * 2; //read mapping dst(scaled - y) -> srcLine in 16x16 sprite
    unsigned int src = pgm_read_byte(srcLine) * 256 + pgm_read_byte(srcLine + 1); //read 2 bytes (16bits, 1 line) from sprite
    byte sl = (y + yPos) / 8;
    if (sl < 7) {
      byte* bufPos = gb.display.getBuffer() + sl * LCDWIDTH + xPos ;
      byte ymask = 1 << ((y  + yPos) % 8);
      for (byte x = 0 ; x < (15 - f ); x++) {
        unsigned int mask = 1 << (15 - pgm_read_byte(factors[f] + x));
        if ((x + xPos) < LCDWIDTH) {
          if ((mask & src) == mask) {
            bufPos[x] |= ymask;
          } else if (renderWhite) {
            bufPos[x] &= (~ymask);
          }
        }
      }
    }
  }
}

// with clipping
/*
  void renderScaledSprite16(byte xPos, byte yPos, byte clipTop, byte clipBottom, const byte clipLeft, const byte clipRight, byte f, boolean renderWhite, const byte* sprite) {
  if ((f < 1) || (f > 15)) return;
  if (xPos > LCDWIDTH || yPos > LCDHEIGHT) return;

  sprite += 2; //skip size
  f--; //match array of pre scales
  for (byte y = 0 + clipTop; y < (15 - f - clipBottom); y++) {
    const byte* srcLine = sprite + (pgm_read_byte(factors[f] + y)) * 2; //read mapping dst(scaled - y) -> srcLine in 16x16 sprite
    unsigned int src = pgm_read_byte(srcLine) * 256 + pgm_read_byte(srcLine + 1); //read 2 bytes (16bits, 1 line) from sprite
    byte sl = (y - clipTop + yPos) / 8;
    if (sl < 7) {
      byte* bufPos = gb.display.getBuffer() + sl * LCDWIDTH + xPos - clipLeft;
      byte ymask = 1 << ((y - clipTop + yPos) % 8);
      for (byte x = 0 + clipLeft; x < (15 - f - clipRight); x++) {
        unsigned int mask = 1 << (15 - pgm_read_byte(factors[f] + x));
        if ((x + xPos) < LCDWIDTH) {
          if ((mask & src) == mask) {
            bufPos[x] |= ymask;
          } else if (renderWhite) {
            bufPos[x] &= (~ymask);
          }
        }
      }
    }
  }
  }
*/
void renderHills(char xOff) {
  for (byte i = 0; i < LCDWIDTH; i++) {
    if (xOff < 0) xOff += 128;
    dbuffer[i] = pgm_read_byte(hills_new + xOff * 3 + 2);                   //line 1
    dbuffer[i + LCDWIDTH] = pgm_read_byte(hills_new + xOff * 3 + 1);    //line 2
    dbuffer[i + LCDWIDTH + LCDWIDTH] = pgm_read_byte(hills_new + xOff * 3);
    dbuffer[i + LCDWIDTH + LCDWIDTH + LCDWIDTH] = 0xFF;
    dbuffer[i + LCDWIDTH + LCDWIDTH + LCDWIDTH + LCDWIDTH] = 0xFF;
    xOff++;
  }
}

void renderCar(char off, byte dir) {
  register byte * buf = (LCDWIDTH * 4 - 12 + LCDWIDTH / 2) + dbuffer + off; //row 5, car (24px) centered (LCDWIDTH/2)
  for (byte i = 0; i < 24; i++) {
    // 1. Mask:
    buf[i + LCDWIDTH] &= pgm_read_byte(cars_m[dir] + 2 * i);
    buf[i] &= pgm_read_byte(cars_m[dir] + 2 * i + 1);

    // 2. render carsprite
    buf[i + LCDWIDTH] |= (gb.frameCount % 2) ? pgm_read_byte(cars_1[dir] + 2 * i) : pgm_read_byte(cars_0[dir] + 2 * i);
    buf[i] |= (gb.frameCount % 2) ? pgm_read_byte(cars_1[dir] + 2 * i + 1) : pgm_read_byte(cars_0[dir] + 2 * i + 1);
  }
}


void renderFloor(char miny) {
  register byte mask1 = B10101010;
  if (gb.frameCount % 2) {
    mask1 = ~mask1;
  }
  register byte mask2 = ~mask1;
  char yrest = (LCDHEIGHT - miny) % 8;
  byte row = miny / 8;
  if (yrest == 0) {
    row--;
  }
  for (char y = row + 1; y < 6; y++) {
    byte x = 0;
    register byte * buf = gb.display.getBuffer() + y * LCDWIDTH; // ((y >> 1) & 0xfc);
    do {
      buf[x] = mask1;
      buf[++x] = mask2;
    } while (++x < LCDWIDTH);
  }
  if (yrest != 0) {
    byte cut = ~((1 << (7 - yrest)) - 1);
    mask1 &= cut;
    mask2 &= cut;
    cut = ~cut;
    register byte * buf = gb.display.getBuffer() + row * LCDWIDTH;
    byte x = 0;
    do {
      buf[x] &= cut; //clear
      buf[x] |= mask1;
      buf[++x] &= cut;
      buf[x] |= mask2;
    } while (++x < LCDWIDTH);
  }
}

void renderObst() {
  for (byte i = numObst; i > 0; i--) {
    //  if (o[i-1].sprite != tunnel){
    //  renderScaledSprite16(o[i - 1].x, o[i - 1].y, 0, 0, 0, 0, o[i - 1].z, o[i - 1].renderWhite, o[i - 1].sprite);
    renderScaledSprite16(o[i - 1].x, o[i - 1].y, o[i - 1].z, o[i - 1].renderWhite, o[i - 1].sprite);
    //  } else {
    //    renderTunnel(o[i - 1].x, o[i - 1].y, o[i-1].z, o[i-1].cb);
    //   }
  }
}

void renderTunnel(char left, char bottom, char height, char right) {
  for (byte i = 0; i < (max(0, left)); i++) {
    dbuffer[i + LCDWIDTH * (bottom / 8 - 1)] = 0xff;
  }
  for (byte i = (min(right, LCDWIDTH)); i < LCDWIDTH; i++) {
    dbuffer[i + LCDWIDTH * (bottom / 8 - 1)] = 0xff;
  }
}

void renderHUD() {
  gb.display.print(F("Spd:"));
  gb.display.print(sp);
  gb.display.print(F(" Pos:"));
  gb.display.print((byte)pos);
  gb.display.print(F(" L:"));
  gb.display.print((byte)((dist - DISTANCE) / lapLength) + 1);
  gb.display.print(F("/"));
  gb.display.print((byte)(totalLength / lapLength));
}

void renderTunnel(int lastl, int l, int lastr, int r, byte y, byte diff, byte z) {
  lastl = (lastl > 0) ? lastl : 0;
  lastl = (lastl < LCDWIDTH) ? lastl : LCDWIDTH - 1;
  l = (l > 0) ? l : 0;
  l = (l < LCDWIDTH) ? l : LCDWIDTH - 1;
  lastr = (lastr > 0) ? lastr : 0;
  lastr = (lastr < LCDWIDTH) ? lastr : LCDWIDTH - 1;
  r = (r > 0) ? r : 0;
  r = (r < LCDWIDTH) ? r : LCDWIDTH - 1;
  char st = lastl - l;
  if (st > 0) {
    gb.display.setColor(BLACK);
    for (byte i = 0; i < st; i++) {
      gb.display.drawFastVLine(i + lastl, y - i * diff / st - 6, 6);
    }
  } else {
    gb.display.setColor(WHITE);
    for (byte i = 0; i < (-st); i++) {
      gb.display.drawFastVLine(lastl - i, y - i * diff / st - 6, 6);
    }
  }
}

void drawSingleLine(int left, int right, byte y, byte color) {
  if (left > 0 && left < 127) {
    gb.display.setColor(color == 1 ? BLACK : WHITE);
    gb.display.drawPixel(left - 1, y);
    gb.display.drawPixel(left, y);
  }
  gb.display.setColor(WHITE);
  if (left < 127 && right > -127) {
    drawFLine(left + 1, right, y, WHITE);
  }
  if (right < LCDWIDTH) {
    gb.display.setColor(color == 1 ? BLACK : WHITE);
    gb.display.drawPixel(right, y);
    gb.display.drawPixel(right + 1, y);
  }
}


