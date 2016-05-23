//INIT
void stepperInit(float _A, float _Vmax, long _Nfull) {
  rotable.current = 0; //Сбрасываем счетчик текущего положения
  rotable.N1 = (_Vmax * _Vmax) / (2 * _A);
  
  rotable.tDelay = 1000000/rotable.speed;
  rotable.koeff = sqrt((2 * 1000000) / rotable.speedUp);
  rotable.sti = micros();
  
  rotable.state = Starting;
  if (rotable.N1 < (_Nfull / 2)) { //Если успеваем разогнаться до максимальной скорости
    rotable.N2 = _Nfull - rotable.N1;
    rotable.mode = Long;
  }
  else { //Если замедляемся раньше достижения макс. скорости
    rotable.N1 = _Nfull/2;
    rotable.N2 = rotable.N1;
    rotable.mode = Fast;
  }
}

//RUN
void StepperRun() { 
  unsigned long dt;
  static unsigned long _t2;
  static unsigned long tCurr;
  tCurr = micros(); //Фиксируем текущее время

  switch(rotable.mode) {
    //######################################################### РЕЖИМ БЕЗ ДОСТИЖЕНИЕМ МАКСИМАЛЬНОЙ СКОРОСТИ #######################################################
    case Fast:
     rotable.current += 1;
       digitalWrite(STEPPIN, HIGH);
       switch (rotable.state) { //Смотрим на каком участке кривой скорости находимся
            case Starting: //Если идет разгон
              if (rotable.current >= rotable.N1) rotable.state = Finishing;
              break;
            case Finishing: //Если идет торможение
              if (rotable.current == rotable.steps) {
                rotable.state = Ready;
                rotable.mode = None;
                Answer(parse.answer);
              }
              break;
          }
      switch (rotable.state) { //Рассчет времени
        case Starting: //Если идет разгон
          dt = ((rotable.koeff * (sqrt(rotable.current + 1) - sqrt(rotable.current))) * 1000);
          rotable.ti = dt + tCurr;  
          rotable.tiLow = dt/2 + tCurr;           
          break;
        case Finishing: //Если идет торможение
          dt = ((rotable.koeff * (sqrt((rotable.steps - rotable.current) + 1) - sqrt((rotable.steps - rotable.current)))) * 1000);
          rotable.ti = dt + tCurr;
          rotable.tiLow = dt/2 + tCurr;
          break;
      }
      rotable.stepIsHigh = HIGH;
      break;  

    
    //######################################################### РЕЖИМ С ДОСТИЖЕНИЕМ МАКСИМАЛЬНОЙ СКОРОСТИ #######################################################
    case Long:
       rotable.current += 1; 
       digitalWrite(STEPPIN, HIGH);
       switch (rotable.state) { //Смотрим на каком участке кривой скорости находимся
            case Starting: //Если идет разгон
              if (rotable.current >= rotable.N1) rotable.state = Running;
              break;
            case Running: //Если двигаемся с одинаковой скоростью
              if (rotable.current >= rotable.N2) rotable.state = Finishing;
              //_t2 = micros();
              break;
            case Finishing: //Если идет торможение
              if (rotable.current == rotable.steps) {
                rotable.state = Ready;
                rotable.mode = None;
                Answer(parse.answer);
              }
              break;
          }
      switch (rotable.state) { //Рассчет времени
        case Starting: //Если идет разгон
          dt = ((rotable.koeff * (sqrt(rotable.current + 1) - sqrt(rotable.current))) * 1000);
          rotable.ti = dt + tCurr;  
          rotable.tiLow = dt/2 + tCurr;           
          break;
        case Running: //Если двигаемся с одинаковой скоростью       
          rotable.ti = rotable.tDelay + tCurr;
          break;
        case Finishing: //Если идет торможение
          dt = ((rotable.koeff * (sqrt((rotable.steps - rotable.current) + 1) - sqrt((rotable.steps - rotable.current)))) * 1000);
          rotable.ti = dt + tCurr;
          rotable.tiLow = dt/2 + tCurr;
          break;
      }         
      rotable.stepIsHigh = HIGH;
      break;

       
      //For Start w/o parameters
      case Infinite:
       rotable.current += 1; 
       digitalWrite(STEPPIN, HIGH);
       switch (rotable.state) { //Смотрим на каком участке кривой скорости находимся
            case Starting: //Если идет разгон
              if (rotable.current >= rotable.N1) rotable.state = Running;
              break;
            case Running: //Если двигаемся с одинаковой скоростью
              if (rotable.current >= rotable.N2) rotable.state = Finishing;
              rotable.current -= 1;
              break;
            case Finishing: //Если идет торможение
              if (rotable.current == rotable.steps) {
                rotable.state = Ready;
                rotable.mode = None;
                Answer(parse.answer);
              }
              break;
          }
      switch (rotable.state) { //Рассчет времени
        case Starting: //Если идет разгон
          dt = ((rotable.koeff * (sqrt(rotable.current + 1) - sqrt(rotable.current))) * 1000);
          rotable.ti = dt + tCurr;   
          rotable.tiLow = dt/2 + tCurr;          
          break;
        case Running: //Если двигаемся с одинаковой скоростью       
          rotable.ti = rotable.tDelay + tCurr;
          rotable.tiLow = rotable.tDelay/2 + tCurr;
          break;
        case Finishing: //Если идет торможение
          dt = ((rotable.koeff * (sqrt((rotable.steps - rotable.current) + 1) - sqrt((rotable.steps - rotable.current)))) * 1000);
          rotable.ti = dt + tCurr;
          rotable.tiLow = dt/2 + tCurr;
          break;
      }  
      rotable.stepIsHigh = HIGH;
      break;
      //
      case None:
      if (rotable.state != Ready)
        Answer("Err 0x44 Init Err");
    }
}

void ToStep(){
  if (rotable.stepIsHigh){
   if(micros() >= rotable.tiLow) {
    rotable.stepIsHigh = LOW;
    digitalWrite(STEPPIN, LOW);    
   }
  }
  else{
  if(micros() >= rotable.ti) StepperRun(); //Если пришло время сделать следующий шаг
  }
}
