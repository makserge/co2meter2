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
#include "TM1639.h"

/*
 NodeMCU -> TM1639
 5V   -> 5V
 GND  -> GND
 D0   -> DIN - white
 D5   -> CLK - green
 D6   -> STB - red
*/

const byte MODE_BUTTON_PIN = 0;

const byte LED_DIN_PIN = 16;
const byte LED_CLK_PIN = 14;
const byte LED_STB_PIN = 12;

const byte S8_RX_PIN = 2;
const byte S8_TX_PIN = 13;

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
const double DATA_UPDATE_INTERVAL = 15;
const double TIME_UPDATE_INTERVAL = 1;
const byte MAX_WIFI_CONNECT_DELAY = 50;

const byte LOW_BRIGTNESS_HOUR = 21;
const byte HIGH_BRIGTNESS_HOUR = 7;

const byte LED_BRIGHTNESS_LOW = 1;
const byte LED_BRIGHTNESS_HIGH = 7;

boolean displayMode;
boolean DISPLAY_MODE_TEMP = true;

boolean isTimeTick;
boolean isUpdateDisplay;

int temp;
int humidity;
int co2;

AlarmID_t timeTimer;

Bounce debouncer; 

WiFiManager wifiManager;
WiFiUDP udp;
IPAddress ntpServerIP;

WiFiClient httpClient;

SoftwareSerial co2SensorSerial(S8_RX_PIN, S8_TX_PIN);

TM1639 ledDisp(LED_DIN_PIN, LED_CLK_PIN, LED_STB_PIN);

time_t getNTPtime() {
  time_t epoch = 0UL;
  while ((epoch = getDataFromNTP()) == 0) {
    delay(NTP_SERVER_RETRY_DELAY);
  }
  epoch -= 2208988800UL;
  return CE.toLocal(epoch, &tcr);
}

unsigned long getDataFromNTP() {
  udp.begin(NTP_CLIENT_PORT);
  if (!WiFi.hostByName(NTP_SERVER, ntpServerIP)) {
    Serial.println("DNS lookup failed.");
    return 0UL;
  }
  Serial.print("sending NTP packet to ");
  Serial.print(NTP_SERVER);
  Serial.print(" ");
  Serial.println(ntpServerIP);
  memset(ntpPacketBuffer, 0, NTP_PACKET_SIZE);
  ntpPacketBuffer[0] = 0xE3;
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
  byte command[] = { 0xFE, 0X44, 0X0, 0X08, 0X02, 0X9F, 0X25 };
  byte response[] = { 0, 0, 0, 0, 0, 0, 0 };
  
  while (!co2SensorSerial.available()) {
    co2SensorSerial.write(command, 7);
    delay(50);
  }
  
  byte timeout = 0;
  while (co2SensorSerial.available() < 7) {
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

void getData() {
  if (displayMode == DISPLAY_MODE_TEMP) {
    temp = getTemperatureData();
    humidity = getHumidityData();
  }
  else {
    co2 = getCo2Data();
  }
  isUpdateDisplay = true;
}

void getTimeTick() {
  isTimeTick = !isTimeTick;
  isUpdateDisplay = true;
}

void updateDisplay() {
  if (!isUpdateDisplay) {
    return;
  }
  isUpdateDisplay = false;
  
  ledDisp.clearDisplay();

  byte hours = hour();
      
  if (displayMode == DISPLAY_MODE_TEMP) {
    byte digit = temp / 10 % 10;
    if (digit > 0) {  
      ledDisp.setDigit(2, digit);
    }
    else {
      ledDisp.switchOffDigit(2);
    }
      
    ledDisp.setDigit(1, temp % 10);
    ledDisp.showDegreeSign();
  
    ledDisp.setDigit(6, humidity / 10 % 10);
    ledDisp.setDigit(5, humidity % 10);
    ledDisp.showHumiditySign();
  }
  else {
    byte digit = co2 / 1000 % 10;
    if (digit > 0) {
      ledDisp.setDigit(3, digit);
    }
    else {
      ledDisp.switchOffDigit(3);
    }

    ledDisp.setDigit(2, co2 / 100 % 10);
    ledDisp.setDigit(1,  co2 / 10 % 10);
    ledDisp.setDigit(0, co2 % 10);
    
    byte minutes = minute();

    if (hours > 9) {
      ledDisp.setDigit(7, hours / 10);
    }
    else {
      ledDisp.switchOffDigit(7);
    }
    ledDisp.setDigit(6, hours % 10);
    
    ledDisp.setDigit(5, minutes / 10);
    ledDisp.setDigit(4, minutes % 10);

    ledDisp.showTimeTick(isTimeTick);
  }

  ledDisp.setIntensity((hours > HIGH_BRIGTNESS_HOUR && hours < LOW_BRIGTNESS_HOUR) ? LED_BRIGHTNESS_HIGH : LED_BRIGHTNESS_LOW);
  ledDisp.updateDisplay();
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

void co2SensorInit() {
  co2SensorSerial.begin(9600); 
}

void checkModeButton() {
  debouncer.update();

  if (debouncer.fell()) {
    displayMode = !displayMode;

    getData();

    if (displayMode == DISPLAY_MODE_TEMP) {
      Alarm.disable(timeTimer);
    }
    else {
      Alarm.enable(timeTimer);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);

  modeButtonInit();
  co2SensorInit();
  
  wifiManager.autoConnect("CO2Meter");
  
  setSyncProvider(getNTPtime);
  setSyncInterval(NTP_SERVER_UPDATE_INTERVAL);

  getData();
 
  Alarm.timerRepeat(DATA_UPDATE_INTERVAL, getData);
  timeTimer = Alarm.timerRepeat(TIME_UPDATE_INTERVAL, getTimeTick);
}

void loop() {
  checkModeButton();
  updateDisplay();
  
  Alarm.delay(100);
}
