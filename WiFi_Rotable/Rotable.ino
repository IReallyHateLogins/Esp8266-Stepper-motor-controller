void RotableInit(){  
  rotable.state = Ready;
  rotable.mode = None;
  rotable.steps = 0;
  rotable.N1 = 0;
  rotable.N2 = 0;
}


void RotableSET(){
  File f = SPIFFS.open("/api/rotable.json", "w+");
  if(rotable.speed < 1 || rotable.speed > 7000)
    rotable.speed = 7000;
  f.print("{\"speed\":"+String(rotable.speed));
  if(rotable.speedUp < 1 || rotable.speedUp > 7000)
    rotable.speedUp = 7000;
  f.print(",\"speedup\":"+String(rotable.speedUp));
  if(rotable.direction != 1 && rotable.direction != 0)
    rotable.direction = 0;
  f.print(",\"direction\":"+String(rotable.direction));
  f.print(",\"stepspercircle\":"+String(rotable.stepsPerCircle));
  f.print(",\"version\":"+String(rotable.version));
  f.print(",\"state\":"+String(rotable.state));
  f.print("\"commands\":[\"start\",\"step\",\"stop\",\"forcestop\",\"on\",\"off\"]}");
  f.close();    
}

void RotableGET(){
  File f = SPIFFS.open("/api/rotable.json", "r");
  String value;
  
  f.find(':');
  value = f.readStringUntil(',');
  rotable.speed = value.toInt();
  
  f.find(':');
  value = f.readStringUntil(',');
  rotable.speedUp = value.toInt();
  
  f.find(':');
  value = f.readStringUntil(',');
  rotable.direction = value.toInt();
  
  f.find(':');
  value = f.readStringUntil(',');
  rotable.stepsPerCircle = value.toInt();

  f.close();
}

String RotableStep(String value){
  long step = value.toInt();
  bool dirFlag = false;
  if (step < 0){
    dirFlag = true;
    step *= -1;
  }
  if (rotable.state != Ready){
    if (rotable.state == Off)
      return "Engine is off";
    else
      return "Device busy";
  }
  rotable.steps = step;
  
  if (rotable.steps <= 0 || rotable.speed <= 0 || rotable.speedUp <= 0)
    return "Error";
  device.dRes = "OK Step ";
  device.dRes += value;
  
  digitalWrite(DIRPIN, (rotable.direction ^ dirFlag)); 
  
  switch (rotable.steps) { //Проверка на кол-во шагов в очереди
    case 1:
    digitalWrite(STEPPIN, HIGH);
    digitalWrite(STEPPIN, LOW);
    return device.dRes;
    break;
    default: //если больше 1, запускаем вращение с ускорением
    stepperInit(rotable.speedUp, rotable.speed, rotable.steps); //Инициируем работу двигателя
    return "Delayed";
  };

  
}

String RotableOn(){
  if (rotable.state != Off){
    if (rotable.state == Ready)
      return "Engine is on";
    else
      return "Device busy. State = " + String(rotable.state) +" Mode = " + String(rotable.mode);
  }  
  rotable.state = Ready;
  digitalWrite(ENBPIN, LOW);
  return "OK On";
}

String RotableOff(){
   if (rotable.state != Ready){
    if (rotable.state == Off)
      return "Engine is off";
    else
      return "Device busy";
  }
  rotable.state = Off; 
  digitalWrite(ENBPIN, HIGH);
  return "OK Off";
}


String RotableStop(){

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
            return "Already stopped";
          if (rotable.state == Finishing)
            return "Already stopping";     
        }
    }
    device.dRes = "OK Stop";   
    return "Delayed";
}

bool RotableForceStop(){
  if (rotable.mode != None){    
    rotable.state = Ready;
    rotable.mode = None;
    return "Ok ForceStop";
  }
  return "Already stopped";
}


 String RotableStart(String value){
  bool hasPar = true;
  if (value == "") 
    hasPar = false;
   if (rotable.state != Ready){
    if (rotable.state == Off)
      return "Engine is off";
    else
      return "Device busy. State = " + String(rotable.state) +" Mode = " + String(rotable.mode);
  }  
  digitalWrite(DIRPIN, rotable.direction); 
  if(hasPar){
    if(value.toInt() <= 0)
        return "Error";
    rotable.steps = rotable.stepsPerCircle * value.toInt();
    stepperInit(rotable.speedUp, rotable.speed, rotable.steps);    
    return "OK Start "+value;       
  }
  else{
    rotable.steps = 480000;
    stepperInit(rotable.speedUp, rotable.speed, rotable.steps); //Инициируем работу двигателя
    rotable.mode = Infinite;
    return "OK Start";
  }
 }

