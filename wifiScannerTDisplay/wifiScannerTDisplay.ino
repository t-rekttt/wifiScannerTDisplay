#include "TFT_eSPI.h"
#include <WiFi.h>
#include <vector>

using namespace std;

TFT_eSPI lcd = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&lcd);
TFT_eSprite titleSprite = TFT_eSprite(&lcd);
TFT_eSprite wifiListSprite = TFT_eSprite(&lcd);
TFT_eSprite timeSprite = TFT_eSprite(&lcd);

TaskHandle_t Task1, Task2;

const char* ssid     = "JiChangWook";
const char* password = "depzainhat";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600;             //time zone * 3600 , my time zone is  +1 GTM
const int   daylightOffset_sec = 3600;   
const int MAX_LINES = 7;
int numSsid = 0;

#define gray 0x6B6D
#define blue 0x0AAD
#define orange 0xC260
#define purple 0x604D
#define green 0x1AE9

int minutes = 0, seconds = 0;
char timeWeekDay[3];
String dayInWeek;
String IP;
int wifiListY = 0;
int skips = 0;

int left = 0;
int right = 14;

int lastLeft = 1, lastRight = 1;

const int ENC_TYPE_WEP  = 5;
const int ENC_TYPE_TKIP = 2;
const int ENC_TYPE_CCMP = 4;
const int ENC_TYPE_NONE = 7;
const int ENC_TYPE_AUTO = 8;
const int ENC_TYPE_UNKNOWN = 255;

String printEncryptionType(int thisType) {
  // read the encryption type and print out the name:
  switch (thisType) {
    case ENC_TYPE_WEP:
      return "WEP";
      break;
    case ENC_TYPE_TKIP:
      return "WPA";
      break;
    case ENC_TYPE_CCMP:
      return "WPA2";
      break;
    case ENC_TYPE_NONE:
      return "None";
      break;
    case ENC_TYPE_AUTO:
      return "Auto";
      break;
  }

  return "Undefined";
}

void loop1( void * pvParameters ){
  while (true) {
    numSsid = WiFi.scanNetworks();

    if (numSsid <= 0)
      continue;

    wifiListSprite.deleteSprite();
    wifiListSprite.createSprite(305, 120);
    wifiListSprite.setTextColor(TFT_WHITE, TFT_TRANSPARENT);
    wifiListSprite.setTextFont(2);
    
    // print the list of networks seen:
    wifiListSprite.print("Number of available networks: ");
    wifiListSprite.println(numSsid);
    
    if (numSsid == -1) {
      wifiListSprite.println("Couldn't get a wifi connection");
    }
    else {
      for (int thisNet = 0; thisNet < numSsid; thisNet++) {
        if (thisNet < skips)
          continue;
          
        if (thisNet - skips >= MAX_LINES)
          break;

        String info = "";
        info += String(thisNet + 1);
        info += String(". ");
        info += String(WiFi.SSID(thisNet));
        info += String(".Signal: ");
        info += String(WiFi.RSSI(thisNet));
        info += String(" dBm");
        info += String(".Encryption: ");
        info += String(printEncryptionType(WiFi.encryptionType(thisNet)));
          
        printSplitString(info, wifiListSprite);
        wifiListSprite.println();
      }
    }

    delay(100);
  }
}

void updateTime(int diff) {
  seconds += 1;

  if (seconds > 59) {
    seconds = 0;
    minutes++;
  }
}

int x=0;
long startF=0;
long endF=0;
double fps=0;

long t=0;
int xt=230;
int yt=8;

void printSplitString(String text, TFT_eSprite &buffer)
{
  int wordStart = 0;
  int wordEnd = 0;
  while ((text.indexOf(' ', wordStart) >= 0) && ( wordStart <= text.length())) {
    wordEnd = text.indexOf(' ', wordStart + 1);
    uint16_t len = buffer.textWidth(text.substring(wordStart, wordEnd));
    if (buffer.getCursorX() + len >= buffer.width()) {
      buffer.println();
      wordStart++;
    }
    buffer.print(text.substring(wordStart, wordEnd));
    wordStart = wordEnd;
  }
}

void drawFrame() {
  int dl = digitalRead(left);
  int dr = digitalRead(right);
  
  if(dl != lastLeft && dl == 0 && skips > 0)
    skips--;
  if(dr != lastRight && dr == 0)
    skips++;

  lastLeft = dl;
  lastRight = dr;
  
  sprite.fillSprite(TFT_BLACK);
  
  timeSprite.createSprite(81, 27);
  timeSprite.setTextColor(purple);
  timeSprite.setTextDatum(MC_DATUM);
  timeSprite.fillRoundRect(0,0,80,26,3,TFT_WHITE);
  timeSprite.drawString((minutes < 10 ? "0" : "") + String(minutes) + ":" + (seconds < 10 ? "0" : "") + String(seconds),40,15,4);
  timeSprite.pushToSprite(&sprite,xt,yt);
  timeSprite.deleteSprite();

  wifiListSprite.pushToSprite(&sprite,xt-219,yt+35, TFT_TRANSPARENT);
  
  sprite.drawRoundRect(xt-224,yt+30,305,130,3,TFT_RED);
  titleSprite.pushToSprite(&sprite,xt-224,yt, TFT_TRANSPARENT);
  sprite.pushSprite(0,0);

  if(t+1000<millis()){
    updateTime(millis() - t);
    t=millis();
  }
}

void loop2( void * pvParameters ) {
  while (true) {
    drawFrame();
  }
}

void setup(void)
{    
  pinMode(left, INPUT_PULLUP);
  pinMode(right, INPUT_PULLUP);
  
  lcd.init();
  lcd.fillScreen(TFT_BLACK);
  lcd.setRotation(3);
  
  sprite.createSprite(320, 170);
  sprite.fillSprite(TFT_BLACK);
  sprite.setSwapBytes(true);
  sprite.setTextColor(TFT_WHITE,0xEAA9);
  sprite.setTextDatum(4);

  titleSprite.createSprite(218,26);
  titleSprite.fillSprite(TFT_TRANSPARENT);
  titleSprite.setTextColor(TFT_WHITE,TFT_TRANSPARENT);
  titleSprite.setTextDatum(MC_DATUM);
  titleSprite.drawRoundRect(0,0,217,26,3,TFT_WHITE);
  titleSprite.drawString("Simple Wifi scanner",109,13,2);
  
  xTaskCreatePinnedToCore(
    loop1,   /* Task function. */
    "loop1",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task1,      /* Task handle to keep track of created task */
    0
  );          /* pin task to core 0 */    

//  xTaskCreatePinnedToCore(
//    loop2,   /* Task function. */
//    "loop2",     /* name of task. */
//    10000,       /* Stack size of task */
//    NULL,        /* parameter of the task */
//    1,           /* priority of the task */
//    &Task2,      /* Task handle to keep track of created task */
//    1
//  );

//  Serial.begin(115200);
//  WiFi.begin(ssid, password);
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(500);
//    Serial.print(".");
//  }
//  Serial.println("");
//  Serial.println("WiFi connected.");
//  IP=WiFi.localIP().toString();
} 

void loop() {
  drawFrame();
}
