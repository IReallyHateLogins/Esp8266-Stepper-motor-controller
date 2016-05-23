void WiFiSetup(bool station){
  const String path = "/api/auth_data.json";
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

void parseWorkCommand(String firstLine){
    digitalWrite(LED,HIGH);
    
    String header = "";
    String response = "";

    //firstLine.toUpperCase();
    firstLine.replace("HTTP/1.1","");


    bool fileNotFound = true;
    if (firstLine.startsWith("GET /"))
    {
       String path = firstLine.substring(4);
       path.toLowerCase();
       path.trim();
        
       if (path == "")
         path = "index.html";
       if (SPIFFS.exists(path))
       {
 
         File f = SPIFFS.open(path, "r");
         response = f.readString();
       
        String contentType = "text/html";
        if (path.endsWith(".js"))
          contentType = "application/javascript";
        if (path.endsWith(".json"))
          contentType = "application/json";
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
       String path = firstLine.substring(4,firstLine.indexOf('?'));
       String values = firstLine.substring(firstLine.indexOf('?')+1);

        if(path == "/api/auth") {
          File auth = SPIFFS.open("/api/auth_data.json", "r");
          auth.seek(8,SeekSet);
          String ssid = auth.readStringUntil('"');
          auth.seek(10,SeekCur);
          String pass = auth.readStringUntil('"');
          auth.close();
      
          do
          {
            if(path.substring(0,4) == "ssid")
              ssid = values.substring(6,values.indexOf('&'));
            else              
              if(path.substring(0,4) == "pass")            
                pass = values.substring(6,values.indexOf('&'));
            values = values.substring(values.indexOf('&')+1);         
          }
          while (values.indexOf('&') != -1);
              
          auth = SPIFFS.open("/api/auth_data.json", "w+");
          auth.print("{\"ssid\":\""+ssid+"\",\"pass\":\""+pass+"\"}");
          
          header = getHeader(200,"text/html", "");
          response = "{\"ssid\":\""+ssid+"\",\"pass\":\""+pass+"\"}\n\r"+String(SPIFFS.exists("auth_data.json"));
          fileNotFound = false;
          auth.close();
        }
        if(path == "/api/device?") {
          //nothing yet
        }
        if(path == "/api/rotable?") {
      
          do
          {
            if(path.substring(5) == "speed")
              rotable.speed = values.substring(7,values.indexOf('&')-1).toInt();
            else              
              if(path.substring(0,7) == "speedup")            
                 rotable.speedUp = values.substring(9,values.indexOf('&')-1).toInt();
              else              
                if(path.substring(0,9) == "direction")            
                   rotable.direction = values.substring(11,values.indexOf('&')-1).toInt();
                else              
                  if(path.substring(0,14) == "stepspercircle")            
                     rotable.stepsPerCircle = values.substring(16,values.indexOf('&')-1).toInt();
            values = values.substring(values.indexOf('&')+1);
          }
          while (values.indexOf('&') == -1);
              
          RotableSET();
          fileNotFound = false;
        }
    }
    
    if (firstLine.startsWith("POST /")){
      String path;
      String values;  
      if(firstLine.indexOf('?') == -1){
        path = firstLine.substring(5);
        values = "";  
      }
      else{
        path = firstLine.substring(5,firstLine.indexOf('?'));
        values = firstLine.substring(firstLine.indexOf('?')+1);
      }
     if(path.startsWith("/api/rotable/")) {
      path = path.substring(14);
      fileNotFound = false;
      if (path == "on")
        response = RotableOn();
      else
        if (path == "off")
          response = RotableOff();
        else
          if (path == "stop")
            response = RotableStop();
          else
          if (path == "forcestop")
            response = RotableForceStop();
          else
            if (path == "step")
              response = RotableStep(values.substring(values.indexOf('=')+1));
            else
            if (path == "start")
                response = RotableStart(values.substring(values.indexOf('=')+1));
            else
                fileNotFound = true;
     }
      
    }

    if (fileNotFound)
    {
      String path = "/errors/fileNotFound.html";

       if (SPIFFS.exists(path))
       { 
         File f = SPIFFS.open(path, "r");
         response = f.readString()+'\n'+firstLine.substring(4,firstLine.indexOf('?'));
         f.close();  
         header = getHeader(404,"text/html","");
       }
       else{
         header = getHeader(404,"text/html",""); 
       }
    
    }

    if(response != "Delayed"){
    client.println(header);    
    client.println(response);
    client.flush();
    }
    
  
  digitalWrite(LED,LOW);

}

void DelayedResponse(){
  if (device.dRes == "")
    return;
  client.println(getHeader(200,"text/html",""));    
  client.println(device.dRes);
  device.dRes = "";
}



