#include "LedControl.h"
#include <Ticker.h>
#include "TimeLib.h"
#include "TimeAlarms.h"
#include "Timezone.h"
#include <WiFiUdp.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include "Bounce2.h"

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "WiFiManager.h"

/*
 NodeMCU -> TM1639
 5V   -> 5V
 D8   -> CS
 D5   -> DIN
 D7   -> CLK
*/

const byte MODE_BUTTON_PIN = 0;

const byte MAX_DIN_PIN = 2;
const byte MAX_CS_PIN = 16;
const byte MAX_CLOCK_PIN = 4;

const byte S8_RX_PIN = 13;
const byte S8_TX_PIN = 15;

const byte I2C_SDA = 5;
const byte I2C_SCL = 4;

const int SI7021_I2C_ADDRESS = 0x40;

TimeChangeRule EEST = {"EEST", Last, Sun, Mar, 3, 180};  //Daylight time = UTC + 3 hours
TimeChangeRule EET = {"EET", Last, Sun, Oct, 4, 120}; //Standard time = UTC + 2 hours
Timezone CE(EEST, EET);
TimeChangeRule *tcr; 

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte ntpPacketBuffer[NTP_PACKET_SIZE];

const char* NTP_SERVER = "pool.ntp.org";
const int NTP_CLIENT_PORT = 2390;
const int NTP_SERVER_PORT = 123;
const int NTP_SERVER_RETRY_DELAY = 16000;

const double NTP_SERVER_UPDATE_INTERVAL = 86400;
const double SENSOR_UPDATE_INTERVAL = 30;
const float TIME_TICK_UPDATE_INTERVAL = 0.5;
const byte MAX_WIFI_CONNECT_DELAY = 50;

const byte LED_BRIGHTNESS = 7;

Bounce debouncer; 

WiFiManager wifiManager;
WiFiUDP udp;
IPAddress ntpServerIP;

WiFiClient httpClient;

Ticker blinker;

LedControl ledDisp = LedControl(MAX_DIN_PIN, MAX_CLOCK_PIN, MAX_CS_PIN, 2);

SoftwareSerial co2SensorSerial(S8_RX_PIN, S8_TX_PIN);

boolean displayMode;
boolean DISPLAY_MODE_TEMP = true;

boolean tickShown;

void changeClockTick() {
  tickShown = !tickShown;
  showTime();
}

time_t getNTPtime() {
  time_t epoch = 0UL;
  while((epoch = getFromNTP()) == 0) {
    delay(NTP_SERVER_RETRY_DELAY);
  }
  epoch -= 2208988800UL;
  return CE.toLocal(epoch, &tcr);
}

unsigned long getFromNTP() {
  udp.begin(NTP_CLIENT_PORT);
  if(!WiFi.hostByName(NTP_SERVER, ntpServerIP)) {
    Serial.println("DNS lookup failed.");
    return 0UL;
  }
  Serial.print("sending NTP packet to ");
  Serial.print(NTP_SERVER);
  Serial.print(" ");
  Serial.println(ntpServerIP);
  memset(ntpPacketBuffer, 0, NTP_PACKET_SIZE);
  ntpPacketBuffer[0] = 0b11100011;
  ntpPacketBuffer[1] = 0;
  ntpPacketBuffer[2] = 6;
  ntpPacketBuffer[3] = 0xEC;
  ntpPacketBuffer[12] = 49;
  ntpPacketBuffer[13] = 0x4E;
  ntpPacketBuffer[14] = 49;
  ntpPacketBuffer[15] = 52;

  udp.beginPacket(ntpServerIP, NTP_SERVER_PORT);
  udp.write(ntpPacketBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
  
   // wait to see if a reply is available
  delay(2000);
  int cb = udp.parsePacket();
  if (!cb) {
    udp.flush();
    udp.stop();
    Serial.println("no packet yet");
    return 0UL;
  }
  udp.read(ntpPacketBuffer, NTP_PACKET_SIZE);
  udp.flush();
  udp.stop();
    
  unsigned long highWord = word(ntpPacketBuffer[40], ntpPacketBuffer[41]);
  unsigned long lowWord = word(ntpPacketBuffer[42], ntpPacketBuffer[43]);
  return (unsigned long) highWord << 16 | lowWord;
}

double getCo2Data() {
  byte command[] = {0xFE, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25};
  byte response[] = {0,0,0,0,0,0,0};
  
  while (!co2SensorSerial.available()) {
    co2SensorSerial.write(command, 7);
    delay(50);
  }
  
  byte timeout = 0;
  while (co2SensorSerial.available() < 7 ) {
    timeout++;  
    if (timeout > 10) {
        while(co2SensorSerial.available())
          co2SensorSerial.read();
          break;
    }
    delay(50);
  }
  for (int i = 0; i < 7; i++) {
    response[i] = co2SensorSerial.read();
  }
  return response[3] * 256 + response[4];
}

float getTemperatureData() {
  unsigned int data[2];
 
  Wire.beginTransmission(SI7021_I2C_ADDRESS);
  Wire.write(0xF3);
  Wire.endTransmission();
  delay(500);
 
  Wire.requestFrom(SI7021_I2C_ADDRESS, 2);
  if (Wire.available() == 2) {
    data[0] = Wire.read();
    data[1] = Wire.read();
  }
  float temp = ((data[0] * 256.0) + data[1]);
  float intTemp = ((175.72 * temp) / 65536.0) - 46.85;
  return intTemp;
}

float getHumidityData() {
  unsigned int data[2];
 
  Wire.beginTransmission(SI7021_I2C_ADDRESS);
  Wire.write(0xF5);
  Wire.endTransmission();
  delay(500);
 
  Wire.requestFrom(SI7021_I2C_ADDRESS, 2);
  if (Wire.available() == 2) {
    data[0] = Wire.read();
    data[1] = Wire.read();
  }
  float intHumidity  = ((data[0] * 256.0) + data[1]);
  intHumidity = ((125 * intHumidity) / 65536.0) - 6;

  return intHumidity;
}

void showData() {
  if (displayMode == DISPLAY_MODE_TEMP) {
    int temp = getTemperatureData();
    int humidity = getHumidityData();

    ledDisp.setDigit(0, 0, temp / 10 % 10, false);
    ledDisp.setDigit(0, 1, temp % 10, false);
    ledDisp.setChar(0, 2, ' ', true);
    ledDisp.setChar(0, 3, 'c', false);
    
    ledDisp.setDigit(0, 4, humidity / 10 % 10, false);
    ledDisp.setDigit(0, 5, humidity % 10, false);
    ledDisp.setRow(0, 6, 0x5);
    ledDisp.setChar(0, 7, 'h', false);
  }
  else {
    int co2 = getCo2Data();
    byte digit = co2 / 1000 % 10;
    if (digit > 0) {
      ledDisp.setDigit(0, 0, digit, false);
    }
    else {
      ledDisp.setChar(0, 0, ' ', false);
    }
    ledDisp.setDigit(0, 1, co2 / 100 % 10, false);
    ledDisp.setDigit(0, 2, co2 / 10 % 10, false);
    ledDisp.setDigit(0, 3, co2 % 10, false);
  }
}

void showTime() {
  byte hours = hour();
  byte minutes = minute();

  if (hours > 10) {
    ledDisp.setDigit(0, 4, hours / 10, false);
  }
  else {
    ledDisp.setChar(0, 4, ' ', false);
  }
  ledDisp.setDigit(0, 5, hours % 10, tickShown);
  ledDisp.setDigit(0, 6, minutes / 10, false);
  ledDisp.setDigit(0, 7, minutes % 10, false);
}

void restart() {
  Serial.println("Will reset and try again...");
  abort();
}

void modeButtonInit() {
  pinMode(MODE_BUTTON_PIN, INPUT);
  debouncer.attach(MODE_BUTTON_PIN);
  debouncer.interval(100);
}

void ledDisplayInit() {
  ledDisp.shutdown(0, false);
  ledDisp.setIntensity(0, LED_BRIGHTNESS);
  ledDisp.clearDisplay(0);
}

void co2SensorInit() {
  co2SensorSerial.begin(9600); 
}

void checkModeButton() {
  debouncer.update();

  if (debouncer.fell()) {
    displayMode = !displayMode;
    if (displayMode == DISPLAY_MODE_TEMP) {
      blinker.detach();

      tickShown = true;
      changeClockTick();

      showData();
    }
    else {
      showTime();
      showData();
      blinker.attach(TIME_TICK_UPDATE_INTERVAL, changeClockTick);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);

  modeButtonInit();
  ledDisplayInit();
  co2SensorInit();
  
  wifiManager.autoConnect("CO2Meter");
  
  setSyncProvider(getNTPtime);
  setSyncInterval(NTP_SERVER_UPDATE_INTERVAL);

  showTime();
  showData();

  Alarm.timerRepeat(SENSOR_UPDATE_INTERVAL, showData);
  blinker.attach(TIME_TICK_UPDATE_INTERVAL, changeClockTick); 
}

void loop() {
  checkModeButton();
  Alarm.delay(100);
}