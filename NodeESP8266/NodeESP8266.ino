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
#define ADDR_STASSID (ADDR)
#define ADDR_STAPASS (ADDR+20)
#define ADDR_APSSID (ADDR+40)
#define ADDR_APPASS (ADDR+60)
#define ADDR_PORTTCP (ADDR+80)

#define STA_SSID_DEFAULT "G"
#define STA_PASS_DEFAULT "132654789"
#define AP_SSID_DEFAULT "ESP8266"
#define AP_PASS_DEFAULT "12345678"

#define PORT_TCP_DEFAULT 123

bool isLogin = false;
String staSSID, staPASS;
String apSSID, apPASS;
String SoftIP, LocalIP;
long portTCP;

bool flagClear = false;

void DEBUG(String s);
void GPIO();
int ScanRF();
void SaveStringToEEPROM(String data,int address);
String ReadStringFromEEPROM(int address);
void ClearEEPROM();
void AccessPoint();
void ConnectWifi(long timeOut);
void GiaTriThamSo();

void setup()
{
  delay(1000);
  EEPROM.begin(512);
  Serial.begin(9600);
  delay(1000);
  GPIO();
  if (EEPROM.read(255) != 255 || flagClear){
    ClearEEPROM();
    ConfigDefault();
    WriteConfig();
  }
  ReadConfig();
  AccessPoint();
  ConnectWifi(4000);



  delay(1000);
  StartServer();
  
}

void loop()
{
  server.handleClient();
  int Di = -1;
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

void ClearEEPROM()
{
  // write a 255 to all 512 bytes of the EEPROM
  for (int i = 0; i < 512; i++)
    EEPROM.write(i, 255);
  EEPROM.commit();
  show("Clear EEPROM");
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
  EEPROM.commit();
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
  isLogin = false;
  staSSID = STA_SSID_DEFAULT;
  staPASS = STA_PASS_DEFAULT;
  apSSID = AP_SSID_DEFAULT;
  apPASS = AP_PASS_DEFAULT;
  portTCP = PORT_TCP_DEFAULT;
  show("Config Default");
}
void WriteConfig()
{
  SaveStringToEEPROM(staSSID, ADDR_STASSID);
  SaveStringToEEPROM(staPASS, ADDR_STAPASS);
  SaveStringToEEPROM(apSSID, ADDR_APSSID);
  SaveStringToEEPROM(apPASS, ADDR_APPASS);
  SaveStringToEEPROM(String(portTCP), ADDR_PORTTCP);
  show("Write Config");
}
void ReadConfig()
{
  staSSID = ReadStringFromEEPROM(ADDR_STASSID);
  staPASS = ReadStringFromEEPROM(ADDR_STAPASS);
  apSSID = ReadStringFromEEPROM(ADDR_APSSID);
  apPASS = ReadStringFromEEPROM(ADDR_APPASS);
  portTCP = atol(ReadStringFromEEPROM(ADDR_PORTTCP).c_str());
  show("Read Config");
  String str = staSSID + "\n" + staPASS + "\n" + apSSID + "\n" + apPASS + "\n" + portTCP; 
  show(str);
}

void AccessPoint()
{
  show("Access Point Config");
  WiFi.disconnect();
  delay(1000);
  // Wait for connection
  show( WiFi.softAP(apSSID.c_str(),apPASS.c_str()) ? "Ready" : "Failed!");
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  SoftIP = ""+(String)myIP[0] + "." + (String)myIP[1] + "." +(String)myIP[2] + "." +(String)myIP[3];
  Serial.println(SoftIP);
}

void ConnectWifi(long timeOut)
{
  show("Connect to other Access Point");
  delay(1000);
  int count = timeOut / 100;
  show("Connecting");
  show(staSSID);
  show(staPASS);
  WiFi.begin(staSSID.c_str(),staPASS.c_str());
  
  while (WiFi.status() != WL_CONNECTED && --count > 0) {
    delay(100);
    Serial.print(".");
  }
  if (count > 0){
    show("Connected");
    IPAddress myIP = WiFi.localIP();
    LocalIP = ""+(String)myIP[0] + "." + (String)myIP[1] + "." +(String)myIP[2] + "." +(String)myIP[3];
    show("Local IP :"); 
    show(LocalIP);
  }else show("Disconnect");
}

void StartServer()
{
  server.on("/", webConfig);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void webConfig() {
  GiaTriThamSo();
  String html = Title();
  if (isLogin)
    html += ContentConfig();
  else 
    html += ContentLogin();
  server.send ( 200, "text/html",html);
}
String SendTRWeb()
{
  String s="";
//  for (int i=0;i<SoTiet;i++)
//    s+="<tr ><td> "+String (i+1)+" </td><td> Tiết "+String (i+1)+"</td><td> "+ConvertLongToSringTime(TimeHoc[i][0])  +"- "+ ConvertLongToSringTime(TimeHoc[i][1] )+"</td></tr>";
  return s;
}
String Title(){
  String html = "<html>\
  <head>\
  <meta charset=\"utf-8\">\
  <title>Config</title>\
  <style>\
    * {margin:0;padding:0}\
    body {width: 600px;height: auto;border: red 3px solid; margin: 0 auto; box-sizing: border-box}\
    .head1{ display: flex; height: 50px;border-bottom: red 3px solid;}\
    .head1 h1{margin: 0 auto;}\
    table, th, td { border: 1px solid black;border-collapse: collapse;}\
    tr{ height: 40px;text-align: center;font-size: 20px;}\
    input { height: 25px;text-align: center;}\
    button {height: 25px;width: 100px;margin: 5px;}\
    button:hover {background: #ccc; font-weight: bold; cursor: pointer;}\
    .subtitle {text-align: left;font-weight: bold;}\
    .content {padding: 10px 20px;}\
    .left , .right { width: 50%; float: left;text-align: left;line-height: 25px;padding: 5px 0;}\
    .left {text-align: right}\
    .listBtn {text-align: center}\
    a {text-decoration: none;}\
  </style>\
  </head>";
  return html;
}
String ContentLogin(){
  String content = "<body>\
    <div class=\"head1\">\
      <h1>Login Config</h1>\
    </div>\
    <div class=\"content\">\
      <form action=\"\" method=\"get\">\
        <div class=\"left\">Name Access Point </div>\
        <div class=\"right\">: <input class=\"input\" placeholder=\"Tên wifi\" name=\"txtNameAP\" required></div>\
        <div class=\"left\">Password Port TCP</div>\
        <div class=\"right\">: <input class=\"input\" placeholder=\"Cổng TCP\" name=\"txtPassPortTCP\" required></div>\
        <div class=\"listBtn\">\
      <button type=\"submit\">Login</button></div>\
      </form>\
    </div>\
  </body>\
  </html>";
  return content;
}
String ContentConfig(){
  String content = "<body>\
    <div class=\"head1\">\
      <h1>Setting config</h1>\
    </div>\
    <div class=\"content\">\
      <form action=\"\" method=\"get\">\
        <div class=\"subtitle\">Station mode (Connect to other Access Point)</div>\
        <div class=\"left\">Name Access Point </div>\
        <div class=\"right\">: <input class=\"input\" placeholder=\"Tên wifi\" name=\"txtStationAP\" value=\""+staSSID+"\" required></div>\
        <div class=\"left\">Password Access Point</div>\
        <div class=\"right\">: <input class=\"input\" placeholder=\"Mật khấu wifi\" name=\"txtStationPassAP\" value=\""+staPASS+"\"></div>\
        <div class=\"left\">Status</div>\
        <div class=\"right\">: Disconnect</div>\
        <div class=\"subtitle\">Access Point mode (This is a Access Point)</div>\
        <div class=\"left\">Name WIFI </div>\
        <div class=\"right\">: <input class=\"input\" placeholder=\"Tên wifi phát ra\" name=\"txtNameStationAP\" value=\""+apSSID+"\" required></div>\
        <div class=\"left\">Password WIFI</div>\
        <div class=\"right\">: <input class=\"input\" placeholder=\"Mật khấu wifi phát ra\" name=\"txtPassStationAP\" value=\""+apPASS+"\"></div>\
        <div class=\"subtitle\">TCP Server</div>\
        <div class=\"left\">Soft IP</div>\
        <div class=\"right\">: <input class=\"input\" placeholder=\"xxx.xxx.xxx.xxx\" disabled value=\""+LocalIP+"\"></div>\
        <div class=\"left\">Local IP</div>\
        <div class=\"right\">: <input class=\"input\" placeholder=\"xxx.xxx.xxx.xxx\" disabled value=\""+SoftIP+"\"></div>\
        <div class=\"left\">MAC Address</div>\
        <div class=\"right\">: <input class=\"input\" placeholder=\"xx-xx-xx-xx-xx-xx\" disabled></div>\
        <div class=\"left\">Port</div>\
        <div class=\"right\">: <input type=\"number\" min=\"0\" class=\"input\" placeholder=\"1234\" name=\"txtPortTCP\" value=\""+String(portTCP)+"\" required></div>\
        <hr>\
        <div class=\"listBtn\">\
          <button type=\"submit\"><a href=\"\">Refresh</a></button>\
          <button type=\"submit\" name=\"btnSave\" value=\"true\">Save</button><button type=\"submit\"><a href=\"?txtRestart=true\">Restart</a></div>\
      </form>\
    </div>\
  </body>\
  </html>";
  return content;
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
  String UserName, PassWord;
  for (uint8_t i=0; i<server.args(); i++){
     
    String Name=server.argName(i); 
    String Value=String( server.arg(i)) ;
    String s1=Name+ ": " +Value;
    Serial.println(s1);
//    txtNameAP=dfdf&txtPassPortTCP=dfdsf
//    txtStationAP=G&txtStationPassAP=132654789&txtNameStationAP=ESP8266&txtPassStationAP=12345678&txtPortTCP=123
//    staSSID = ReadStringFromEEPROM(ADDR_STASSID);
//    staPASS = ReadStringFromEEPROM(ADDR_STAPASS);
//    apSSID = ReadStringFromEEPROM(ADDR_APSSID);
//    apPASS = ReadStringFromEEPROM(ADDR_APPASS);
//    portTCP = atol(ReadStringFromEEPROM(ADDR_PORTTCP).c_str());

    if (Name.indexOf("txtNameAP") >= 0)
      UserName =  Value ;
    else  if (Name.indexOf("txtPassPortTCP") >= 0)
      PassWord =  Value ;
    if (Name.indexOf("txtStationAP") >= 0)
      staSSID =  Value ;
    else if (Name.indexOf("txtStationPassAP") >= 0)
      staPASS =  Value ;
    else if (Name.indexOf("txtNameStationAP") >= 0)
      apSSID =  Value ;
    else if (Name.indexOf("txtPassStationAP") >= 0)
      apPASS =  Value ;
    else if (Name.indexOf("txtPortTCP") >= 0)
      portTCP =  atol(Value.c_str());
    if (Name.indexOf("btnSave") >= 0)
    {
      WriteConfig();
      show("Save config");
    }else if (Name.indexOf("txtRestart") >= 0)
    {
      setup();
      show("Restart Device");
    }
  }
  if (UserName.equals(apSSID) && PassWord.equals(String(portTCP)))
    isLogin = true;
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


