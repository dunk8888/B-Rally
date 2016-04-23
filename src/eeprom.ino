void loadEEPROM() {
  if (EEPROM.read(0) != EEPROMTOKEN1 || EEPROM.read(1) != EEPROMTOKEN2 || EEPROM.read(2) != EEPROMTOKEN3) {
    clearEEPROM();
    curTrack = 0;
    maxTrack = 0;
    return;
  }
  curTrack = EEPROM.read(4);
  maxTrack = EEPROM.read(5);
}
void saveEEPROM() {
  EEPROM.write(0, EEPROMTOKEN1);
  EEPROM.write(1, EEPROMTOKEN2);
  EEPROM.write(2, EEPROMTOKEN3);
  EEPROM.write(4, curTrack);
  EEPROM.write(5, maxTrack);
}

void clearEEPROM() {
  EEPROM.write(0, EEPROMTOKEN1);
  EEPROM.write(1, EEPROMTOKEN2);
  EEPROM.write(2, EEPROMTOKEN3);
  for (int i = 3; i < 1024; i++) {
    if (EEPROM.read(i)) {
      EEPROM.write(i, 0);
    }
  }
}

void trySaveEEPROM(byte curTrack, byte pos, unsigned int fc) {
  if (EEPROM.read(0) != EEPROMTOKEN1 || EEPROM.read(1) != EEPROMTOKEN2 || EEPROM.read(2) != EEPROMTOKEN3) {
    clearEEPROM();
    return;
  }
  unsigned int sec = (fc / 20);
  if (EEPROM.read(curTrack * 4 + 6) > 0) { //is there a result?
    if (EEPROM.read(curTrack * 4 + 6) > pos)EEPROM.write(curTrack * 4 + 6, pos); //better place
    if  ((unsigned int)(EEPROM.read(curTrack * 4 + 7) * 256 + EEPROM.read(curTrack * 4 + 8)) > (sec)) { //better time
      EEPROM.write(curTrack * 4 + 7, (sec / 256)); //better time - msb
      EEPROM.write(curTrack * 4 + 8, (sec % 256)); //better time - lsb
    }
  } else {
    EEPROM.write(curTrack * 4 + 6, pos);
    EEPROM.write(curTrack * 4 + 7, (sec / 256)); //first time - msb
    EEPROM.write(curTrack * 4 + 8, (sec % 256)); //first time - lsb
  }
  nextTrack = curTrack;
  if (pos < 4) {
    if (curTrack < totalTrack - 1)nextTrack++;
    if (nextTrack > maxTrack)maxTrack = nextTrack;
  }
  if (EEPROM.read(4) != nextTrack) EEPROM.write(4, nextTrack);
  if (EEPROM.read(5) != maxTrack) EEPROM.write(5, maxTrack);
}






