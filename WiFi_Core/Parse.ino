void ReadIncoming(){
  char incomingChar;
  if (ComSerial.available() > 0){
    device.interrupt = true;
    incomingChar = ComSerial.read();
    if(incomingChar == '\n')
      return;
    if (incomingChar == '\r' )
    {
        NextInput(true);
        if (device.mode == Echo && parse.crc[0] != 0x1233){          
          String str = String(parse.count)+' ';
          for (byte i = 0; i<parse.count; i++)
            str += String(parse.crc[i],HEX)+' ';
          Answer(str);
        }
        else{
          ParseCommand();
        } 
        ClearInput();
    }
    else{        
      if (incomingChar != ' '){
        if (incomingChar == '.')
            NextInput(false);   
        parse.crc[parse.count] ^= incomingChar;
        parse.input+=incomingChar;          
      }
      else{
        if (parse.count < 6)
            NextInput(false);
      }              
     }
    device.interrupt = false;
  }
}

void ClearInput(){
  parse.input = "";
  parse.prev_input = "";
  parse.crc[0]=0;
  parse.crc[1]=0;
  parse.crc[2]=0;  
  parse.crc[3]=0;
  parse.crc[4]=0;
  parse.crc[5]=0;
  parse.count = 0; 
}

void NextInput(bool last){
  parse.crc[parse.count] ^= (unsigned int)((parse.input[0] ^ parse.input[parse.input.length()-1] ^ ((parse.input.length())<<3))<<8);  
  parse.count++;
  if (last)
    return;
  parse.prev_input = parse.input;
  parse.input = "";
}


void ParseCommand(){
  parse.error = "Err";
  bool done = false;
  switch (parse.count){
    case 1:{
      switch (parse.crc[0]){
        case 0x0b56:{//GET
          done = GET();
          break;
        }
        case 0x0a21:{//Echo
            done = DeviceEcho();
          break;      
        }
        case 0x1233:{//Normal
            done = DeviceNormal();
          break;      
        }
      }
      break;
      
    }
    case 2:{  
      switch (parse.crc[0]){
        case 0x0b56:{//GET(device)
            switch(parse.crc[1]){
              case 0x0f43:{//Rotable
                done = GRotable();
                break;
              }
              default:{
              done = DeviceGET();
              break;
              }
            }
          break;      
        }
        case 0x0f43:{//Rotable
          switch(parse.crc[1]){
            case 0x580f:{
              done = RotableOn();
              break;
            }
            case 0x6861:{                
              done = RotableOff();
              break;
            }
            case 0x7616:{//.Stop
              done = RotableStop();
              break;
            }
            case 0x0e4b:{//.ForceStop
              done = RotableForceStop();
              break;
            }
            case 0x6a6e:{//Start
              done = RotableStart(false);
              break;
            }
          }
          break;
        }        
      }
      break;
    }
    case 3:{  
      switch (parse.crc[0]){
        case 0x0b56:{//GET(objects)
          if(parse.crc[1] == 0x0f43)//Rotable
              done = RotableGET();
          break;  
        }
          case 0x0f43:{//Rotable
            switch(parse.crc[1]){
              case 0x761c:{
                done = RotableSteps();
                break;
              }
              case 0x6a6e:{
                done = RotableStart(true);
                break;
              }
            }             
            break;           
          }
          case 0x1f42:{
            if(parse.crc[1] == 0x6710)
              DeviceBaud();
            break;      
          }
        }
        break;
      }        
    case 4:{  
      switch (parse.crc[0])
        case 0x1f42:{//SET
          if(parse.crc[1] == 0x0f43)//Rotable
              done = RotableSET();
      }
      break;
    }
  }
  if (!done) Answer(parse.error);
}



void Answer(String output){
  if (ComSerial.availableForWrite())
  {
  digitalWrite(RS485_Dir,HIGH);
  unsigned int l = output.length();
  
  for(int i = 0; i < l; i++)
    ComSerial.write(output[i]);
  ComSerial.write(0x0A);
  ComSerial.write(0x0D);
  
  ComSerial.flush();
  
  //убираем за собой мусор
  while(ComSerial.available())
    byte t = ComSerial.read();
  digitalWrite(RS485_Dir,LOW);
  }
}
