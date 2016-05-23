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



void DeviceInit(){
  device.mode = Normal;
  device.dRes = "";
  RotableInit();
}


void DeviceGET(){
  //nothing yet
}

String DeviceNormal(){
    device.mode = Normal;
    return "OK Normal";
}

String DeviceEcho(){
    device.mode = Echo;   
    return "OK Echo";
}
