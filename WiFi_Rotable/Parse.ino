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
