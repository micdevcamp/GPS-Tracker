#include "U8glib.h"
#include <OneButton.h>
#include <Time.h>
#include <avr/pgmspace.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <AnalogSmooth.h>
#include <SD.h>

#define OLED_RESET 4
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST);
bool displayDimmed = false;
bool displayOn = true;

#define SCREEN_OFF 10000
#define REGULAR_FONT u8g_font_5x7r

#define BATTERY_PIN 0
#define GPSRXPin 11
#define GPSTXPin 10

SoftwareSerial ss(GPSRXPin, GPSTXPin);
static const uint32_t GPSBaud = 9600;
String fileName;

TinyGPSPlus gps;

OneButton b1(A1, true);
OneButton b2(A2, true);

time_t startRec = 0;



byte b1State = 0;
byte b2State = 0;
byte displayMode = 0;

byte displayState = 0;
byte selectNo = 0;

double latitude;
double longitude;

float batteryPercentage;

const int chipSelect = 53;

unsigned long lastUpdateTime = 0;

unsigned long dist = 0;
unsigned int speedValue = 0;

char c[] = "    ";

File dataFile;

AnalogSmooth as100 = AnalogSmooth(9999999);

void setup() {                
  Serial.begin(9600);
  Serial2.begin(9600);

  //Pin setup
  pinMode(BATTERY_PIN, INPUT);

  //SD Setup
  pinMode(53, OUTPUT);
  if (!SD.begin(chipSelect)) {
    Serial.println(F("Card failed, or not present"));
    return;
  }
  Serial.println(F("card initialized."));
  
  //GPS
  ss.begin(GPSBaud);
  
  //Init screen
  u8g.setColorIndex(1);
  u8g.setFont(REGULAR_FONT);
  displayAll();

  Serial.println(F("------------------------"));
}

void loop() {
  b1State = b1.tick();
  b2State = b2.tick();
  b1.setClickTicks(20);
  b2.setClickTicks(20);
  if(displayOn && !displayDimmed){
    handleButtons();
  }

  modeManagement();
  
  screenPowerManagement();

  updateGpsStatus();

  updateBatteryPercentage();

  timeSpecialFormat();

  manageBluetooth();
}

void handleButtons(){
  bool change = false;
  if(displayState == 0){
    if (b2State == b2.DURING_LONG_PRESS_STATE){
      if(isRecording()){
        b2State = 0;
        change = true;
        startRec = 0;
        writeFooter();
      }
    }
    else if (b2State == b2.CLICK_STATE){
      startRec = now();
      fileName = timeSpecialFormat();
      writeHeaeder();
      change = true;
    }
    else if (b1State == b1.CLICK_STATE){
      displayMode++;
      change = true;
    }
    
  }
  else if(displayState == 1){
    if (b1State == b1.CLICK_STATE){
      displayMode--;
      change = true;
    }
  }

  
  else if(displayState == 2){
    
  }

  
  else if(displayState == 3){
    
  }

  
  else if(displayState == 10){
    
  }
  
  if(change){
    displayAll();
  }
}

void updateGpsStatus(){
  while (ss.available() > 0){
    if (gps.encode(ss.read())){
      if (gps.date.isValid() && gps.time.isValid())
      {
        if(millis() - lastUpdateTime > 3600000 || lastUpdateTime == 0){
          setTime(gps.time.hour(), gps.time.minute(), gps.time.second(), gps.date.day(), gps.date.month(), gps.date.year());
          lastUpdateTime = millis();
        }
      }
      if (gps.location.isValid()){
        //increment distance
        if(dist > 0){
          //dist += TinyGPSPlus::distanceBetween(gps.location.lat(), gps.location.lng(), latitude, longitude);
        }
        
        //update lat and lon
        latitude = gps.location.lat();
        longitude = gps.location.lng();
        displayContent();
        writeFix();
      }
    }
    if (gps.speed.isValid()){
      speedValue = gps.speed.kmph();
    }
  }
}

void displayHeader(){
  u8g.drawBox( 0, 0, 128, 8);
  u8g.setColorIndex(0);
  u8g.setPrintPos(3, 7);
  char sz[32];
  sprintf(sz, "%02d/%02d %02d:%02d ", day(), month(), hour(), minute());
  u8g.print(sz);
  if(isRecording()){
    u8g.setPrintPos(70, 7); 
    u8g.print("*REC");
  }
  u8g.setPrintPos(98, 7);
  u8g.print("S");
  u8g.setPrintPos(103, 7);
  u8g.print(gps.satellites.value());
  u8g.setPrintPos(112, 7);
  u8g.print("B");
  u8g.setPrintPos(117, 7);
  u8g.print((String)(int(batteryPercentage*100)));
  u8g.setColorIndex(1);
}

void displayContent(){
  if(displayState == 0){
    u8g.setPrintPos(3, 17); 
    u8g.print("Lat :");
    u8g.setPrintPos(30, 17); 
    u8g.print(latitude, 16);
    
    u8g.setPrintPos(3, 25); 
    u8g.print("Lon :");
    u8g.setPrintPos(30, 25); 
    u8g.print(longitude, 16);

    /*u8g.setPrintPos(3, 33); 
    u8g.print("Dist :");
    u8g.setPrintPos(40, 33); 
    u8g.print(dist);

    u8g.setPrintPos(3, 40); 
    u8g.print("Speed :");
    u8g.setPrintPos(45, 40); 
    u8g.print(speedValue);*/
    
  }
  
//  else if(displayState == 1){
//    u8g.setColorIndex(0);
//    //u8g.print(manualBrightness);
//    u8g.print(F("%"));
//    u8g.setColorIndex(1);
//    u8g.setPrintPos(55, 62); 
//    u8g.setFont(REGULAR_FONT);
//    
//  }
//  
//  if(displayState == 2){
//    setChevrons(3);
//    u8g.setPrintPos(1, 25); 
//    u8g.print(c[0]);
//    u8g.print(F(" 100%"));
//    u8g.setPrintPos(1, 40); 
//    u8g.print(c[1]);
//    u8g.print(F(" Cloud"));
//    u8g.setPrintPos(1, 55); 
//    u8g.print(c[2]);
//    u8g.print(F(" Eco"));
//  }
//  
//  else if(displayState == 3){
//    setChevrons(3);
//    u8g.setPrintPos(1, 25); 
//    u8g.print(c[0]);
//    u8g.print(F(" Set Temp"));
//    u8g.setPrintPos(1, 40); 
//    u8g.print(c[1]);
//    u8g.print(F(" On/Off Time"));
//    u8g.setPrintPos(1, 55); 
//    u8g.print(c[2]);
//    u8g.print(F(" Demo"));
//  }
//  
//  else if(displayState == 10){
//    u8g.print("80");
//    u8g.print("%");
//    
//    u8g.drawBox( 53, 40, 20, 24);
//    u8g.setColorIndex(0);
//    u8g.setPrintPos(55, 62); 
//    u8g.setColorIndex(1);
//    u8g.setFont(REGULAR_FONT);
//  }
}

void displayAll(){
  u8g.firstPage();  
  do {
    displayHeader();
    displayContent();
  } while( u8g.nextPage() );
}

void setChevrons(byte n){
  c[selectNo] = '>';
  for(byte i = 0; i < n; i++){
    if (i != selectNo){
      c[i] = ' ';
    }
  }
}

void screenPowerManagement(){
  if(displayOn && b1.getLastActivityTime() + SCREEN_OFF < millis() && b2.getLastActivityTime() + SCREEN_OFF < millis()){
    u8g.sleepOn();
    displayOn = false;
  }
  else if(!displayOn && (b1State == 1 || b2State == 1)){
    u8g.sleepOff();
    displayOn = true;
    displayAll();
  }
}

void modeManagement(){
  /*if(mode == MODE_OFF){
   
  }
  else if(mode == MODE_MANUAL){
   
  }
  else if (mode == MODE_100){
    
  }
  else if (mode == MODE_CLOUD){
    
  }
  else if (mode == MODE_ECO){
   
  }
  else{
    
  }*/
}

String getTimeStr(){
  String spaceH = "";
  String spaceM = "";
  if(hour() < 10){
    spaceH = " ";
  }
  if(minute() < 10){
    spaceM = "0";
  }
  return spaceH + hour() + ":" + spaceM + minute();
}

void updateBatteryPercentage(){
  batteryPercentage = (as100.analogReadSmooth(BATTERY_PIN)-613)/246;
}

bool isRecording(){
  return startRec != 0;
}

void writeHeaeder(){
  char str[12];
  fileName.toCharArray(str, 8);
  str[7] = '.';
  str[8] = 'G';
  str[9] = 'P';
  str[10] = 'X';
  str[11] = 0;
  dataFile = SD.open(str, FILE_WRITE);

  dataFile.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
  dataFile.println("<gpx version=\"1.1\" creator=\"Me\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd http://www.garmin.com/xmlschemas/GpxExtensions/v3 http://www.garmin.com/xmlschemas/GpxExtensionsv3.xsd http://www.garmin.com/xmlschemas/TrackPointExtension/v1 http://www.garmin.com/xmlschemas/TrackPointExtensionv1.xsd\" xmlns=\"http://www.topografix.com/GPX/1/1\" xmlns:gpxtpx=\"http://www.garmin.com/xmlschemas/TrackPointExtension/v1\" xmlns:gpxx=\"http://www.garmin.com/xmlschemas/GpxExtensions/v3\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">");
  dataFile.println("<trk>");
  dataFile.println("<trkseg>");
}

void writeFix(){
  dataFile.print("<trkpt lon=\"");
  dataFile.print(longitude,16);
  dataFile.print("\" lat=\"");
  dataFile.print(latitude,16);
  dataFile.println("\">");
  dataFile.print("<time>");
  dataFile.print(year());
  dataFile.print("-");
  dataFile.print(month());
  dataFile.print("-");
  dataFile.print(day());
  dataFile.print("T");
  dataFile.print(hour());
  dataFile.print(":");
  dataFile.print(minute());
  dataFile.print(":");
  dataFile.print(second());
  dataFile.print(".000Z");
  dataFile.println("</time>");
  dataFile.println("</trkpt>");
}

void writeFooter(){
  dataFile.println("</trkseg>");
  dataFile.println("</trk>");
  dataFile.println("</gpx>");
  dataFile.flush();
  dataFile.close();
}

String timeSpecialFormat(){
  char sz[32];
  sprintf(sz, "%02d%c%c%c%c%c", year()-2000, getChar(month()), getChar(day()), getChar(hour()), getChar(minute()), getChar(second()));
  return (String)sz;
}

char getChar(int n){
  if (n < 10){
    return (char)n+48;
  }
  else if (n < 36){
    return (char)(n+87);
  }
  else{
    return (char)(n+29);
  }
}

void manageBluetooth(){
  String buff = "";
  while (Serial2.available() > 0) {
    buff += (char)Serial2.read();
    delay(10);
  }
  if(buff.length() > 0){
    //Browse the root folder
    File root = SD.open("/");
    while(true) {
      File entry =  root.openNextFile();
      if (!entry) {
         break;
      }
      if((String)entry.name() > buff){
        //If this file is more recent, then send it via bluetooth
        Serial2.println(entry.name());
        while (entry.available()) {
          Serial2.write(entry.read());
        }
        Serial2.println("");
      }
    }
  }
}
