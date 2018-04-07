#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiServer.h>
#include <EEPROM.h>
#include <WiFiUdp.h>

#include "font.c"
#include <Ticker.h>  //Ticker Library


long udpPort = 4210;
WiFiUDP Udp;
String receivedUDP;
char incomingPacket[255];
Ticker ticker;

// long timeTimer = 200000L; // 250us
long timeTimer = 300000L; // 250us
#define DEBUGGING1
//#define ESP8266
//#ifdef ESP8266 
//  #define STCP  14 //D5
//  #define SHTP  12 //D6
//  #define DTD   13 //D7
//  #define DTX   15 //D8
//
//  #define LSA  16 //D0
//  #define LSB  5  //D1
//  #define LSC  4  //D3
//#else 
//  #define STCP  PA3 //D5
//  #define SHTP  PA4 //D6
//  #define DTD   PA5 //D7
//  #define DTX   PA6 //D8
//
//  #define LSA  PA0 //D0
//  #define LSB  PA1  //D1
//  #define LSC  PA2  //D3
//#endif

#define LSA  1 //D0
#define LSB  3  //D1
#define LSC  5  //D3

#define STCP  0 //D5
#define SHTP  2 //D6
#define DTD   15 //D7
#define DTX   15 //D8

char  buff[8][4];
int hSang;

void log(String s) {
  #ifdef DEBUGGING
    Serial.println(s);
  #endif
}

void ConvertStringToBuff(const char* str) {
  int h, c;
  int hang, cot, block;

  int hangBuff, cotBuff;
  int indexByte;

  for (h = 0; h < 8; h++) {
    for (c = 0; c < 4; c++) {
      buff[h][c] = 0x00;
    }
  }


  for (h = 0; h < 8; h++) {
    hang = h;
    for (c = 0; c < 32; c++) {
      block = c / 8;
      cot = (block + 1 ) * 8 - c % 8 - 1;
      cot--; // show Column 1
      int index = cot / 6;

      int _bit = 1;
      if ( GetBit(str[index], hang, cot % 6) == 0) {
        _bit = 0;
      }

      hangBuff = hang;
      indexByte = c / 8;
      byte tg = buff[hangBuff][indexByte] | (_bit << (c%8));
      buff[hangBuff][indexByte] = tg;

    }
  }
}
void GPIO() {
  pinMode(STCP, OUTPUT);
  pinMode(SHTP, OUTPUT);
  pinMode(DTD, OUTPUT);
  pinMode(DTX, OUTPUT);
  pinMode(LSA, OUTPUT);
  pinMode(LSB, OUTPUT);
  pinMode(LSC, OUTPUT);
  digitalWrite(STCP, LOW);
  digitalWrite(SHTP, LOW);
  digitalWrite(DTD, LOW);
  digitalWrite(DTX, LOW);
  digitalWrite(LSA, LOW);
  digitalWrite(LSB, LOW);
  digitalWrite(LSC, LOW);
}
void PushBit(int s)
{
  digitalWrite(SHTP,LOW);
  if (s==1) {
    // digitalWrite(DTD,HIGH);
    digitalWrite(DTX,HIGH);
  }
  else {
    //digitalWrite(DTD,HIGH);
    digitalWrite(DTX,LOW); 
  }
   (SHTP,HIGH); 
}
void PushByte(byte s) {
  for (int i = 0; i < 8; i++) {
    int maBit = s & ( 1 << i);
    if ( maBit == 0) {
      PushBit(0);
    } else {
      PushBit(1);
    }
  }
}

void TestBuff() {
  digitalWrite(STCP,LOW);
  for (int c = 0; c < 4; c++) {
    PushByte(buff[hSang][c]);
  }
  digitalWrite(STCP,LOW);
  digitalWrite(STCP,HIGH);
  HangSang(7- hSang);
}
void Show() {
  digitalWrite(STCP,LOW);
  //delay(1);
  digitalWrite(STCP,HIGH);
 // delay(1);
}
int GetBit(int ValueChar, int hang, int cot) {
  byte maByte = MaFont[ValueChar - 32][cot];
  int maBit = maByte & ( 1 << hang);
  return maBit;
}
int h;
void Quet(const char* str) {

  int h, c;
  for (h = 0; h < 8; h++) {
    digitalWrite(STCP,LOW);
    int hang, cot, block;
    for (c = 0; c < 32; c++) {
      hang = h;
      block = c / 8;
      cot = (block + 1 ) * 8 - c % 8 - 1;
      cot--; // show Column 1
      int index = cot / 6;
      if ( GetBit(str[index], hang, cot % 6) == 0) {
        PushBit(0);
      } else {
        PushBit(1);
      }
      //PushBit(0);
    }
    Show();
    //delay(1);
    HangSang(7- hang);
    // delay(1);
    delayMicroseconds(300);
  }
}
void Quet1(const char* str) {
  int h, c;
  int hang, cot, block;
  digitalWrite(STCP,LOW);
  hang = hSang;
  for (c = 0; c < 32; c++) {
    block = c / 8;
    cot = (block + 1 ) * 8 - c % 8 - 1;
    cot--; // show Column 1
    int index = cot / 6;
    if ( GetBit(str[index], hang, cot % 6) == 0) {
      PushBit(0);
    } else {
      PushBit(1);
    }
    //PushBit(0);
  }
  Show();
  //delay(1);
  HangSang(7- hang);
  // delay(1);
  //delayMicroseconds(600);
}
int cr = 0;
void QuetChay(char* str, long timeScroll) {

  int h, c;
  for (h = 0; h < 8; h++) {
    digitalWrite(STCP,LOW);
    int hang, cot, block;
    for (c = 0; c < 32; c++) {
      hang = h;
      block = c / 8;
      cot = (block + 1 ) * 8 - c % 8 - 1;

      int index = cot / 6;
      if ( GetBit(str[index], hang, cot % 6) == 0) {
        PushBit(0);
      } else {
        PushBit(1);
      }
      //PushBit(0);
    }
    Show();
    //delay(1);
    HangSang(7- hang);
    delay(2);
  }
}
void ClearScreen(int _bit) {
  digitalWrite(STCP,LOW);
  for (int i = 0 ;i < 32; i++) {
    PushBit(_bit);
  }
  Show();
}
void HangSang(int h)
{
  switch (h)
    {
      case 0:  digitalWrite(LSA,LOW);digitalWrite(LSB,LOW);digitalWrite(LSC,LOW); break;
      case 1:  digitalWrite(LSA,HIGH);digitalWrite(LSB,LOW);digitalWrite(LSC,LOW); break;
      case 2:  digitalWrite(LSA,LOW);digitalWrite(LSB,HIGH);digitalWrite(LSC,LOW); break;
      case 3:  digitalWrite(LSA,HIGH);digitalWrite(LSB,HIGH);digitalWrite(LSC,LOW); break;
      case 4:  digitalWrite(LSA,LOW);digitalWrite(LSB,LOW);digitalWrite(LSC,HIGH); break;
      case 5:  digitalWrite(LSA,HIGH);digitalWrite(LSB,LOW);digitalWrite(LSC,HIGH); break;
      case 6:  digitalWrite(LSA,LOW);digitalWrite(LSB,HIGH);digitalWrite(LSC,HIGH); break;
      case 7:  digitalWrite(LSA,HIGH);digitalWrite(LSB,HIGH);digitalWrite(LSC,HIGH); break;
    } 
}
//bool onScreen = true;
void timer0_ISR (void) {
  if (hSang >= 8) {
    hSang = 0;
  }
  // Quet("1234567");
  // Quet1("123456");
//  TestBuff();
  hSang++;
  timer0_write(ESP.getCycleCount() + timeTimer); // 80MHz == 1sec 80000000L); // 80MHz == 1sec
}
String listenUDP(){
 int packetSize = Udp.parsePacket();
 if (packetSize)
 {
  log("Received UDP packet");
  // log("Received" + String(packetSize) + "bytes form "+ Udp.remoteIP().toString() + " ,port " + Udp.remotePort());
  int len = Udp.read(incomingPacket, 255);
  if (len > 0){
   incomingPacket[len] = 0;
  }
  // Serial.printf("UDP packet contents: %s\n", incomingPacket);
  // log("UDP packet:"+ String(incomingPacket));
  return String(incomingPacket);
 }
 return "";
}
//getAddress("#S#apSSID##VIP 01#E#");
//packet = "#S#apSSID#VIP 01#E#";
// return VIP 01;
String getData(String packet){
  String s1 = "";
  int i1 = packet.indexOf("##");
  int i2 = packet.indexOf("#E#");
  if (i2 > i1)
    s1 = packet.substring(i1+2,i2);
  return s1;
}

char* string2char(String command){
  char *szBuffer = new char[command.length()+1];
     strcpy(szBuffer,command.c_str( ));
  return szBuffer; 
}
const char* str1;
#define LED 2
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  GPIO();
  log("begin UDP port");
  Udp.begin(udpPort);
  str1 = "mbell   ";
  digitalWrite(DTD,HIGH);
  ClearScreen(1);
  ConvertStringToBuff(str1);

//  noInterrupts();
//  timer0_isr_init();
//  timer0_attachInterrupt(timer0_ISR);
//  timer0_write(ESP.getCycleCount() + timeTimer); // 80MHz == 1sec 800000L
//  interrupts();

//  for (int i = 0; i < 8 ; i++) {
//    HangSang(i);
//    delay(1000);
//  }
  log("End setup ");

//  pinMode(LED, OUTPUT);
//  digitalWrite(LED, HIGH);

}

void loop() {

  // receivedUDP = listenUDP();
  // if (receivedUDP.length() > 0)
  // {
  //   log(receivedUDP);
  //   //pushBufferRF(receivedUDP);
  //   String data = getData(receivedUDP);
  //   if (data.length() > 0 ) {
  //     //log(data);
  //     data.replace(" ","");
  //     str1 = string2char(data + "   ");
  //     ConvertStringToBuff(str1);
  //     Serial.println(str1);
  //   }
  // }
  // digitalWrite(LED, !digitalRead(LED));
  // delay(1000);
  if (hSang >= 8) {
    hSang = 0;
  }
  TestBuff();
  hSang++;
  delay(3);
  
}
