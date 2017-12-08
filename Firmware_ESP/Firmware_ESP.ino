/**********************************************
Firmware ESP8266 v1-12
Vision 1.0
Author : Nguyen Van Quan
Configuration Default :
  Serial baud rate : 9600 
  TCP SERVER PORT 333
  ACCESS POINT
Command AT:
- AT 
- AT+MODE?
- AT+MODE=[mode] 
- AT+CWJAP?
- AT+CWJAP="SSID","Password"
- AT+CIFSR
- AT+CWSAP?
- AT+CWSAP="SSID","Password"
- AT+HTTP="request"
- AT+CIPCLOSE
- AT+CIPSEND="data send"
Response: 
- OK : Command successfuly.
- FAIL: Command not successfuly.
- Other.
 *********************************************/

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266HTTPClient.h>
#include <EEPROM.h>

String ssid = "huy huy";
String password = "19101994";

String ap_ssid = "ESP8266";
String ap_password = "";

int Mode=1;

#define DEBUGGING1
#define ADDR 0
#define ADDR_SSID (ADDR+0)
#define ADDR_PASS (ADDR+20)
#define ADDR_APSSID (ADDR+40)
#define ADDR_APPASS (ADDR+60)
#define ADDR_MODE (ADDR+80)


// Start a TCP Server on port 333
WiFiServer server(333);

/*
 * Function DEBUG
 */
void DEBUG(String s){
    #ifdef DEBUGGING 
      Serial.println(s);
    #endif
}
/*
 * Function Read Config 
 */
void ReadAllConfig()
{
  if ((int)EEPROM.read(ADDR_SSID)<20)
    ssid=ReadStringFromEEPROM(ADDR_SSID);
  if ((int)EEPROM.read(ADDR_PASS)<20)
    password=ReadStringFromEEPROM(ADDR_PASS);
  if ((int)EEPROM.read(ADDR_APSSID)<20)
    ap_ssid=ReadStringFromEEPROM(ADDR_APSSID);
  if ((int)EEPROM.read(ADDR_APPASS)<20)
    ap_password=ReadStringFromEEPROM(ADDR_APPASS);
  if ((int)EEPROM.read(ADDR_MODE)==1 || (int)EEPROM.read(ADDR_MODE)==2)
    Mode=EEPROM.read(ADDR_MODE);
  else Mode=3;
  #ifdef DEBUGGING 
        Serial.println("STATION SSID ="+ssid);
        Serial.println("STATION PASSWORD="+password);
        Serial.println("ACCESSPOINT SSID="+ap_ssid);
        Serial.println("ACCESSPOINT PASSWORD="+ap_password);
  #endif
}
/*
 * Function ESP8266 Access Point 
 * Parameter : +nameWifi   : Name Access Point
 *             +passWifi   : Password Access Point 
 * Note : if  passWifi="" then Access Point Mode OPEN
 *        if  passWifi.length()>8 then Access Point Mode WPA2
 * Return: None.
 */
void AP_Wifi(String nameWifi,String passWifi)
{
  Serial.println( WiFi.softAP(nameWifi.c_str(),passWifi.c_str()) ? "OK" : "FAIL");
  #ifdef DEBUGGING 
      Serial.print("Configuring access point...");  
      Serial.println(nameWifi+"-"+passWifi);  
      IPAddress myIP = WiFi.softAPIP();
      Serial.print("AP IP address: ");
      Serial.println(myIP);
  #endif
  Serial.println("OK"); 
  
}
/*
 * Function ESP8266 Connect To Other Access Point
 * Parameter : +nameWifi   : Name Other Access Point
 *             +passWifi   : Password Other Access Point 
 * Note : if Access Point Mode OPEN then passWifi="" 
 *        if Access Point Mode WPA2  then passWifi.length()>8
 * Return: None.
 */
void ConnectWifi(String nameWifi,String passWifi)
{
  //WiFi.disconnect();
  WiFi.begin(nameWifi.c_str(),passWifi.c_str());
  #ifdef DEBUGGING 
      Serial.print("Connecting");
  #endif
  int d=0;
  while (WiFi.status() != WL_CONNECTED && d<60) {
        delay(100);
        d=d+1;
        #ifdef DEBUGGING 
          Serial.print(".");
        #endif
   }
  #ifdef DEBUGGING 
      if (d<60){
        Serial.println("IP local :");
        Serial.println(WiFi.localIP()); 
      }
  #endif
  if (d<60)Serial.println("OK"); 
  else Serial.println("FAIL");
}
/*
 * Function setup
 */
void setup (){
  Serial.begin(9600);
  Serial.println("Config");
  EEPROM.begin(512);
  ReadAllConfig();              // Read All Config
  WiFi.disconnect();
  if (Mode==3)
  {
    WiFi.mode(WIFI_AP_STA);
    ConnectWifi(ssid,password); //Connect to Access Point
    AP_Wifi(ap_ssid,ap_password); // ESP8266 Access Point 
  }
  else if (Mode==1)
  {
    WiFi.mode(WIFI_AP);
     AP_Wifi(ap_ssid,ap_password); // ESP8266 Access Point 
    
  }
  else if (Mode==2){
     WiFi.mode(WIFI_STA);
     ConnectWifi(ssid,password); //Connect to Access Point
    
  }
   
  
  //ConnectWifi(ssid,password);   // ESP8266 connect to other access point
  server.begin(); // Start the TCP server port 333
  Serial.println("Begin");
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
  #ifdef DEBUGGING 
    Serial.println("Write lengt "+String(len)+"\n");
  #endif
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
  #ifdef DEBUGGING 
  Serial.println("Read length "+String(len)+"\n");
  #endif
  for (int i=1;i<=len;i++)
    s+=(char)EEPROM.read(address+i);
  return s;
}
/*
 * Function HTTP_REQUEST_GET
 * Parameter : +request : Request GET from ESP To Website (or Address Website)
 * Example : request= https://www.google.com.vn/?gfe_rd=cr&ei=yBDmWPrYHubc8ge42aawBA&gws_rd=ssl#q=ESP8266&*
 * Return: Page content.
 */
void HTTP_REQUEST(String request)
{
  if(WiFi.status()== WL_CONNECTED)
  {  
    HTTPClient http;  //Declare an object of class HTTPClient
    //String yeucau="http://iotproject.comeze.com/UpData.php?Data="+String(so);
    //String yeucau="http://managerpower.comli.com/uploaddata.php?user=tranngoctk112&pass=anhngoc1995&data=123;1,2;3,4;4,5;5,25";
    //String yeucau="http://iotproject.comeze.com/UpData.php?Data="+s;
    String yeucau=request;
    http.begin(yeucau);//Specify request destination
    int httpCode= http.GET();//Send the request
      #ifdef DEBUGGING 
        Serial.println(yeucau);
        Serial.println("http code "+String(httpCode));  
      #endif 
    if(httpCode>0){    //Check the returning code

    String payload = http.getString();   //Get the request response payload
    Serial.println(payload);                     //Print the response payload
    }
    http.end();   //Close connection
  }
}
/*
 * Function SET_AT_COMMAND
 * Parameter : +commandAT    : Command AT Communication ESP
 * Return: None.
 */
void SET_AT_COMMAND(String commandAT)
{
    #ifdef DEBUGGING 
        Serial.println("\n"+commandAT);  
    #endif 
    if (commandAT.indexOf("AT")>=0 && commandAT.length()<=4)
    {
      Serial.println("OK"); 
    }else if (commandAT.indexOf("AT+MODE?")>=0){
      
       Serial.println("MODE: "+String(Mode)); 
       
    }
    else if (commandAT.indexOf("AT+MODE=")>=0){
      int index=commandAT.indexOf("=");
      String m=commandAT.substring(index+1,index+2);
      #ifdef DEBUGGING 
          Serial.println("MODE: "+m); 
      #endif
      if (m=="1" || m=="2" || m=="3")
      {
        Mode=atoi(m.c_str());
        Serial.println("MODE: "+Mode); 
        EEPROM.write(ADDR_MODE,Mode);
        EEPROM.commit();
        Serial.println("OK");
      }else Serial.println("FAIL");
      
    }     
    else if (commandAT.indexOf("AT+CWJAP?")>=0){
      if (WiFi.status()== WL_CONNECTED)
        Serial.println(ssid+"-"+password);
      else Serial.println("No AP");
    }else if (commandAT.indexOf("AT+CWJAP=")>=0){
      
      ssid= commandAT.substring(commandAT.indexOf("=\"")+2,commandAT.indexOf("\",\""));
      password= commandAT.substring(commandAT.indexOf("\",\"")+3,commandAT.lastIndexOf("\""));
    
      SaveStringToEEPROM(ssid,ADDR_SSID);
      SaveStringToEEPROM(password,ADDR_PASS);

      ssid=ReadStringFromEEPROM(ADDR_SSID);
      password=ReadStringFromEEPROM(ADDR_PASS);
      #ifdef DEBUGGING 
        Serial.println("EEPROM ssid="+ssid);
        Serial.println("EEPROM password="+password);
      #endif
     
      ConnectWifi(ssid,password);
    }else if (commandAT.indexOf("AT+CIFSR")>=0){
       Serial.print("+AP:");
       Serial.println(WiFi.softAPIP()); 
      if (WiFi.status()== WL_CONNECTED)
      {
        Serial.print("+ST:");
        Serial.println(WiFi.localIP()); 
      } 
      else Serial.println("No IP");
    }
    else if (commandAT.indexOf("AT+HTTP=")>=0)
    {
      String request=commandAT.substring(commandAT.indexOf("AT+HTTP=")+9,commandAT.lastIndexOf("\""));
      #ifdef DEBUGGING 
        Serial.println(request);
      #endif
      HTTP_REQUEST(request);
    }
    else if (commandAT.indexOf("AT+CWSAP?")>=0){
        Serial.println(ap_ssid+"-"+ap_password);
    }
    else if (commandAT.indexOf("AT+CWSAP=")>=0){
      String ap_ssid1= commandAT.substring(commandAT.indexOf("=\"")+2,commandAT.indexOf("\",\""));
      String ap_password1= commandAT.substring(commandAT.indexOf("\",\"")+3,commandAT.lastIndexOf("\""));
      if (ap_password1.length()==0 || ap_password1.length()>=8)
      {
        SaveStringToEEPROM(ap_ssid1,ADDR_APSSID);
        SaveStringToEEPROM(ap_password1,ADDR_APPASS);
  
        ap_ssid=ReadStringFromEEPROM(ADDR_APSSID);
        ap_password=ReadStringFromEEPROM(ADDR_APPASS);
     
        #ifdef DEBUGGING 
          Serial.println("EEPROM ap_ssid="+ap_ssid);
          Serial.println("EEPROM ap_password="+ap_password);
        #endif
        AP_Wifi(ap_ssid,ap_password);
      }else Serial.println("FAIL");
    }
}
WiFiClient client;
void loop() {
  if (Serial.available())
  {
    String commandAT=Serial.readString();
    SET_AT_COMMAND(commandAT);
  }
  client = server.available();
  if (client) {
    Serial.println("Client connected.");
    while (client.connected()) {
      
      if (client.available()) {
       //char command = client.read();
        String command = client.readString();
        Serial.println("+IPD:"+command);  
        
      }
      
      if (Serial.available()) {
        String commandAT=Serial.readString();
        SET_AT_COMMAND(commandAT);
        if (commandAT.indexOf("AT+CIPCLOSE")>=0)
        {
           client.stop();
           Serial.println("OK");
        }
        else if (commandAT.indexOf("AT+CIPSEND=")>=0)
        {
          String SendDataToClient=commandAT.substring(commandAT.indexOf("=\"")+2,commandAT.lastIndexOf("\""))+"\r\n";
          #ifdef DEBUGGING 
            Serial.println(SendDataToClient);
          #endif
          client.write(SendDataToClient.c_str());
          Serial.println("SEND OK");
        }
      }
      delay(10);
    }
    client.stop();
    Serial.println("Client disconnected");
  }
  delay(10);
}
