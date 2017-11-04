#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiServer.h>
#include <EEPROM.h>

ESP8266WebServer server(80);

#define RESET 12 
#define VT 5
#define D0 16
#define D1 14
#define D2 12
#define D3 13


#define DEBUGGING
#define ADDR 0
#define ADDR_TIMELIMIT ADDR
#define ADDR_SSID (ADDR+10)
#define ADDR_PASS (ADDR+30)
#define ADDR_PASSDOOR (ADDR+50)

#define SSID_DEFAULT "ESP8266 AP"
#define PASS_DEFAULT "12345678"
#define TIMELIMIT_DEFAULT 5000
#define PASSDOOR_DEFAULT "12345678"

const char* stassid = "dlink";
const char* stapassword = "";
const char* apssid = "AP_ESP8266"; // tên wifi phát ra
const char* appassword = "12345678"; // mật khẩu wifi phát ra


void DEBUG(String s);
void GPIO();
int ScanRF();

int Mode=0;
int d=0;

void setup()
{
  delay(1000);
  EEPROM.begin(512);
  Serial.begin(9600);
  delay(1000);
  
  GPIO();
//  ConfigDefault();
//  WriteConfig();
//  ReadConfig();
//  ConnectWifi();
//  AccessPoint();
  delay(1000);
 
  //StartServer();
  
}

void loop()
{
  int Di = -1;
  //server.handleClient();
  if (digitalRead(VT) == HIGH)
  {
    show("VT HIGH");
    Di = ScanRF();
  }

  if (Di != -1)
  {
    switch (Di)
    {
      case D0: show("D0"); 
        break;
      case D1: show("D1");
        break;
      case D2: show("D2");
        break;
      case D3: show("D3");
        break;
    }
    Di = -1;
  }
  
  delay(50);
}

void show(String s)
{
  #ifdef DEBUGGING 
    Serial.println(s);
  #endif
}

void GPIO()
{
  show("GPIO");
// pinMode(PINCHUONG,OUTPUT);
// digitalWrite(PINCHUONG,HIGH);
  pinMode(VT,INPUT_PULLUP); 
  pinMode(D0,INPUT_PULLUP); 
  pinMode(D1,INPUT_PULLUP); 
  pinMode(D2,INPUT_PULLUP); 
  pinMode(D3,INPUT_PULLUP); 

 
}

void CheDo()
{
   
}
int ScanRF()
{
  if (digitalRead(D0)==HIGH)
  {
    while (digitalRead(D0)==HIGH);
    return D0;
  }
  else if (digitalRead(D1)==HIGH)
  {
    while (digitalRead(D1)==HIGH);
    return D1;
  }else if (digitalRead(D2)==HIGH)
  {
    while (digitalRead(D2)==HIGH);
    return D2;
  }
  else if (digitalRead(D3)==HIGH)
  {
    while (digitalRead(D3)==HIGH);
    return D3;
  }
  return -1;
}

/*
 * Function Save String To EEPROM
 * Parameter : +data    : String Data
 *             +address : address in EEPROM
 * Return: None.
 */
void SaveStringToEEPROM(String data,int address)
{
  int len=data.length();
  EEPROM.write(address,len); 
  for (int i=1;i<=len;i++)
    EEPROM.write(address+i,data.charAt(i-1));
  //EEPROM.commit();
}
/*
 * Function Read String From EEPROM
 * Parameter : +address : address in EEPROM
 * Return: String.
 */
String ReadStringFromEEPROM(int address)
{
  String s="";
  int len=(int)EEPROM.read(address);
 
  for (int i=1;i<=len;i++)
    s+=(char)EEPROM.read(address+i);
  return s;
}

void ConfigDefault()
{
//  for (int i=0;i<SoTiet-1;i++)
//    R[i]=5;
//  TietHoc=45;
//  GioBd=7;
//  PhutBd=45;
//  Println("Config Default");
}
void WriteConfig()
{
//  int Address=0;
//  EEPROM.write(Address,GioBd);
//  EEPROM.write(Address+1,PhutBd);
//  EEPROM.write(Address+2,TietHoc);
//  Address=Address+3;
//  for (int i=0;i<SoTiet-1;i++)
//      EEPROM.write(Address+i,R[i]);
//  EEPROM.commit();
//  Println("Write Config");
}
void ReadConfig()
{
//  Println("Read Config");
//  int Address=0;
//  GioBd=(int)EEPROM.read(Address);
//  PhutBd=(int)EEPROM.read(Address+1);
//  TietHoc=(int)EEPROM.read(Address+2);
//  Address=Address+3;
//  for (int i=0;i<SoTiet-1;i++)
//      R[i]=(int)EEPROM.read(Address+i);
//  Print("GioBd= "+String (GioBd));
//  Print("PhutBd= "+String (PhutBd));
//  Print("TietHoc= "+String (TietHoc));
//  Print("\n");
//  for (int i=0;i<SoTiet-1;i++)
//      Println("R["+String(i)+"]="+String(R[i]));
}
void AccessPoint()
{
  WiFi.disconnect();
  WiFi.softAP(apssid,appassword);
  Serial.println("");
  // Wait for connection

  Serial.println("done");
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
}

void ConnectWifi()
{
  WiFi.disconnect();
  delay(1000);
  WiFi.begin(stassid,stapassword);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
  }
  Serial.println("IP local :");
  Serial.println(WiFi.localIP()); 
}

void StartServer()
{
  server.on("/Config", handleConfig);
  server.on("/Home", handleHome);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void handleConfig() {
  GiaTriThamSo();
  String httpcode= "";
//  String httpcode="<html>\
//<head>\
//<meta charset=\"utf-8\">\
//<title>Config</title>\
//<style>\
//  * {margin:0;padding:0}\
//  body {width: 600px;height: 650px;border: red 3px solid; margin: 0 auto; }\
//  .head1{ height: 50px;border-bottom: red 3px solid;  }\
//  table, th, td { border: 1px solid black;border-collapse: collapse; }\
//  tr{ height: 40px;text-align: center;font-size: 20px;}\
//  input { height: 25px;text-align: center;}\
//  button {height: 25px;width: 100px;background: red;}\
//</style>\
//</head>\
//<body >\
//  <center>\
//  <div class=\"head1\"> \
//    <h1>Cài Đặt Chuông Báo</h1>\
//  </div>\
//  <div class=\"content\">\
//    <br>\
//    <form action=\"\" method=\"get\">\
//     Thời gian bắt đầu    :<input type=\"number\" name=\"GioBd\" min=\"5\" max=\"23\" value=\""+String(GioBd)+"\"/> Giờ\ 
//     <input type=\"number\" name=\"PhutBd\" min=\"0\" max=\"60\" value=\""+String(PhutBd)+"\"/> Phút--\
//      Tiết học:<input type=\"number\" name=\"TietHoc\" min=\"0\" max=\"60\" value=\""+String(TietHoc)+"\"/> Phút<br> <br>\
//      <button type=\"submit\" name=\"Lưu lại\">Lưu lại</button>\
//    </form>\
//    <form action=\"\" method=\"get\">\
//    <br>\
//      Thời gian hệ thống:\
//      <input type=\"date\" name=\"SetNgay\" min=\"2010-01-01\" max=\"2020-12-31\" />\
//      <input type=\"time\" name=\"SetGio\" />\
//      <button type=\"submit\" name=\"\">Cài đặt</button>\
//    </form>\
//  </div>\
//  </center>\
//</body>\
//</html>";
  server.send ( 200, "text/html",httpcode);
}
String SendTRWeb()
{
  String s="";
//  for (int i=0;i<SoTiet;i++)
//    s+="<tr ><td> "+String (i+1)+" </td><td> Tiết "+String (i+1)+"</td><td> "+ConvertLongToSringTime(TimeHoc[i][0])  +"- "+ ConvertLongToSringTime(TimeHoc[i][1] )+"</td></tr>";
  return s;
}
void handleHome() {
  String httpcode="";
//  String httpcode="<html>\
//<head>\
//<meta http-equiv=\"refresh\" content=\"1\">\
//<meta charset=\"utf-8\">\
//<title>Home</title>\
//<style>\
// * {margin:0;padding:0}\
//  body {width: 600px; height: 650px;border: red 3px solid;margin: 0 auto; }\
//  .head1{ height: 50px;border-bottom: red 3px solid;}\
//  table, th, td { border: 1px solid black;border-collapse: collapse; }\
//  tr {height: 40px;text-align: center;font-size: 20px;}\
//</style>\
//</head>\
//<body >\
//  <center>\
//  <div class=\"head1\">\
//    <h1>Chuông Báo Giờ Học</h1>\
//  </div>\
//  <div class=\"content\">\
//    <h1>"+FormatDate()+"&nbsp &nbsp"+FormatTime()+"</h1>\
//    <table >\
//      <tr>\
//        <th width=\"50px\" >STT</th>\
//        <th width=\"200px\">Tiết học</th>\
//        <th width=\"250px\"> Thời gian</th>\
//      </tr>\
//      "+SendTRWeb()+"\
//    </table>\
//  </div>\
//  </center>\
//</body>\
//</html>";
  server.send ( 200, "text/html",httpcode);
}
void GiaTriThamSo()
{
  String message="";
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  String s1="";
  for (uint8_t i=0; i<server.args(); i++){
     
    String Name=server.argName(i); 
    String Value=String( server.arg(i)) ;
    s1=Name+ ": " +Value;
    Serial.println(s1);
  }
  if (server.args()>5)
  {
    WriteConfig();
    ReadConfig();
  }
}
void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}


