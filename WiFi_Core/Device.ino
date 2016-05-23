void DeviceInit(){
  Reset();
  device.baudRate = BaudRead();
  device.mode = Normal;
  RotableInit();
  ClearInput();
  device.interrupt = false;
}

void Reset(){
  if (digitalRead(ResetPin))
    return; 

//  delay(200);
//  digitalWrite(LED,HIGH);
//  delay(200);  
//  digitalWrite(LED,LOW);
//  delay(1600);
    
  if (digitalRead(ResetPin))
    return;
  EEPROM.put(0,0x290f);

  EEPROM.put(10,(float)0);//speed
  EEPROM.put(20,(float)0);//speedUp
  EEPROM.put(30,(bool)0); //direction
  EEPROM.put(40,(long)0);//stepPerCircle
  //EEPROM.put(50,(long)0);//
  //EEPROM.put(60,(unsigned int)0);//shotDelay
      
//  digitalWrite(LED,HIGH);
//  delay(200);  
//  digitalWrite(LED,LOW);
//  delay(200);  
//  digitalWrite(LED,HIGH);
//  delay(200);  
//  digitalWrite(LED,LOW);
//  delay(200);
}

bool GET(){
  
  Answer("OK-7 GET");
  Answer("{");
  Answer("  \"Id\": \""+device.id+"\",");
  Answer("  \"Version\": \""+device.version+"\",");
  Answer("  \"BaudRate\": "+String(device.baudRate)+',');
  switch(device.mode){
    case Normal:{
      Answer("  \"Mode\": \"Normal\",");
      break;
    }
    case Echo:{
      Answer("  \"Mode\": \"Ehho\",");
      break;
    }
  }
  Answer("  \"Objects\": [\"Rotable\"],");
  Answer("}");
  return true;
}

bool DeviceBaud(){
  switch(parse.crc[2]){
    case 0x290f:{//9600
      EEPROM.put(0, 0x290f);
      Answer("OK BoudRate 9600");
      break;  
    }
    case 0x293a:{//19200 
      EEPROM.put(0, 0x293a);
      Answer("OK BoudRate 19200");
      break;  
    }
    case 0x2b3f:{//38400
      EEPROM.put(0, 0x2b3f);
      Answer("OK BoudRate 38400");
      break;  
    }
    case 0x2d34:{//57600
      EEPROM.put(0, 0x2d34);
      Answer("OK BoudRate 57600");
      break;  
    }
    case 0x3107:{//115200
      EEPROM.put(0, 0x3107);
      Answer("OK BoudRate 115200");
      break;  
    }
    default :{
      return false;
    }
  }
  device.baudRate = BaudRead();
  ComSerial.end();
  ComSerial.begin(device.baudRate);
  return true;
}

bool DeviceGET(){
  String answer = "";
  switch(parse.crc[1]){
    case 0x0823:{//Mode
      answer+="\"Mode\": ";
      switch (device.mode){
        case Normal:
          answer+="\"Normal\"";
          break;
        case Echo:
          answer+="\"Echo\"";
          break;
    }     
    Answer(answer);
      break;
    }
    case 0x3d2d:{//Id
      answer+="\"Id\": \"";
      answer+=device.id; 
      Answer(answer+'\"');
      break;
    }
    case 0x005a:{//Version
      answer+="\"Version\": \"";
      answer+=device.version;     
      Answer(answer+'\"');      
      break;
    }
    case 0x6f65:{//CommandList
      
    Answer("OK-2 GET CommandList");
    Answer("Normal[newAddress]");
    Answer("Echo[newAddress]");
      
      break;
    } 
    case 0x6710:{
      answer+="\"BaudRate\": ";
      answer+=device.baudRate;     
      Answer(answer);
      break;   
    }
    default:{
      return false;
    }
  }
  return true;
}

bool DeviceNormal(){
  String answer = "";
    device.mode = Normal;
    answer+="OK Normal";
    Answer(answer);
    return true;
}

bool DeviceEcho(){
  String answer = "";
    device.mode = Echo;    
    answer+="OK Echo";
    Answer(answer);
    return true;
}

unsigned long BaudRead(){
  int baud;
  EEPROM.get(0, baud);
  switch(baud){
    default:
    case 0x290f:{//9600
      return 9600;
      break;  
    }
    case 0x293a:{//19200 
      return 19200;
      break;  
    }
    case 0x2b3f:{//38400
      return 38400;
      break;  
    }
    case 0x2d34:{//57600
      return 57600;
      break;  
    }
    case 0x3107:{//115200
      return 115200;
      break;  
   }
  }
}

