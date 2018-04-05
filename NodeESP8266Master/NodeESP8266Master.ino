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

#define DEBUGGING
#define ESP8266
#ifdef ESP8266 
  #define STCP  14 //D5
  #define SHTP  12 //D6
  #define DTD   13 //D7
  #define DTX   15 //D8

  #define LSA  16 //D0
  #define LSB  5  //D1
  #define LSC  2  //D3
#else 
  #define STCP  PA3 //D5
  #define SHTP  PA4 //D6
  #define DTD   PA5 //D7
  #define DTX   PA6 //D8

  #define LSA  PA0 //D0
  #define LSB  PA1  //D1
  #define LSC  PA2  //D3
#endif

void log(String s) {
  #ifdef DEBUGGING
    Serial.println(s);
  #endif
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
    digitalWrite(DTD,HIGH);
    digitalWrite(DTX,HIGH);
  }
  else {
    digitalWrite(DTD,LOW);
    digitalWrite(DTX,LOW); 
  }

  //delay(1);
  //delayMicroseconds(50);
  digitalWrite(SHTP,HIGH); 
  // delay(1);
  // //delayMicroseconds(50);
  // digitalWrite(SHTP,LOW); 

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
    delay(1);
  }
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
void TurnOffScreen() {
  int h, c;
  for (h = 0; h < 8; h++) {
    digitalWrite(STCP,LOW);
    for (c = 0; c < 32; c++) { 
      PushBit(1);
    }
    Show();
    //delay(1);
    HangSang(7- h);
    delay(1);
  }
}
void ClearScreen() {
  digitalWrite(STCP,LOW);
  for (int i = 0 ;i < 32; i++) {
    PushBit(1);
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
  //if (onScreen)
  //noInterrupts();
  Quet("123456");
  // else 
  //   TurnOffScreen();
  timer0_write(ESP.getCycleCount() + 1600000L); // 80MHz == 1sec 80000000L); // 80MHz == 1sec
  //interrupts();
}
String listenUDP(){
 int packetSize = Udp.parsePacket();
 if (packetSize)
 {
  log("Received UDP packet");
  log("Received" + String(packetSize) + "bytes form "+ Udp.remoteIP().toString() + " ,port " + Udp.remotePort());
  int len = Udp.read(incomingPacket, 255);
  if (len > 0){
   incomingPacket[len] = 0;
  }
  Serial.printf("UDP packet contents: %s\n", incomingPacket);
  log("UDP packet:"+ String(incomingPacket));
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
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  GPIO();
  delay(1000);
  log("Start");
  ClearScreen();
  delay(1000);
  //Test();
  log("End setup ");
  //onScreen = true;
  //  noInterrupts();
  // timer0_isr_init();
  // timer0_attachInterrupt(timer0_ISR);
  // timer0_write(ESP.getCycleCount() + 1600000L); // 80MHz == 1sec
  // interrupts();

  log("begin UDP port");
  Udp.begin(udpPort);
  str1 = "MBELL  ";
}

void loop() {
  // put your main code here, to run repeatedly:
  //Quet("123456");
  // PushTest();
  // HangSang(0);
  // delay(1000);
  // ClearScreen(1);
  // Test();
  // delay(1000);
  
  //delay(5000);
  //onScreen = !onScreen;
  // if (onScreen == true) {
  //   onScreen = false;
  //   log("false");
  // }
  // else {
  //   onScreen = true;
  //   log("true");
  // }
  receivedUDP = listenUDP();
  if (receivedUDP.length() > 0)
  {
    log(receivedUDP);
    //pushBufferRF(receivedUDP);
    String data = getData(receivedUDP);
    if (data.length() > 0 ) {
      //log(data);
      data.replace(" ","");
      str1 = string2char(data + "   ");
      Serial.println(str1);
    }
    //str = c_str();
  }
  Quet(str1);
  //delay(1);
}
