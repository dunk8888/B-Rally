#define PAUSEMENULENGTH 4
#define TRACKMENULENGTH 10
//#define LOADSAVEMENULENGTH 3

const char strResume[]  PROGMEM = "Resume";
const char strRestart[] PROGMEM = "Restart";
//const char strLoadSave[] PROGMEM = "Load/Save";
//const char strStats[] PROGMEM = "Results";
const char strSelect[]  PROGMEM = "Select track";
const char strExit[]    PROGMEM = "Exit";
const char strCalibrate[] PROGMEM = "Calibrate Accel";
const char * const pausemenu[PAUSEMENULENGTH] PROGMEM = {strResume, strRestart, strSelect, strExit,};
const char * const pausemenu2[PAUSEMENULENGTH + 1] PROGMEM = {strResume, strRestart, strSelect, strExit, strCalibrate};


const char strTrack0[] PROGMEM = "Easy1";
const char strTrack1[] PROGMEM = "Easy2";
const char strTrack2[] PROGMEM = "Medium1";
const char strTrack3[] PROGMEM = "Medium2";
const char strTrack4[] PROGMEM = "Hard1";
const char strTrack5[] PROGMEM = "Hard2";
const char strTrack6[] PROGMEM = "Harder1";
const char strTrack7[] PROGMEM = "Harder2";
const char strTrack8[] PROGMEM = "Hardest1";
const char strTrack9[] PROGMEM = "Hardest2";

const char * const trackmenu[TRACKMENULENGTH] PROGMEM = {strTrack0, strTrack1, strTrack2, strTrack3, strTrack4, strTrack5, strTrack6, strTrack7, strTrack8, strTrack9,};
/*
  const char strLoad[]  PROGMEM = "Load";
  const char strSave[] PROGMEM = "Save";
  const char strClear[] PROGMEM = "Clear";
  const char * const loadsavemenu[LOADSAVEMENULENGTH] PROGMEM ={strLoad,strSave,strClear,};
*/
