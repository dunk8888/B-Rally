void running() {
  fc++;
  if ((dist > totalLength + DISTANCE) && (GAMESTATE == RUNNING)) {
    GAMESTATE = GAMEOVER;
    trySaveEEPROM(curTrack, pos, fc);
  }
  //playerinput
  if (gb.buttons.repeat(BTN_LEFT, 1) && (dir != 1)) {
    dir = 2;
  }
  if (gb.buttons.repeat(BTN_RIGHT, 1) && (dir != 5)) {
    dir = 4;
  }
  if (gb.buttons.repeat(BTN_B, 1)) {
    if (sp > 3) sp = sp - 4;
    else sp = 0;
    if (dir == 2)dir = 1;
    if (dir == 4)dir = 5;
  }
  if (gb.buttons.repeat(BTN_A, 1) && (GAMESTATE != COUNTDOWN)) {
    if (sp < MAXSPEED) sp = sp + ((MAXSPEED - sp + 128) / 128);
  } else {
    if (sp > 0) sp = sp - 1;
  }
  if ((gb.buttons.released(BTN_RIGHT) || gb.buttons.released(BTN_LEFT) || gb.buttons.released(BTN_A)) && !ACCAvail) dir = 3;
#ifdef WIREON
  if (ACCAvail) {
    int acc[3];
    getAccelerometerData(acc);
    if (((ACCInvert ? ACCLeft - acc[1] : acc[1] - ACCLeft) < 0) && (dir != 1)) dir = 2;
    if (((ACCInvert ? ACCRight - acc[1] : acc[1] - ACCRight) > 0) && (dir != 5)) dir = 4;
    if ((gb.buttons.released(BTN_RIGHT) || gb.buttons.released(BTN_LEFT) || gb.buttons.released(BTN_A)) || (((ACCInvert ? ACCLeft - acc[1] : acc[1] - ACCLeft) > 0) && ((ACCInvert ? ACCRight - acc[1] : acc[1] - ACCRight) < 0))) dir = 3;
  }
#endif  /* WIREON */
  switch (dir) {
    case 1: if (playerx < 80) playerx += s + 10; break;
    case 2: if (playerx < 80) playerx += s + 5; break;
    case 4: if (playerx > -80) playerx -= s + 5; break;
    case 5: if (playerx > -80) playerx -= s + 10; break;
    default: break;
  }



  if (gb.buttons.pressed(BTN_C)) {
    GAMESTATE = PAUSE;

  }

  if ((playerx < -(ROADWIDTH * segments[0].lanes) || playerx > (ROADWIDTH * segments[0].lanes) ) ) { //offroad
    if (sp > 20) sp = sp / 2;
    if (sp > 0) offx = (offx == 1) ? -1 : 1;
  }
  miny = 99;
  minheight = 99;
  dx = 0;
  dy = 0;
  s = sp / 24;
  checkCollision();  //decrease sp(eed) if collision with opponent detected
  dist += s;
  numObst = 0;
  //shift segments (forward move)
  for (byte i = 0; i < DISTANCE - s; i++) {
    segments[i] = segments[i + s];
  }
  //fill segments with data
  for (byte i = s; i > 0; i--) {
    segments[DISTANCE - i] = getNextSegment();
  }
  //side-force
  playerx = playerx + (((segments[0].x ) * s));// (((segments[0].x / 2) * s));

  //draw background-hills
  hill_x += ((s > 1) ? ((segments[0].x) / 2) : 0);
  /* *********** 70%CPU ****************
    gb.display.drawBitmap(hill_x - LCDWIDTH + offx, hill_y, hills);
    gb.display.drawBitmap(hill_x + offx, hill_y, hills);
  */
  /* ************ 60% CPU ************** */
  renderHills(hill_x + offx);
  renderHUD();

  //calculate y for each segment
  for (byte z = 1; z < DISTANCE; z++) {
    dy = dy + segments[z].y;
    segments[z].height = HALFLCDHEIGHT + (HALFLCDHEIGHT / (z)) + (5 * dy / (z + 4));
    if (segments[z].height < minheight) minheight = segments[z].height;
  }

  //draw grey floor
  renderFloor(minheight);

  updateOpps();
  lastl = 0;
  lastr = LCDWIDTH;
  for (byte z = 1; z < DISTANCE; z++) {
    byte y = segments[z].height;
    byte w = (byte)((ROADWIDTH * segments[z].lanes) / (z));
    dx = dx + segments[z].x; //-15;
    int t = HALFLCDWIDTH + dx + playerx / z;
    l = t - w;
    r = t + w;
    if (y < miny && y > 5) {
      //todo: optimizing, mostly diffy should between 0 and about 10
      byte diffy = miny - y;
      // if (segments[z].signs != 4) {
      for (byte i = 0; i < diffy; i++) {
        drawSingleLine(l - l * i / diffy + lastl * i / diffy, r - r * i / diffy + lastr * i / diffy, i + y, ((dist + z) / 3) % 3);
      }
      /* } else {
         for (byte i = 0; i < diffy; i++) {
           drawSingleLine(l - l * i / diffy + lastl * i / diffy, r - r * i / diffy + lastr * i / diffy, i + y, ((dist + z) / 3) % 3);
         }
         //tunnel??
         //  renderTunnel(lastl,l,lastr,r,y,diffy,z);
        }*/
      lastl = l;
      lastr = r;
      miny = y;
    }
    if ((numObst < MAXOBST) && (z < 64)) {
      byte zt = pgm_read_byte(zLut + z);
      char zt16 = 16 - zt;
      char b = y - zt16; //bottom line

      switch (segments[z].signs) {
        case 5:  for (byte i = 0; i < 4; i++) {
            if ((t > 0) && ((t + zt) < LCDWIDTH)) {
              addObst(zt, t - zt16 / 2 + (i - 2) * zt16, y  - zt, sign_finish, true);
            }
          }; break;
        case 2: if (((r + zt) < LCDWIDTH) && (r > 0))   addObst(zt, r,        b, sign_left, true); break;
        case 1: if ((l > zt16) && (l < LCDWIDTH))       addObst(zt, l - zt16, b, sign_right, true); break;
        case 3: if ((l > zt16) && (l < LCDWIDTH))       addObst(zt, l - zt16, b, sign_lanes, true);
          if (((r + zt) < LCDWIDTH) && (r > 0))   addObst(zt, r,        b, sign_lanes, true); break;
      }

      //no tunnels
      /*
        if ((segments[z].signs == 4)){
        o[numObst].z = zt;
        o[numObst].x = l;
        o[numObst].y = y;
        o[numObst].cb = r;    //misused
        o[numObst].sprite = tunnel;
         numObst++;
        }*/

      if ((segments[z].opp != 0) && (numObst < MAXOBST)) {
        if ((t > 0) && ((t + zt) < LCDWIDTH)) {
          addObst(zt, t - zt16 / 2 + opponents[(segments[z].opp - 1)].x / z, b, opponents[(segments[z].opp - 1)].sprite, false);
        }
        segments[z].opp = 0;
      }
    }
  }
  renderObst();
  //draw player/play motor sound
  renderCar(offx, dir);
  gb.sound.playNote((sp) % 32, 1, 0);
  // lastx = segments[0].x;
  if (GAMESTATE == COUNTDOWN) {
    countdown--;
    if (countdown == 0) {
      GAMESTATE = RUNNING;
    } else {
      //renderScaledSprite16(LCDWIDTH / 2 - 8 + (countdown % 20) / 2, LCDHEIGHT / 4 - 8 + (countdown % 20) / 2, 0, 0, 0, 0, (countdown % 20), true, Countdown[countdown / 20]);
      renderScaledSprite16(LCDWIDTH / 2 - 8 + (countdown % 20) / 2, LCDHEIGHT / 4 - 8 + (countdown % 20) / 2, (countdown % 20), true, Countdown[countdown / 20]);
      fc--;
    }
  }
}

//void addObst(char z,char x, char y, char cb, const byte * sprite, boolean renderWhite){
void addObst(char z, char x, char y, const byte * sprite, boolean renderWhite) {
  if (numObst > MAXOBST) return;
  o[numObst].z = z;
  o[numObst].x = x;
  o[numObst].y = y;
  //o[numObst].cb = cb;
  o[numObst].sprite = sprite;
  o[numObst].renderWhite = renderWhite;
  numObst++;
}

void loadLevel(byte lvl) {
  if (lvl > maxTrack) {
    gb.popup(F("Track locked"), 60);
    return;
  }
  curTrack = lvl;
  levelStart();
}

void levelStart() {
  dist = 0;
  hill_x = 0;
  playerx = 0;
  sp = 0;
  pf_lseek(curTrack * 131072); //jump to track start within dat file
  //load totalLength
  pf_res = pf_read((void*)&pf_buf, 2, &pf_rcount);
  totalLength = (byte)pf_buf[0] * 256 + (byte)pf_buf[1];
  //load lapLength
  pf_res = pf_read((void*)&pf_buf, 2, &pf_rcount);
  lapLength = (byte)pf_buf[0] * 256 + (byte)pf_buf[1];

  //init the 1. <DISTANCE> Segments
  for (byte i = 0; i < DISTANCE; i++) {
    segments[i] = getNextSegment();
    dist++;
  }

  for (byte i = 0; i < 10; i++) {
    opponents[i].d = 11 - i;
    opponents[i].sprite = oppsprite[i];
    opponents[i].x = (i % 2) * ROADWIDTH - ROADWIDTH / 2;
    opponents[i].s = 2 - (float)i / 10; //from 1 to 2  - slowest speed 24, fastest speed 50
    opponents[i].r = 0;
    segments[20 - i].opp = i + 1;
  }
  pos = 11;
  fc = 0;
  countdown = 60; //60;
  GAMESTATE = COUNTDOWN;
  gb.popup((const __FlashStringHelper*)pgm_read_word(&trackmenu[curTrack]), 60);
}

void gameover() {
  char trackBuffer[12];
  strcpy_P(trackBuffer, (char*)pgm_read_word(&(trackmenu[curTrack])));
  if (pos < 4) {
    gb.display.println(F("Congratulations! "));
  }
  gb.display.println(F("You reached "));
  gb.display.print((byte)pos);
  gb.display.println(F(". Place"));
  gb.display.print(F("at track "));
  gb.display.println(trackBuffer);
  gb.display.print(F("Press A"));
  for (byte i = 0; i < 8; i++) {
    float s1 = (gb.frameCount + i * 8);
    //   renderScaledSprite16((byte)(sin(s1 / 30) * 34 + 34), (byte)(16 - cos(s1 / 20) * 16), 0, 0, 0, 0, (byte)(cos(s1 / 20) * 7 + 8), false, GameOver[7 - i]);
    renderScaledSprite16((byte)(sin(s1 / 30) * 34 + 34), (byte)(16 - cos(s1 / 20) * 16), (byte)(cos(s1 / 20) * 7 + 8), false, GameOver[7 - i]);
  }
  if (gb.buttons.pressed(BTN_A)) {
    curTrack = nextTrack;
    loadLevel(curTrack);
  }
}

void updateOpps() {
  pos = 11;
  unsigned int posOnLap = (dist - DISTANCE) % lapLength;
  for (byte i = 0; i < 10; i++) {
    // opponents[i].d = (((long)fc * (20 - i)))/7; // /6: best: approx 3min/10000seg, last: approx 5min/10000seg
    if ((GAMESTATE == RUNNING) && (opponents[i].d < totalLength + i + 20)) opponents[i].r += (opponents[i].s * 24);
    if (opponents[i].r > 24) {
      opponents[i].d += opponents[i].r / 24;
      opponents[i].r %= 24;
    }
    if (opponents[i].d < (dist - DISTANCE)) pos--;

    if ((opponents[i].d % lapLength < posOnLap + DISTANCE) && (opponents[i].d % lapLength > posOnLap)) {
      //opponent in view, should drive normally
      opponents[i].s = 2 - (float)abs(segments[opponents[i].d % lapLength - posOnLap].x) / 7;
      segments[opponents[i].d % lapLength - posOnLap].opp = i + 1;
    } else {
      opponents[i].s = 2 - (float)i / 10; //from 1 to 2  - slowest speed 24, fastest speed 50
      if ((opponents[i].d + 200) < dist) opponents[i].s += 1; //drive faster if behind
    }
    //AI??
  }
}

void checkCollision() {
  unsigned int posOnLap = (dist - DISTANCE) % lapLength;
  for (byte i = 0; i < 10; i++) {
    if (((opponents[i].d % lapLength) > (posOnLap)) && (opponents[i].d % lapLength) < (posOnLap + s)) {
      if (abs(playerx + opponents[i].x) < 15) {
        sp = opponents[i].s * 20; // =24* (5/6)  //sp/2;
        s = opponents[i].s * 5 / 6; ///2;
      }
    }

  }

}
