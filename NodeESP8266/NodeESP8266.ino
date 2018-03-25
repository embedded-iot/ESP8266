#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiServer.h>
#include <EEPROM.h>
#include <WiFiUdp.h>

ESP8266WebServer server(80);

#define RESET 4 
#define VT A0
// #define RESET 4 
// #define VT 5
#define D0 16
#define D1 14
#define D2 12
#define D3 13
#define LED 2
// #define SERVER_PIN A0 
#define SERVER_PIN 5 

#define DEBUGGING
#define RFTEST true
#define RFCHANNEL 15
#define LENGTH_BUFFER_RF 10


#define ADDR 0
#define ADDR_STASSID (ADDR)
#define ADDR_STAPASS (ADDR_STASSID+20)
#define ADDR_STAIP (ADDR_STAPASS+20)
#define ADDR_STAGATEWAY (ADDR_STAIP+20)
#define ADDR_STASUBNET (ADDR_STAGATEWAY+20)

#define ADDR_APSSID (ADDR_STASUBNET+20)
#define ADDR_APPASS (ADDR_APSSID+20)
#define ADDR_APIP (ADDR_APPASS+20)
#define ADDR_APGATEWAY (ADDR_APIP+20)
#define ADDR_APSUBNET (ADDR_APGATEWAY+20)

#define ADDR_PORTTCP (ADDR_APSUBNET+20)
#define ADDR_RFCONFIG (ADDR_PORTTCP+20)

#define ID_DEFAULT 9
#define NAME_DEFAULT "MBELL"

#define TIME_LIMIT_RESET 3000
#define PORT_TCP_DEFAULT 333
// Start a TCP Server on port 333
WiFiServer tcpServer(PORT_TCP_DEFAULT);
#define PORT_UDP_DEFAULT 4210
WiFiUDP Udp;

#define PTRO1
#ifdef  PTRO
  #define STA_SSID_DEFAULT "G"
  #define STA_PASS_DEFAULT "132654789"
  #define AP_SSID_DEFAULT NAME_DEFAULT + String(ID_DEFAULT)
  #define AP_PASS_DEFAULT ""
  #define STA_IP_DEFAULT "192.168.0.122"
  #define STA_GATEWAY_DEFAULT "192.168.0.1"
  #define STA_SUBNET_DEFAULT "255.255.255.0"
  #define AP_IP_DEFAULT "192.168." + String(ID_DEFAULT) + ".1"
  #define AP_GATEWAY_DEFAULT "192.168." + String(ID_DEFAULT) + ".1"
  #define AP_SUBNET_DEFAULT "255.255.255.0"
#else 
  #define STA_SSID_DEFAULT NAME_DEFAULT + String(ID_DEFAULT - 1)
  #define STA_PASS_DEFAULT ""
  #define AP_SSID_DEFAULT NAME_DEFAULT +String(ID_DEFAULT)
  #define AP_PASS_DEFAULT ""
  #define STA_IP_DEFAULT "192.168." + String(ID_DEFAULT - 1) + ".122"
  #define STA_GATEWAY_DEFAULT "192.168." + String(ID_DEFAULT - 1) + ".1"
  #define STA_SUBNET_DEFAULT "255.255.255.0"
  #define AP_IP_DEFAULT "192.168." + String(ID_DEFAULT) + ".1"
  #define AP_GATEWAY_DEFAULT "192.168." + String(ID_DEFAULT) + ".1"
  #define AP_SUBNET_DEFAULT "255.255.255.0"
#endif

IPAddress broadCast;

bool isServer =  false; 
bool flagClear = false;

bool isLogin = false;
bool isConnectAP = false;
String staSSID, staPASS;
String staIP, staGateway, staSubnet;
String apSSID, apPASS;
String apIP, apGateway, apSubnet;
String SoftIP, LocalIP;

String MAC;
long portTCP;
long timeStation = 7000;

String bufferRF[LENGTH_BUFFER_RF+1];
int flagRF[LENGTH_BUFFER_RF+1];
String channelRF[RFCHANNEL+1];

long udpPort = 4210;
char incomingPacket[255];
String receivedUDP;
int idWebSite = 0;

void DEBUG(String s);
bool isValidStringIP(String strIP);
void GPIO();
int ScanRF();
void SaveStringToEEPROM(String data,int address);
String ReadStringFromEEPROM(int address);
void ClearEEPROM();
void AccessPoint();
void ConnectWifi(long timeOut);
void GiaTriThamSo();
String listenUDP();
IPAddress convertStringToIPAddress(String stringIP);
String ContentVerifyRestart();
String ContentLogin();
String SendTRRFConfig();
String ContentConfig();
String webView();
String SendTRViewHome();
void GiaTriThamSo();


void setup()
{
  delay(500);
  Serial.begin(115200);
  idWebSite = 0;
  isLogin = false;
  WiFi.disconnect();
  EEPROM.begin(512);
  GPIO();
  if (digitalRead(SERVER_PIN) == LOW) {
    isServer = true;
    show("I am server!");
  }
    
  if (EEPROM.read(500) != 255 || flagClear){
    ClearEEPROM();
    ConfigDefault();
    WriteConfig();
  }
  ReadConfig();
  delay(1000);

  WiFi.mode(WIFI_AP_STA);
  delay(1000);
  ConfigNetwork();
  
  //ConnectWifi(4000);
  //isConnectAP
  //AccessPoint();
  delay(1000);
  ConnectWifi(timeStation); 
  Serial.println("Begin TCP Server");
  tcpServer.begin(); // Start the TCP server port 333
  delay(1000);
  if (isConnectAP == false)
  {
    //WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    show("Set WIFI_AP");
    //ConfigNetwork();
  }
  AccessPoint();

  delay(1000);
 
  StartServer();
  
  // Setup the UDP port
  if (isConnectAP == true){
   
  }
  show("begin UDP port");
  Udp.begin(udpPort);
  String strBroadCast = staIP.substring(0,staIP.lastIndexOf(".")) + ".255";
  broadCast = convertStringToIPAddress(strBroadCast);
  show("IP broadCast");
  show(broadCast.toString());
  digitalWrite(LED,HIGH);
  delay(1000);
  // Notify: Connect AP success. 
  if (isConnectAP == false){
    blinkLed(3,500);
  }
  show("End Setup()");
  delay(1000);
}

WiFiClient client ;
long timeLogout = 30000;
long t = 0;
long timeRandom = 10000;
long t1 = 0;
long intNumber = 0;
String resultRF = "";
int idChannel = -1;
bool flagForward = false;

long tStation = 0;
long timeReconectToOtherAP = 2 * 60 * 1000; // 10 phut

void loop()
{
  server.handleClient();
  if (millis() - t > timeLogout) {
    isLogin = false;
    t = millis();
  }

  if (WiFi.status() != WL_CONNECTED && millis() - tStation > timeReconectToOtherAP) {
    show("Restart Device, Reconnect AP");
    tcpServer.close();
    // show("Reconnect To Other AP - " + staSSID);
    // digitalWrite(LED,LOW);
    // ConnectWifi(timeStation);
    // digitalWrite(LED,HIGH);
    // WiFiServer tcpServer(PORT_TCP_DEFAULT);
    // tcpServer.begin(); // Start the TCP server port 333
    // delay(1000);
    // if (isConnectAP == true){
    //   show("begin UDP port");
    //   Udp.begin(udpPort);
    //   blinkLed(3,500);
    //   show("Connected To Other AP - " + staSSID);
    // } else {
    //   show("Not connected To Other AP - " + staSSID);
    // }
    setup();
    tStation = millis();
  }

  if (digitalRead(RESET) == LOW)
  {
    //ConfigDefault();
    long t=TIME_LIMIT_RESET/100;
    while (digitalRead(RESET)==LOW && t-- >= 0){
      delay(100);
    }
    if (t < 0){
      show("RESET DEFAULT CONFIG");
      // while (digitalRead(RESET)==LOW);
      ConfigDefault();
      WriteConfig();
      setup();
    }
  }
  
  #if RFTEST
    //resultIdRF = ListenRF();
    idChannel = ListenIdRF();
   
  #else 
    if (millis() - t1 > timeRandom) {
      idChannel = random(RFCHANNEL);
//      //resultRF = "#S#" + apSSID + "##VIP" +String(intNumber++) + "#E#";
//      String data = getStringByIdChannelRF(rd);
//      resultRF = EncodePacket(apSSID, data);
//      //show(resultRF);
//      pushBufferRF(resultRF);
      
      t1 =  millis();
    }
    else resultRF = "";
  #endif

  if (idChannel != -1) {
    String data = getStringByIdChannelRF(idChannel);
    String packet = EncodePacket(apSSID, data);
    resultRF = packet;
    pushBufferRF(resultRF);
    idChannel = -1;
  }else resultRF = "";
  
  receivedUDP = listenUDP();
  
      
  if (receivedUDP.length() > 0)
  {
    show(receivedUDP);
    SendUdp(broadCast.toString(), udpPort, receivedUDP);
    pushBufferRF(receivedUDP);
  }

  if (isConnectAP && resultRF.length() > 0) 
  {
    digitalWrite(LED,LOW);
    delay(50);
    show(broadCast.toString() + "-" + resultRF);
    SendUdp(broadCast.toString(), udpPort, resultRF);
    digitalWrite(LED,HIGH);
  }
  // Nếu là server thì hiển thị dữ liệu của cả những nút con
  if (isServer && receivedUDP.length() > 0) {
    resultRF = receivedUDP;
    pushBufferRF(resultRF);
  }
  
  // if () {
  //    if (!client.connected()) {
  //      client.close();
  //    }
  // }

  //client = tcpServer.available();
  if (!client.connected()) {
        // try to connect to a new client
        client = tcpServer.available();
    } else {
      // read data from the connected client
      if (client.available() > 0) {
        String stringClient = client.readString();
        stringClient += "OK";
        if (stringClient.indexOf("#S#") >= 0) {
          pushBufferRF(stringClient);
          SendUdp(broadCast.toString(), udpPort, stringClient);
          show("+IPD:" + stringClient);  
        }
        else show("+IPD:" + stringClient);  
      }
      if (resultRF.length() > 0) {
        digitalWrite(LED,LOW);
        show(resultRF);
        if (!client.println(resultRF.c_str())) {
          // client.stop();
          show("CLient timeout");
          tcpServer.close();
        }
        else show("SEND OK");
        delay(50);
        digitalWrite(LED,HIGH);
      }
  }

  delay(50);
}
void show(String s)
{
  #ifdef DEBUGGING 
    Serial.println(s);
  #endif
}


void blinkLed(int numberRepeat, long tdelay) {
  int i=0;
  while (i++ < numberRepeat) {
    digitalWrite(LED,LOW);delay(tdelay);
    digitalWrite(LED,HIGH);delay(tdelay);
  }
}
String EncodePacket(String address, String data){
  return "#S#" + address + "##" +data+ "#E#";
}
void GPIO()
{
  show("GPIO");
// pinMode(PINCHUONG,OUTPUT);
// digitalWrite(PINCHUONG,HIGH);
  pinMode(LED,OUTPUT);
  digitalWrite(LED,LOW);
  pinMode(RESET,INPUT_PULLUP);
  pinMode(SERVER_PIN,INPUT_PULLUP); 
  pinMode(D0,INPUT_PULLUP); 
  pinMode(D1,INPUT_PULLUP); 
  pinMode(D2,INPUT_PULLUP); 
  pinMode(D3,INPUT_PULLUP); 

}

void ChannelRFDefault(){
  for (int i=0;i< RFCHANNEL ; i++){
     channelRF[i] = "VIP "+ String(i);
  }
}

String getStringByIdChannelRF(int id){
  if (id >=0 && id < RFCHANNEL)
    return channelRF[id];
  return "";
}
void ConfigNetwork(){
 // Fire up wifi station
//  show("Station configuration ... ");
//  IPAddress STAlocal_IP = convertStringToIPAddress(staIP);
//  IPAddress STAgateway = convertStringToIPAddress(staGateway);
//  IPAddress STAsubnet = convertStringToIPAddress(staSubnet);
//  show(STAlocal_IP.toString());
//  show(STAgateway.toString());
//  show(STAsubnet.toString());
//  show(WiFi.config(STAlocal_IP, STAgateway, STAsubnet) ? "OK" : "Failed!");
 // Configure the Soft Access Point. Somewhat verbosely... (for completeness sake)
 show("Soft-AP configuration ... ");
 IPAddress APlocal_IP = convertStringToIPAddress(apIP);
 IPAddress APgateway = convertStringToIPAddress(apGateway);
 IPAddress APsubnet = convertStringToIPAddress(apSubnet);
 show(APlocal_IP.toString());
 show(APgateway.toString());
 show(APsubnet.toString());
 show(WiFi.softAPConfig(APlocal_IP, APgateway, APsubnet) ? "OK" : "Failed!"); // configure network
}
IPAddress convertStringToIPAddress(String stringIP) 
{
  IPAddress IP ;
  IP.fromString(stringIP);
  return (IP);
}
bool isValidStringIP(String strIP){
  IPAddress IP ;
  if (IP.fromString(strIP)){
    //show("IP true");
    return true;
  }
  //show("IP false");
  return false;
}
String ListenRF()
{
  int Di = -1;
  if (analogRead(VT) > 500)
  {
    show("VT HIGH");
    Di = ScanRF();
  }
  if (Di != -1)
  {
    if (Di == D0)
      return "D0";
    else if (Di == D1)
      return "D1";
    else if (Di == D2)
      return "D2";
    else if (Di == D3)
      return "D3";
  }
  return "";
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
int RFPIN[4] = {D0, D1, D2, D3};
int ListenIdRF()
{
  int Di = -1;
 
  if (analogRead(VT) > 500)
  {
    int timeOut = millis();
    show("VT HIGH");
    while (analogRead(VT) > 500 && millis() - timeOut < 2000){
       Di = -1;
       for (int i = 0;i<4; i++){
         if (digitalRead(RFPIN[i]) == HIGH) {
          Di += pow(2, i);
         }
       }
    }
    show(String(Di));
  }
  return Di;
}
void SendUdp(String address , long localUdpPort, String data){
 Udp.beginPacket(address.c_str(), localUdpPort);
 Udp.write(data.c_str());
 Udp.endPacket();
}
String listenUDP(){
 int packetSize = Udp.parsePacket();
 if (packetSize)
 {
  show("Received UDP packet");
  show("Received" + String(packetSize) + "bytes form "+ Udp.remoteIP().toString() + " ,port " + Udp.remotePort());
  int len = Udp.read(incomingPacket, 255);
  if (len > 0){
   incomingPacket[len] = 0;
  }
  Serial.printf("UDP packet contents: %s\n", incomingPacket);
  show("UDP packet:"+ String(incomingPacket));
  return String(incomingPacket);
 }
 return "";
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
  staIP = STA_IP_DEFAULT;
  staGateway = STA_GATEWAY_DEFAULT;
  staSubnet = STA_SUBNET_DEFAULT;
  apSSID = AP_SSID_DEFAULT;
  apPASS = AP_PASS_DEFAULT;
  apIP = AP_IP_DEFAULT;
  apGateway = AP_GATEWAY_DEFAULT;
  apSubnet = AP_SUBNET_DEFAULT;
  portTCP = PORT_TCP_DEFAULT;

  ChannelRFDefault();
  show("Config Default");
}
void WriteConfig()
{
  SaveStringToEEPROM(staSSID, ADDR_STASSID);
  SaveStringToEEPROM(staPASS, ADDR_STAPASS);
  SaveStringToEEPROM(staIP, ADDR_STAIP);
  SaveStringToEEPROM(staGateway, ADDR_STAGATEWAY);
  SaveStringToEEPROM(staSubnet, ADDR_STASUBNET);
  SaveStringToEEPROM(apSSID, ADDR_APSSID);
  SaveStringToEEPROM(apPASS, ADDR_APPASS);
  SaveStringToEEPROM(apIP, ADDR_APIP);
  SaveStringToEEPROM(apGateway, ADDR_APGATEWAY);
  SaveStringToEEPROM(apSubnet, ADDR_APSUBNET);
  SaveStringToEEPROM(String(portTCP), ADDR_PORTTCP);

  for (int i = 0; i< RFCHANNEL; i++)
  {
    SaveStringToEEPROM(channelRF[i], ADDR_RFCONFIG + i*10);
  }
  
  show("Write Config");
}
void ReadConfig()
{
  staSSID = ReadStringFromEEPROM(ADDR_STASSID);
  staPASS = ReadStringFromEEPROM(ADDR_STAPASS);
  staIP = ReadStringFromEEPROM(ADDR_STAIP);
  staGateway = ReadStringFromEEPROM(ADDR_STAGATEWAY);
  staSubnet = ReadStringFromEEPROM(ADDR_STASUBNET);
  apSSID = ReadStringFromEEPROM(ADDR_APSSID);
  apPASS = ReadStringFromEEPROM(ADDR_APPASS);
  apIP = ReadStringFromEEPROM(ADDR_APIP);
  apGateway = ReadStringFromEEPROM(ADDR_APGATEWAY);
  apSubnet = ReadStringFromEEPROM(ADDR_APSUBNET);
  portTCP = atol(ReadStringFromEEPROM(ADDR_PORTTCP).c_str());
  show("Read Config");
  String str = "Station: \n" + staSSID + "\n" + staPASS + "\n" + staIP + "\n" + staGateway + "\n" + staSubnet; 
  show(str);
  str = "Access Point: \n" + apSSID + "\n" + apPASS + "\n" + apIP + "\n" + apGateway + "\n" + apSubnet; 
  show(str);
  show("Channel RF:");
  for (int i = 0; i< RFCHANNEL; i++)
  {
    channelRF[i] = ReadStringFromEEPROM(ADDR_RFCONFIG + i*10);
    flagRF[i] = 0;
    show(channelRF[i]);
  }
  
}

void AccessPoint()
{
  show("Access Point Config");
  //WiFi.disconnect();
  delay(1000);
  // Wait for connection
  show( WiFi.softAP(apSSID.c_str(),apPASS.c_str()) ? "Ready" : "Failed!");
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  SoftIP = ""+(String)myIP[0] + "." + (String)myIP[1] + "." +(String)myIP[2] + "." +(String)myIP[3];
  Serial.println(SoftIP);
  byte mac[6];
  WiFi.macAddress(mac);
  MAC = String(mac[0],HEX)+ "-" + String(mac[1],HEX)+ "-" + String(mac[2],HEX)+ "-" + String(mac[3],HEX)+ "-" + String(mac[4],HEX)+ "-" + String(mac[5],HEX);;
  show("MAC");
  show(MAC);
}


void ConnectWifi(long timeOut)
{
  show("Connect to other Access Point");
  delay(1000);
  int count = timeOut / 200;
  show("Connecting");
  show(staSSID);
  show(staPASS);
  WiFi.begin(staSSID.c_str(),staPASS.c_str());
  
  while (WiFi.status() != WL_CONNECTED && --count > 0) {
    delay(200);
    Serial.print(".");
  }
  if (count > 0){
    show("Connected");
    IPAddress myIP = WiFi.localIP();
    LocalIP = ""+(String)myIP[0] + "." + (String)myIP[1] + "." +(String)myIP[2] + "." +(String)myIP[3];
    show("Local IP :"); 
    show(LocalIP);
    staIP =  LocalIP;
    apGateway = staIP.substring(0,staIP.lastIndexOf(".")) + ".1";
    isConnectAP = true;
  }else {
    isConnectAP = false;
    show("Disconnect");
  }
}


void StartServer()
{
  server.on("/", webConfig);
  server.on("/rfconfig", webRFConfig);
  if (isServer){
    server.on("/home", webViewHome);
  }
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void webConfig() {
  GiaTriThamSo();
  String html = Title();
  if (idWebSite == 0) {
    html += ContentLogin();
  }
  else if (idWebSite == 1) {
    html += ContentConfig();
  }
  else if (idWebSite == 2) {
    html += ContentVerifyRestart();
  }else html += ContentLogin();
  server.send ( 200, "text/html",html);
}
void webRFConfig() {
  String html = Title();
  html += ChannelRFConfig();
  server.send ( 200, "text/html",html);
}

void webViewHome() {
  String html = Title();
  html += webView();
  server.send ( 200, "text/html",html);
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
    .head1 h1{margin: auto;}\
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
    table {width: 100%;}\
    .column {width: 50%;text-align: center;}\
    .noboder {border: none;}\
    .card-rf {background: yellow;color: red;font-size: 90px;text-align: center;}\
    .tr-active {background: #0095ff !important;}\
  </style>\
  </head>";
  return html;
}
String ContentVerifyRestart() {
  String content = "<body>\
    <div class=\"head1\">\
      <h1>Restart Device</h1>\
    </div>\
    <div class=\"content\">\
      <form action=\"\" method=\"get\">\
      <div class=\"subtitle\">Do you want restart device?</div>\
      <div class=\"listBtn\">\
        <button type=\"submit\" name=\"txtVerifyRestart\" value=\"false\">No</button>\
        <button type=\"submit\" name=\"txtVerifyRestart\" value=\"true\">Yes</button>\
      <div>\
      </form>\
    </div>\
  </body>\
  </html>";
  return content;
}
String ContentLogin(){
  String content = "<body>\
    <div class=\"head1\">\
      <h1>Login Config</h1>\
    </div>\
    <div class=\"content\">\
      <form action=\"\" method=\"get\">\
        <div class=\"left\">Name Access Point </div>\
        <div class=\"right\">: <input class=\"input\" placeholder=\"Tên wifi\" name=\"txtNameAP\" value=\""+apSSID+"\" required></div>\
        <div class=\"left\">Password Port TCP</div>\
        <div class=\"right\">: <input class=\"input\" type=\"password\" placeholder=\"Mat khau\" name=\"txtPassPortTCP\" value=\"\" required></div>\
        <div class=\"listBtn\">\
        <a href=\"/home\" target=\"_blank\">Visit Home Page!</a>\
      <button type=\"submit\">Login</button></div>\
      </form>\
    </div>\
  </body>\
  </html>";
  return content;
}
String DisplayStationIP() {
  String html;
  if (isConnectAP) {
    return "<div class=\"left\">AP IP</div>\
        <div class=\"right\">: <input class=\"input\" placeholder=\"xxx.xxx.xxx.xxx\" disabled name=\"txtSTAIP\" value=\""+staIP+"\"></div>";
  }
  return "";
}
String ContentConfig(){
  String strBroadCast = staIP.substring(0,staIP.lastIndexOf(".")) + ".255";
  String content = "<body>\
    <div class=\"head1\">\
      <h1>Setting config</h1>\
    </div>\
    <div class=\"content\">\
      <form action=\"\" method=\"get\">\
        <div class=\"subtitle\">Station mode (Connect to other Access Point)</div>\
        <div class=\"left\">Name Access Point </div>\
        <div class=\"right\">: <input class=\"input\" placeholder=\"Tên wifi\" name=\"txtSTAName\" value=\""+staSSID+"\" required></div>\
        <div class=\"left\">Password Access Point</div>\
        <div class=\"right\">: <input class=\"input\" placeholder=\"Mật khấu wifi\" name=\"txtSTAPass\" value=\""+staPASS+"\"></div>\
        " + DisplayStationIP() + "\
        <div class=\"left\">Status</div>\
        <div class=\"right\">: "+(isConnectAP == true ? "Connected" : "Disconnect")+"</div>\
        <div class=\"subtitle\">Access Point mode (This is a Access Point)</div>\
        <div class=\"left\">Name</div>\
        <div class=\"right\">: <input class=\"input\" placeholder=\"Tên wifi phát ra\" name=\"txtAPName\" value=\""+apSSID+"\" required></div>\
        <div class=\"left\">Password</div>\
        <div class=\"right\">: <input class=\"input\" placeholder=\"Mật khấu wifi phát ra\" name=\"txtAPPass\" value=\""+apPASS+"\"></div>\
        <div class=\"left\">Soft IP</div>\
        <div class=\"right\">: <input class=\"input\" placeholder=\"xxx.xxx.xxx.xxx\" name=\"txtAPIP\" disabled value=\""+apIP+"\"></div>\
        <div class=\"left\">MAC Address</div>\
        <div class=\"right\">: <input class=\"input\" placeholder=\"xx-xx-xx-xx-xx-xx\" disabled value=\""+MAC+"\"></div>\
        <div class=\"subtitle\">TCP/UDP Server</div>\
        <div class=\"left\">TCP PORT</div>\
        <div class=\"right\">: <input type=\"number\" min=\"0\" class=\"input\" disabled value=\""+String(PORT_TCP_DEFAULT)+"\" ></div>\
        <div class=\"left\">IP Broadcast</div>\
        <div class=\"right\">: <input min=\"0\" class=\"input\" disabled value=\""+ strBroadCast +"\" ></div>\
        <div class=\"left\">UPD PORT</div>\
        <div class=\"right\">: <input type=\"number\" min=\"0\" class=\"input\" disabled value=\""+String(PORT_UDP_DEFAULT)+"\" ></div>\
        <hr>\
        <div class=\"listBtn\">\
          <button type=\"submit\"><a href=\"?txtRefresh=true\">Refresh</a></button>\
          <button type=\"submit\" name=\"btnSave\" value=\"true\">Save</button>\
          <button type=\"submit\"><a href=\"?txtRestart=true\">Restart</a></button>\
          <button type=\"submit\"><a href=\"?txtLogout=true\">Logout</a></button>\
        </div>\
        <hr>\
        <a href=\"/rfconfig\" target=\"_blank\">Visit RF Config Page!</a>\
      </form>\
    </div>\
  </body>\
  </html>";
  return content;
}

String ChannelRFConfig(){
  GiaTriThamSo();
  String content = "<body>\
    <div class=\"head1\">\
    <h1>Setting RF</h1>\
    </div>\
    <div class=\"content\">\
      <form action=\"\" method=\"get\">\
        <table><tr class=\"row\"><th>ID RF</th><th>NAME RF</th></tr>"+ SendTRRFConfig() +"</table>\
        <br><hr>\
        <div class=\"listBtn\">\
          <button type=\"submit\"><a href=\"/rfconfig?\">Refresh</a></button>\
          <button type=\"submit\">Save</button>\
          <button type=\"submit\"><a href=\"/\">Login</a></button>\
        </div>\
      </form>\
    </div>\
  </body>\
  </html>";
  return content;
}

String SendTRRFConfig()
{
  String s="";
  for (int i=0;i< RFCHANNEL ;i++) {
    String id = (i < 10 ? "0" + String(i) : String(i));
    s += "<tr class=\"row\"><td class=\"column\">"+ id +"</td><td class=\"column\"><input type=\"text\" class=\"input noboder\" maxlength=\"10\" placeholder=\"Tên bàn\" name=\"txtChannelRF"+id+"\" value=\""+ channelRF[i] +"\" required></td></tr>";
  }
  //show(s);
  return s;
}
String webView(){
  String content = "<body>\
    <div class=\"head1\">\
    <h1>HOME PAGE</h1>\
    </div>\
    <div class=\"card-rf " + isTrActive(0) + "\">"+ getData(bufferRF[0]) +"</div>\
    <div class=\"content\">\
    <form action=\"\" method=\"get\">\
      <table>\
      <tr class=\"row\"><th>ID</th><th>NAME RF</th></tr>"+ SendTRViewHome() +"\
      </table>\
      <br><hr>\
      <div class=\"listBtn\">\
      <button type=\"submit\"><a href=\"/\">Login</a></button>\
      </div>\
    </form>\
    <script type=\"text/javascript\">\
      setInterval(function() {\
      window.location.reload();\
      }, 2000);\
    </script>\
    </div>\
  </body>\
  </html>";
  return content;
}
String SendTRViewHome()
{
  String s="";
  for (int i = 1;i< LENGTH_BUFFER_RF ;i++) {
    String id = (i < 10 ? "0" + String(i) : String(i));
    if (bufferRF[i].length() > 0)
      s += "<tr class=\"row" + isTrActive(i) + "\"><td class=\"column\">"+ id + "-" + getAddress(bufferRF[i]) +"</td><td class=\"column\">"+ getData(bufferRF[i]) +"</td></tr>";
  }
  return s;
}

String isTrActive(int id) {
  if (flagRF[id] == 1 )
    return " tr-active";
  return "";
}
void pushBufferRF(String packet){
//  if (isCheckPacket(packet) == false);
//    return ;
  int i1 = packet.indexOf("#S#");
  int i2 = packet.indexOf("#E#");
  if (i1 >= i2){
    show("Error packet : " + packet);
    return ;
  }
    
  if (packet.indexOf("OK") > 0) {
    String nameAP = getAddress(packet);
    String strData = getData(packet);
    for (int i = 0 ; i< LENGTH_BUFFER_RF ; i++){
      if (bufferRF[i].indexOf(nameAP) >= 0 && bufferRF[i].indexOf(strData+"#E") >= 0) {
        flagRF[i] = 1;
        show(bufferRF[i] + "=>Accept!");
      }
    }
    return ;
  }

  String tg[LENGTH_BUFFER_RF+1];
  int i = 0;
  int tgFlag[LENGTH_BUFFER_RF+1];

  tg[0] = packet;
  tgFlag[0] = 0;
  for (i = 1 ; i< LENGTH_BUFFER_RF ; i++){
    tg[i] = bufferRF[i-1];
    tgFlag[i] = flagRF[i-1];
  }
  for (i = 0 ; i< LENGTH_BUFFER_RF ; i++){
    bufferRF[i] = tg[i];
    flagRF[i] = tgFlag[i];
  } 
}
//getAddress("#S#apSSID##VIP 01#E#");
//packet = "#S#apSSID#VIP 01#E#";
// return VIP 01;
String getData(String packet){
  String s1 = "No data";
  int i1 = packet.indexOf("##");
  int i2 = packet.indexOf("#E#");
  if (i2 > i1)
    s1 = packet.substring(i1+2,i2);
  return s1;
}
//getAddress("#S#apSSID##VIP 01#E#");
//packet = "#S#apSSID##VIP 01#E#";
// return VIP 01;
String getAddress(String packet){
  String s1 = "No Address";
  int i1 = packet.indexOf("#S#");
  int i2 = packet.indexOf("##");
  if (i2 > i1)
    s1 = packet.substring(i1+3,i2);
  return s1;
}
void GiaTriThamSo()
{
  t = millis();
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
    //show(s1);
    if (isLogin == true) {
      if (Name.indexOf("txtLogout") >= 0){
        isLogin = false;
        show("Logout");
      }
      else if (Name.indexOf("txtSTAName") >= 0){
        if (Value != staSSID && Value.length() > 0){
          staSSID =  Value ;
          show("Set staSSID : " + staSSID);
        }
      }
      else if (Name.indexOf("txtSTAPass") >= 0){
        if (Value != staPASS) {
          staPASS =  Value ;
          show("Set staPASS : " + staPASS);
        }
      }
      else if (Name.indexOf("txtSTAIP") >= 0){
        if (isValidStringIP(Value) && Value != staIP) {
          staIP =  Value ; 
          show("Set staIP : " + staIP);
        }
      }
      else if (Name.indexOf("txtSTAGateway") >= 0){
        if (isValidStringIP(Value) && Value != staGateway){
          staGateway =  Value ;
          show("Set staGateway : " + staGateway);
        }
      }
      else if (Name.indexOf("txtSTASunet") >= 0){
        if (isValidStringIP(Value) && Value != staSubnet){
          staSubnet =  Value ;
          show("Set apSSID : " + staSubnet);
        }
      }
      else if (Name.indexOf("txtAPName") >= 0){
        if (Value != apSSID && Value.length() > 0){
          String keyEnd = Value.substring(Value.length() - 1, Value.length());
          if ( keyEnd > "0" && keyEnd <= "9") {
            apSSID =  Value ;
            show("Set apSSID : " + apSSID);
            show("Auto gen IP:");
            apIP = "192.168." + keyEnd + ".1";
            apGateway = apIP;
            show(apIP);
          } else {
            show("apSSID is invalid.");
          }
        }
      }
      else if (Name.indexOf("txtAPPass") >= 0){
        if (Value != apPASS){
          if (Value.length() >= 8) { // Length password >= 8 
            apPASS =  Value ;
            show("Set apPASS : " + apPASS);
          } else {
            show("txtAPPass is invalid (Value.length() >= 8 && Value != apPASS)");
          }
          
        } 
      }
      else if (Name.indexOf("txtAPIP") >= 0){
        if (isValidStringIP(Value) && Value != apIP) {
          apIP =  Value ;  
          show("Set apIP : " + apIP);
        }
      }
      else if (Name.indexOf("txtAPGateway") >= 0){
        if (isValidStringIP(Value) && Value != apGateway){
          apGateway =  Value ;
          show("Set apGateway : " + apGateway);
         }
      }
      else if (Name.indexOf("APSubnet") >= 0){
        if (isValidStringIP(Value) && Value != apSubnet){
          apSubnet =  Value ;
          show("Set apSubnet : " + apSubnet);
        }
      }
      else if (Name.indexOf("txtPortTCP") >= 0) {
        if (Value != String(portTCP)){
          portTCP =  atol(Value.c_str());
          show("Set portTCP : " + portTCP);
        }
      }
      else if (Name.indexOf("txtRestart") >= 0){
        idWebSite = 2;
        show("Verify restart");
        show(Value);
      }
      else if (Name.indexOf("btnSave") >= 0)
      {
        WriteConfig();
        show("Save config");
      }
      else if (Name.indexOf("txtVerifyRestart") >= 0)
      {
        if ( Value.indexOf("true") >=0 ) {
          setup();
          show("Restart Device");
          idWebSite = 0;
        }
        else idWebSite = 1;
      }
    }else {
      if (Name.indexOf("txtNameAP") >= 0)
        UserName =  Value ;
      else if (Name.indexOf("txtPassPortTCP") >= 0)
        PassWord =  Value ;

      if (UserName.equals(apSSID) && PassWord.equals(String(apSSID))){
        isLogin = true;
        idWebSite = 1;
        show("Login == true");
      }else {
        idWebSite = 0;
        isLogin = false;
      }
    }
    if (Name.indexOf("txtChannelRF") >=0){
      int i1 = Name.indexOf("RF");
      if (i1 > 0){
        String strId = Name.substring(i1 + 2,i1 + 4);
        int id = strId.toInt();
        if (Value != channelRF[id]) {
          channelRF[id] = Value;
          WriteConfig();
          show("Save config");
        }
      }
    }
    
    Name = "";
    Value = "";
  }
  if (isLogin == false)
    idWebSite = 0;
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


