
#include "TM1639.h"

TM1639::TM1639(byte dataPin, byte clkPin, byte stbPin) {
  DIN_PIN = dataPin;
  CLK_PIN = clkPin;
  STB_PIN = stbPin;

  pinMode(DIN_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(STB_PIN, OUTPUT);
  
  digitalWrite(STB_PIN, HIGH);
  digitalWrite(CLK_PIN, HIGH);
  
  clearDisplay();
}

void TM1639::setIntensity(byte intensity) {
  if (intensity > MAX_INTENSITY) {
    intensity = MAX_INTENSITY;
  }
  
  sendCommand(AUTOMATIC_ADDRESS_MODE);
  sendCommand(DISPLAY_OFF_BRIGHTNESS | intensity);
}

void TM1639::clearDisplay() {
  byte i;
  for (i = 0; i < 16; i++) {
    ledData[i] = 0;
    
    display(i, ledData[i]);
  }
}

void TM1639::setDigit(byte digit, byte value) {
  byte data[16];
  
  switch (digit) {
    case 0:
      memcpy(data, DIGIT0[value], 16);
      break;
    case 1:
      memcpy(data, DIGIT1[value], 16);
      break;
    case 2:
      memcpy(data, DIGIT2[value], 16);
      break;
    case 3:
      memcpy(data, DIGIT3[value], 16);
      break;
    case 4:
      memcpy(data, DIGIT4[value], 16);
      break;
    case 5:
      memcpy(data, DIGIT5[value], 16);
      break;
    case 6:
      memcpy(data, DIGIT6[value], 16);
      break;
    case 7:
      memcpy(data, DIGIT7[value], 16);
      break;   
  }
  sendData(data);
}

void TM1639::setDigitOff(byte digit) {
  byte data[16];
  memcpy(data, DIGIT_SWITCH_OFF_DATA, 16);
  
  sendData(data);
}

void TM1639::showTimeTick(boolean isShown) {
  byte data[16];
  memcpy(data, isShown ? TIME_SEMICOLON_DATA : DIGIT_SWITCH_OFF_DATA, 16);
  
  sendData(data);
}

void TM1639::showDegreeSign() {
  byte data[16];
  memcpy(data, DEGREE_DATA, 16);
  
  sendData(data);
}

void TM1639::showHumiditySign() {
  byte data[16];
  memcpy(data, HUMIDITY_DATA, 16);
  
  sendData(data);
}

void TM1639::sendCommand(byte data) {
  digitalWrite(STB_PIN, LOW);
  
  writeByte(data);
  
  digitalWrite(STB_PIN, HIGH);
}

void TM1639::display(byte addr, byte data) {
  sendCommand(DIRECT_ADDRESS_MODE);
  
  digitalWrite(STB_PIN, LOW);
  
  writeByte(START_ADDRESS | addr);
  writeByte(data);
  
  digitalWrite(STB_PIN, HIGH);
}

void TM1639::sendData(byte data[16]) {
  byte i;
  for (i = 0; i < 16; i++) {
    ledData[i] = ledData[i] | data[i];
    
    display(i, ledData[i]);
  }
}

void TM1639::writeByte(byte data) {
  byte i;

  for (i = 0; i < 8; i++) {
    digitalWrite(CLK_PIN, LOW);
    digitalWrite(DIN_PIN, ((data & 0x01) != 0) ? HIGH : LOW);
    data = data >> 1;
    digitalWrite(CLK_PIN, HIGH);
  }
}
 


