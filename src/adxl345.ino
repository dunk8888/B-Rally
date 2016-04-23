#ifdef WIREON
#define ACC (0x53)    //ADXL345 ACC address
#define A_TO_READ (6)

void testACC() {
  Wire.beginTransmission(ACC);
  if (!Wire.endTransmission()) ACCAvail = true; //returncode==0 -> there is a device at ACC

}
void writeTo(int DEVICE, byte address, byte val) {
  Wire.beginTransmission(DEVICE);
  Wire.write(address);        // send register address
  Wire.write(val);        // send value to write
  Wire.endTransmission();
}


//reads num bytes starting from address register on ACC in to buff array
void readFrom(int DEVICE, byte address, int num, byte buff[]) {
  Wire.beginTransmission(DEVICE);
  Wire.write(address);        //sends address to read from
  Wire.endTransmission();

  Wire.beginTransmission(DEVICE);
  Wire.requestFrom(DEVICE, num);    // request 6 bytes from ACC
  int i = 0;
  while (Wire.available())   //ACC may send less than requested (abnormal)
  {
    buff[i] = Wire.read(); // receive a byte
    i++;
  }
  Wire.endTransmission();
}
void initAcc() {
  //Turning on the ADXL345
  writeTo(ACC, 0x2D, 0);
  writeTo(ACC, 0x2D, 16);

  //DATA-Format (fullRES+16g)
  writeTo(ACC, 0x31, 0x0B);

  writeTo(ACC, 0x2D, 8);

  //BW_RATE
  writeTo(ACC, 0x2C, 0x06);
  //writeTo(ACC, 0x2C, 0x09);

  writeTo(ACC, 0x2E, 0x80);
  delay(100);
  //Offsets
  writeTo(ACC, 0x1E, 0x00);
  writeTo(ACC, 0x1F, 00);
  writeTo(ACC, 0x20, 0);

}
void getAccelerometerData(int * result) {
  int regAddress = 0x32;    //first axis-acceleration-data register on the ADXL345
  byte buff[A_TO_READ];
  readFrom(ACC, regAddress, A_TO_READ, buff); //read the acceleration data from the ADXL345
  //each axis reading comes in 10 bit resolution, ie 2 bytes.  Least Significat Byte first!!
  //thus we are converting both bytes in to one int
  result[0] = (((int)buff[1]) << 8) | (int)buff[0];
  result[1] = (((int)buff[3]) << 8) | (int)buff[2];
  result[2] = (((int)buff[5]) << 8) | (int)buff[4];
}



#endif
