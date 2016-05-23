void RotableInit(){  
  rotable.state = Ready;
  rotable.mode = None;
  rotable.steps = 0;
  rotable.N1 = 0;
  rotable.N2 = 0;

  EEPROM.get(10,rotable.speed);
  EEPROM.get(20,rotable.speedUp);
  EEPROM.get(30,rotable.direction); 
  EEPROM.get(40,rotable.stepsPerCircle);
}

bool GRotable(){
  Answer("OK-8 GET Rotable");
  Answer("{");
  Answer("  \"Version\": \""+device.version+"\",");
  switch(rotable.state){
    case Ready:{
      Answer("  \"State\": \"Ready\",");
      break;
    }
    case Starting:{
      Answer("  \"State\": \"Starting\",");
      break;
    }
    case Running:{
      Answer("  \"State\": \"Running\",");
      break;
    }
    case Finishing:{
      Answer("  \"State\": \"Finishing\",");
      break;
    }
    case Off:{
      Answer("  \"State\": \"Off\",");
      break;
    }
    case Error:{
      Answer("  \"State\": \"Error\",");
      break;
    }
  }
  Answer("  \"Speed\": "+String((long)rotable.speed)+',');
  Answer("  \"SpeedUp\": "+String((long)rotable.speedUp)+',');
  Answer("  \"Direction\": "+String(rotable.direction)+',');
  Answer("  \"StepsPerCircle\": "+String(rotable.stepsPerCircle)+',');
  Answer("}");
  return true;
}

bool RotableSET(){
  String answer = "";  
  if (rotable.state != Ready && rotable.state!= Off){
    Answer("Device busy");
    return true;
  }
  switch(parse.crc[2]){
  case 0x7a69:{//.Speed
        if(parse.input.toInt() <= 0)
          return false;
        if(parse.input.toInt() > 7000){
          Answer("Speed is limited to 7000");
          return true;
        }
        rotable.speed = parse.input.toInt();
        EEPROM.put(10,rotable.speed);
        answer+="OK Speed = ";
        answer+=String((long)rotable.speed);      
        Answer(answer);       
      break;
      }
      case 0x1e4c:{//.SpeedUp
        if(parse.input.toInt() <= 0)
          return false;
        if(parse.input.toInt() > 7000){
          Answer("SpeedUp is limited to 7000");
          return true;
        }
        rotable.speedUp = parse.input.toInt();
        EEPROM.put(20,rotable.speedUp);
        answer+="OK SpeedUp = ";
        answer+=String((long)rotable.speedUp);      
        Answer(answer);      
      break;
      }
      case 0x106b:{//.Direction
        switch(parse.input[0]){
          case '0':
            rotable.direction = false;
            break;
          case '1':
            rotable.direction = true;
            break;
          default:
            return false;
        }
        EEPROM.put(30,rotable.direction);
        answer+="OK Direction = ";
        answer+=String(rotable.direction);      
        Answer(answer);      
      break;
      }
      case 0x331a:{//.StepsPerCircle        
        if(parse.input.toInt() <= 0)
          return false;
        rotable.stepsPerCircle = parse.input.toInt();
        EEPROM.put(40,rotable.stepsPerCircle);
        answer+="OK StepsPerCircle = ";
        answer+=String(rotable.stepsPerCircle);      
        Answer(answer);
        break; 
      }
      default:{
        return false;
      }
  } 
  return true;    
}

bool RotableGET(){
  String answer = "";
  switch (parse.crc[2]){
  case 0x7a69:{//.Speed
      answer+="\"Speed\" : ";
      answer+=(long)rotable.speed;      
      Answer(answer);      
      break;
    }
    case 0x1e4c:{//.SpeedUp
      answer+="\"SpeedUp\" : ";
      answer+=(long)rotable.speedUp;      
      Answer(answer);         
    break;
    }
    case 0x106b:{//.Direction
      answer+="\"Direction\" : ";
      answer+=rotable.direction;      
      Answer(answer);         
    break;
    }
    case 0x7b79:{//.State
      answer+="\"State\": \"";
      switch (rotable.state){
        case Ready:
              answer += "Ready";
              break;
        case Starting:
              answer += "Starting";
              break;
        case Running:
              answer += "Running";
              break;
        case Finishing:
              answer += "Finishing";
              break;
        case Off:
              answer += "Off";
              break;
        case Error:
              answer += "Error";
              break;                
      }
      Answer(answer+'\"');         
    break;
    }
    case 0x0074:{//.Version
      answer+="\"Version\" : ";
      answer+=rotable.version;      
      Answer(answer);         
    break;
    }
    case 0x331a:{//.StepsPerCircle        
      answer+="\"StepsPerCircle\" : ";
      answer+=rotable.stepsPerCircle;      
      Answer(answer);   
      break; 
    }
    case 0x3a4b:{//.CommandList 
    
    Answer("OK-6 GET Rotable CommandList");
    Answer("Steps[stepCount]");
    Answer("Start[none|CircleCount]");
    Answer("Stop");
    Answer("ForceStop");
    Answer("On");
    Answer("Off");
             
    break;
    }
    default:{
      return false;
    }
  }
  return true;
}

bool RotableSteps(){  
  long steps = parse.input.toInt();
  bool dirFlag = false;
  if (steps < 0){
    dirFlag = true;
    steps *= -1;
  }
  if (rotable.state != Ready){
    if (rotable.state == Off)
      Answer("Engine is off");
    else
      Answer("Device busy");
    return true;
  }
  rotable.steps = steps;
  
  if (rotable.steps <= 0 || rotable.speed <= 0 || rotable.speedUp <= 0)
    return false;
  parse.answer = "OK Step ";
  parse.answer += parse.input;
  
  digitalWrite(DIRPIN, (rotable.direction ^ dirFlag)); 
  
  switch (rotable.steps) { //Проверка на кол-во шагов в очереди
    case 1:
    digitalWrite(STEPPIN, HIGH);
    digitalWrite(STEPPIN, LOW);
    Answer(parse.answer);
    break;
    default: //если больше 1, запускаем вращение с ускорением
    stepperInit(rotable.speedUp, rotable.speed, rotable.steps); //Инициируем работу двигателя
    break;
  };
  return true;
  
}

bool RotableOn(){
  if (rotable.state != Off){
    if (rotable.state == Ready)
      Answer("Engine is on");
    else
      Answer("Device busy. State = " + String(rotable.state) +" Mode = " + String(rotable.mode));
    return true;
  }  
  rotable.state = Ready;
  digitalWrite(ENBPIN, LOW);
  Answer("OK On");
  return true;
}

bool RotableOff(){
   if (rotable.state != Ready){
    if (rotable.state == Off)
      Answer("Engine is off");
    else
      Answer("Device busy");
    return true;
  }
  rotable.state = Off; 
  digitalWrite(ENBPIN, HIGH);
  Answer("OK Off");
  return true;
}


bool RotableStop(){

    if (rotable.state == Starting){
      rotable.current = rotable.steps - rotable.current;
      rotable.state = Finishing;     
    }
    else{
      if (rotable.state == Running){
        rotable.current = rotable.N2;
        rotable.state = Finishing;
        }
        else{
          if (rotable.state == Ready)
            Answer("Already stopped");
          if (rotable.state == Finishing)
            Answer("Already stopping");     
        }
    }
    parse.answer = "OK Stop";   
    return true;
}

bool RotableForceStop(){
  if (rotable.mode != None){    
    rotable.state = Ready;
    rotable.mode = None;
    Answer("Ok ForceStop");
  }
  else{
    Answer("Already stopped");
  }
  return true;
}


 bool RotableStart(bool hasPar){
   if (rotable.state != Ready){
    if (rotable.state == Off)
      Answer("Engine is off");
    else
      Answer("Device busy. State = " + String(rotable.state) +" Mode = " + String(rotable.mode));
    return true;
  }  
  digitalWrite(DIRPIN, rotable.direction); 
  if(hasPar){
    if(parse.input.toInt() <= 0)
        return false;
    rotable.steps = rotable.stepsPerCircle * parse.input.toInt();
    stepperInit(rotable.speedUp, rotable.speed, rotable.steps);    
    Answer("OK Start "+String(parse.input.toInt()));       
  }
  else{
    rotable.steps = 480000;
    stepperInit(rotable.speedUp, rotable.speed, rotable.steps); //Инициируем работу двигателя
    rotable.mode = Infinite;
    Answer("OK Start");
  }
  return true;
 }

