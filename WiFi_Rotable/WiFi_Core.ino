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
    
    if (firstLine.startsWith("POST /")){
      
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



