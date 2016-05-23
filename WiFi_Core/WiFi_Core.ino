#include <ESP8266WiFi.h>
#include <FS.h>

//

//States for rotable
enum StateRotable{
  Ready = 0,
  Starting = 1,
  Running = 2,  
  Finishing = 3,
  Off = 4,
  Error = 5
};

//Rotation modes
enum ModeRotable{
  None = 0,
  Long = 1,
  Fast = 2,
  Infinite = 3
};

//Mode for device
enum Mode{
  Normal = 0,
  Echo = 1
};

struct Parse{
  unsigned int crc[6];//crc for inputed txt
  byte count = 0;//txt count
  String input, prev_input, error, answer;
}parse;

struct Device{
  Mode mode;
  const String id = "Prefpro Rotable v.0.5";
  const String version = "0.0.5.6";
  //const unsigned int baudRate[] = {9600 - 0x290f, 19200 - 0x293a, 38400 - 0x2b3f, 57600 - 0x2d34, 115200 - 0x3107};
  unsigned long baudRate;
  bool interrupt;//internal interrupts on/off state
}device;


struct Rotable{
  const String version = "0.0.5.1";
  ModeRotable mode;
  bool direction, stepIsHigh;
  unsigned long N1, N2, ti, tiLow, sti, current, steps, stepsPerCircle, tDelay;
  float koeff,speed,speedUp;
  StateRotable state; 
}rotable;



//const byte LED = A2;

const byte STEPPIN = 14; //STEP 
const byte DIRPIN  = 15; //DIRECTION
const byte ENBPIN = 13; //ENABLE

//

const byte BUTTON = 4; //pin for button
const byte LED = 2; //pin for button

WiFiClient client;
WiFiServer server(80);
bool WiFiStationMode;

//const char* ssid = "TP-LINK";
//const char* password = "aere2014";

const char* ssidAP = "ESP Core WiFi Station";

void WiFiSetup(bool station){
  const String path = "/core/auth_data.json";
  if(!SPIFFS.exists(path)){
    File auth = SPIFFS.open(path, "w+");
    auth.print("{\"ssid\":\"Null\",\"pass\":\"Null\"}");
    auth.close();
  }
  if(station){
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssidAP);
    IPAddress hostAP(192,168,1,1);
    WiFi.softAPConfig(hostAP, hostAP, IPAddress(255,255,255,0));
  
    server.begin();
    WiFiStationMode = true;
    }
  else{
    WiFi.mode(WIFI_STA);

    
    File auth = SPIFFS.open(path, "r");
    auth.seek(9,SeekSet);
    String ssid = auth.readStringUntil('"');
    auth.seek(9,SeekCur);
    String pass = auth.readStringUntil('"');
    auth.close();

    IPAddress ip(192,168,1,110);
    WiFi.config(ip, ip, IPAddress(255,255,255,0));
    while ( WiFi.status() != WL_CONNECTED){
      if (pass == "Null"){
          WiFi.begin(ssid.c_str());
      }
      else{
          Serial.println(ssid+' '+pass);
          WiFi.begin(ssid.c_str(), pass.c_str());
      }


      for(byte i = 0; i < 50;i++){        
        digitalWrite(LED,LOW);
        delay(100);
        digitalWrite(LED,HIGH);
        delay(100);
      }
    }
        
    server.begin();  
    WiFiStationMode = false;
  }
}

bool ButtonPressed(){
  delay(200);
  if (digitalRead(BUTTON) != 0)
    return false;
    
  delay(200);
  digitalWrite(LED,LOW);
  delay(200);  
  digitalWrite(LED,HIGH);
  delay(1600);
    
  if (digitalRead(BUTTON) != 0)
    return false;
    
  digitalWrite(LED,LOW);
  delay(200);  
  digitalWrite(LED,HIGH);
  delay(200);
  digitalWrite(LED,LOW);
  delay(200);  
  digitalWrite(LED,HIGH);
  delay(200);
  digitalWrite(LED,LOW);
  delay(200);  
  digitalWrite(LED,HIGH);

  return true;   
}


void setup() {  
  pinMode(LED,OUTPUT);
  pinMode(BUTTON,INPUT);  
  pinMode(STEPPIN, OUTPUT); 
  pinMode(DIRPIN, OUTPUT);
  pinMode(ENBPIN, OUTPUT);

  DeviceInit();
  
  digitalWrite(LED,HIGH);
  digitalWrite(ENBPIN, LOW);
  digitalWrite(STEPPIN, LOW); 
  digitalWrite(DIRPIN, LOW);

  
  SPIFFS.begin();

  Serial.begin(9600);
  Dir dir = SPIFFS.openDir("/core");
  Serial.println("/core");
  while (dir.next()) {
    Serial.print(dir.fileName());
    File f = dir.openFile("r");
    Serial.println(f.size());
  }
  dir = SPIFFS.openDir("/errors");
  Serial.println("/errors");
  while (dir.next()) {
    Serial.print(dir.fileName());
    File f = dir.openFile("r");
    Serial.println(f.size());
  }  
  dir = SPIFFS.openDir("/cw");
  Serial.println("/cw");
  while (dir.next()) {
    Serial.print(dir.fileName());
    File f = dir.openFile("r");
    Serial.println(f.size());
  }  
  dir = SPIFFS.openDir("/w");
  Serial.println("/w");
  while (dir.next()) {
    Serial.print(dir.fileName());
    File f = dir.openFile("r");
    Serial.println(f.size());
  }
  
  delay(100);
  WiFiSetup(ButtonPressed());
  digitalWrite(LED,LOW);
  }

String getHeader(int responseCode,String contentType,String contentSize)
{
  String res = "";
  if (responseCode == 200)
  res += "HTTP/1.1 200 OK";
  else if (responseCode == 404)
        res += "HTTP/1.1 404 Page not found";
  else
    res += "HTTP/1.1 " + String(responseCode) + " Invalid request";

  res += "\r\nContent-Type: " + contentType + ";charset=utf-8";
  if (contentSize != "")
    res += "\r\nContent-Length: " + contentSize;  

  res += "\r\n\r\n";
  
  return res;
}

void parseWorkCommand(String firstLine)
{
    digitalWrite(LED,HIGH);
    
    String header = "";
    String response = "";

    //firstLine.toUpperCase();
    firstLine.replace("HTTP/1.1","");


    bool fileNotFound = true;
    if (firstLine.startsWith("GET /"))
    {
       String path = firstLine.substring(5);
       path.trim();
       if (path == "")
         path = "index.html";
       if (WiFiStationMode){        
         path = "/cw/"+path;
       }
       else{
         path = "/w/"+path;
       }

        path.toLowerCase();
        
       if (SPIFFS.exists(path))
       {
 
         File f = SPIFFS.open(path, "r");
         response = f.readString();
       
        String contentType = "text/html";
        if (path.endsWith(".js"))
          contentType = "application/javascript";
        else if (path.endsWith(".css"))
          contentType = "text/css";

       String fileSize = String(f.size());
       header = getHeader(200,contentType, fileSize);     
           f.close();
       fileNotFound = false;
       }
    }
    if (firstLine.startsWith("PUT /"))
    {
       String path = firstLine.substring(5);
       path.trim();
//       if (!SPIFFS.exists("auth_data.json"))
//        return;
            
        File auth = SPIFFS.open("/core/auth_data.json", "r");
        auth.seek(8,SeekSet);
        String ssid = auth.readStringUntil('"');
        auth.seek(10,SeekCur);
        String pass = auth.readStringUntil('"');
        auth.close();
    
        if (path.substring(0,22) == "api/device/ssid?value=")
            ssid = path.substring(22); 
        if (path.substring(0,22) == "api/device/pass?value=")
            pass = path.substring(22); 
            
        auth = SPIFFS.open("/core/auth_data.json", "w+");
        auth.print("{\"ssid\":\""+ssid+"\",\"pass\":\""+pass+"\"}");
        
        header = getHeader(200,"text/html", "");
        response = "{\"ssid\":\""+ssid+"\",\"pass\":\""+pass+"\"}\n\r"+String(SPIFFS.exists("auth_data.json"));
        fileNotFound = false;
        auth.close();
    }

    if (fileNotFound)
    {
      String path = "/errors/fileNotFound.html";

       if (SPIFFS.exists(path))
       { 
         File f = SPIFFS.open(path, "r");
         response = f.readString();
         f.close();  
         header = getHeader(404,"text/html","");
       }
       else{
         header = getHeader(404,"text/html",""); 
       }
    
    }

    client.println(header);    
    client.println(response);
    client.flush();
    
  
  digitalWrite(LED,LOW);

}



void loop() {  
  client = server.available();
 
  if (!client) {
    return;
  }
  while(!client.available()){
    delay(1);
  }
  String firstLine = "";

  while ( client.available()) {
    
    char c = client.read();
    if (c == '\n')
      parseWorkCommand(firstLine);
    else 
      firstLine += c;
  }
}

