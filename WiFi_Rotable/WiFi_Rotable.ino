#include <ESP8266WiFi.h>
#include <FS.h>

void DeviceInit();
bool ButtonPressed();
void DelayedResponse();
void stepperInit(float, float , long);
void WiFiSetup(bool);
void InterruptSetup(unsigned long);
void parseWorkCommand(String);
void RotableInit();
void ToStep();
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

struct Device{
  Mode mode;
  const String id = "Prefpro Rotable v.0.5";
  const String version = "0.0.5.6";
  String dRes;
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
  Dir dir = SPIFFS.openDir("/api");
  Serial.println("/api");
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
  InterruptSetup(5);
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

