#include "font.c"
// #define STCP  14 //D5
// #define SHTP  12 //D6
// #define DTD   13 //D7
// #define DTX   15 //D8

// #define LSA  16 //D0
// #define LSB  5  //D1
// #define LSC  2  //D3
#define STCP  PA3 //D5
#define SHTP  PA4 //D6
#define DTD   PA5 //D7
#define DTX   PA6 //D8

#define LSA  PA0 //D0
#define LSB  PA1  //D1
#define LSC  PA2  //D3

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
void PushBitDo(int s)
{
  digitalWrite(SHTP,LOW);
  delay(2);
  if (s==1) {
    digitalWrite(DTX,HIGH);
  }
  else {
    digitalWrite(DTX,LOW);
  }
  delay(2);
  digitalWrite(SHTP,HIGH); 
}
void PushBitXanh(int s)
{
  digitalWrite(SHTP,LOW);
  delay(1);
  if (s==1) {
    digitalWrite(DTX,HIGH);
  }
  else {
    digitalWrite(DTX,LOW);
  }
  delay(1);
  digitalWrite(SHTP,HIGH); 
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

void Quet(char* str) {

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
void PushTest() {
  digitalWrite(STCP,LOW);
  for (int i = 0; i < 32; i++){
    if (i == 8)
      PushBit(1);
    else 
      PushBit(0);
  }
  Show();
}
void PushTest1() {

  for (int i = 0; i < 32; i++){
    digitalWrite(STCP,LOW);
    PushBit(0);
    Show();
    delay(1000);
  }
}
void Test() {
  digitalWrite(STCP,LOW);
  for (int i = 0 ;i < 32; i++) {
    if (i %2 == 1)
      PushBit(0);
    else PushBit(1);
  }
  Show();
}
void ClearScreen(int bit) {
  digitalWrite(STCP,LOW);
  for (int i = 0 ;i < 32; i++) {
    PushBit(bit);
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
void TestHang() {
  for (int i = 0; i< 8; i++){
    HangSang(i);
    delay(500);
  }
}
void log(String s) {
  Serial.println(s);
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  GPIO();
  
  HangSang(0);
  delay(1000);
  log("Start");
  ClearScreen(1);
  delay(1000);
  //Test();
  log("End setup ");
}

void loop() {
  // put your main code here, to run repeatedly:
  Quet("123456");
  // PushTest();
  // HangSang(0);
  // delay(1000);
  // ClearScreen(1);
  // Test();
  // delay(1000);
}
