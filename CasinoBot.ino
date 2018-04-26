#include <Servo.h>

#define p 6 //Number of pins in use
#define sleepTime 60000 //Milliseconds before sleeping

const int l = 12;//Number of roulette leds
const int pins[] = {12, 11, 10, 9, 8, 7};
const int leds[][2] = {{7,8}, {8,7}, {7,9}, {9,7}, {7,10}, {10,7}, {7,11}, {11,7}, {7,12}, {12,7}, {8,9}, {9,8}, {8,10}, {10,8}, {8,11}, {11,8}, {8,12}, {12,8}, {9,10}, {10,9}, {9,11}, {11,9}, {9,12}, {12,9}, {10,11}, {11,10}, {10,12}, {12,10}, {11,12}, {12,11}};
//Numbers is an array describing which led numbers will be turned on (reference diagram needed):
//Ok so this is really complicated but basically its the 8bit numbers are the DDRB and PORTB bytes needed to turn on the numbers on the betting board
const byte numbers[][14][2] = {{{B00000101, B00000001},{B00000101, B00000100},{B00001001, B00000001},{B00001001, B00001000},{B00010001, B00000001},{B00010001, B00010000},{B00001010, B00000010},{B00001010, B00001000},{B00010010, B00000010},{B00010010, B00010000},{B00001100, B00000100},{B00001100, B00001000}},
                          {{B00001001, B00000001},{B00001001, B00001000},{B00001010, B00000010},{B00001010, B00001000}},
                          {{B00000101, B00000001},{B00000101, B00000100},{B00001001, B00000001},{B00001001, B00001000},{B00000110, B00000010},{B00000110, B00000100},{B00010010, B00000010},{B00010010, B00010000},{B00001100, B00000100},{B00001100, B00001000}},
                          {{B00000101, B00000001},{B00000101, B00000100},{B00001001, B00000001},{B00001001, B00001000},{B00000110, B00000010},{B00000110, B00000100},{B00001010, B00000010},{B00001010, B00001000},{B00001100, B00000100},{B00001100, B00001000}},
                          {{B00001001, B00000001},{B00001001, B00001000},{B00010001, B00000001},{B00010001, B00010000},{B00000110, B00000010},{B00000110, B00000100},{B00001010, B00000010},{B00001010, B00001000}},
                          {{B00000101, B00000001},{B00000101, B00000100},{B00010001, B00000001},{B00010001, B00010000},{B00000110, B00000010},{B00000110, B00000100},{B00001010, B00000010},{B00001010, B00001000},{B00001100, B00000100},{B00001100, B00001000}},
                          {{B00000101, B00000001},{B00000101, B00000100},{B00010001, B00000001},{B00010001, B00010000},{B00000110, B00000010},{B00000110, B00000100},{B00001010, B00000010},{B00001010, B00001000},{B00010010, B00000010},{B00010010, B00010000},{B00001100, B00000100},{B00001100, B00001000}},
                          {{B00000101, B00000001},{B00000101, B00000100},{B00001001, B00000001},{B00001001, B00001000},{B00001010, B00000010},{B00001010, B00001000}},
                          {{B00000101, B00000001},{B00000101, B00000100},{B00001001, B00000001},{B00001001, B00001000},{B00010001, B00000001},{B00010001, B00010000},{B00000110, B00000010},{B00000110, B00000100},{B00001010, B00000010},{B00001010, B00001000},{B00010010, B00000010},{B00010010, B00010000},{B00001100, B00000100},{B00001100, B00001000}},
                          {{B00000101, B00000001},{B00000101, B00000100},{B00001001, B00000001},{B00001001, B00001000},{B00010001, B00000001},{B00010001, B00010000},{B00000110, B00000010},{B00000110, B00000100},{B00001010, B00000010},{B00001010, B00001000}}};
const int lengths[] = {12, 4, 10, 10, 8, 10, 12, 6, 14, 10};//These are the lengths of the above number arrays -- will use to avoid mem errors
int activeNum; //This will be used only in multi to display numbers, will range 0-9 inclusive, -1 if not in use
const int sPin = 2, rPin = 4, lPin = 3, cPin = 6;//S = start, l/r = left/right, c = chocolate
int onPin;
int onOffState;
unsigned long buttonPress;
Servo betServo;
Servo rWheel0; 
Servo rWheel1;
Servo rWheel2;
Servo cWheel;
int p1, p2;//Player 1+2's number choices in the betting game
bool locked;//true = locked, false = unlocked

void setup() {
  //Initialize timer overflow interrupts:
  cli();
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;
  bitSet(TCCR2B, CS10);
  bitSet(TCCR2B, CS12);//CS10 + CS12 initialize a 1024 prescaler
  bitSet(TIMSK2, TOIE1);//Allows for timer overflows to be interrupted
  sei();
  //Regular setup
  pinMode(sPin, INPUT);
  pinMode(lPin, INPUT);
  pinMode(rPin, INPUT);
  pinMode(cPin, INPUT);
  onPin = 26;
  onOffState = false;
  activeNum = -1;
  p1 = -1;
  p2 = -1;
  pinMode(A4, OUTPUT);
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  betServo.attach(A4);
  betServo.writeMicroseconds(1500);
  rWheel0.attach(A0);
  rWheel0.writeMicroseconds(1550);
  rWheel1.attach(A1);
  rWheel1.writeMicroseconds(1500);
  rWheel2.attach(A2);
  rWheel2.writeMicroseconds(1500);
  cWheel.attach(A3);
  cWheel.writeMicroseconds(2000); //2000 = locked, 1000 = unlocked
  locked = true;
  randomSeed(analogRead(A5));
  Serial.begin(9600);
}

void loop() {//This can be repurposed to roulette easily
  // This code will be game selection:
  if(digitalRead(cPin) == HIGH){
    if(locked == true){
      cWheel.writeMicroseconds(1000);
      locked = false;
    }
    else if(locked == false){
      cWheel.writeMicroseconds(2000);
      locked = true;
    }
    delay(200);
  }
  if (onOffState == false){
    if(digitalRead(sPin) == HIGH || digitalRead(rPin) == HIGH || digitalRead(lPin) == HIGH){
      onOffState = true;
      onPin = 26;
      shift(onPin);//This will hover roulette selection LED
      delay(200);
      betServo.writeMicroseconds(1500);//We only want it to reset on the next game for convenience
      buttonPress = millis();
    }
  }else{
    if(digitalRead(rPin) == HIGH){
      onPin++;
      if(onPin > 28) onPin = 26;
      shift(onPin);
      delay(200);
      buttonPress = millis();
    }
    if(digitalRead(lPin) == HIGH){
      onPin--;
      if(onPin<26) onPin = 28;
      shift(onPin);
      delay(200);
      buttonPress = millis();
    }
    if(digitalRead(sPin) == HIGH){
      gameStart(onPin);
    }
    if(millis() - buttonPress > sleepTime){
      lose();
    }
  }
}

void gameStart(int game){
  int wheels = 0;
  switch (game){
    case 26:
      onPin = 0;
      shift(onPin);
      break;
    case 28:
      rWheel0.writeMicroseconds(2000);
      rWheel1.writeMicroseconds(2000);
      rWheel2.writeMicroseconds(2000);
      deactivate();
      break;
    case 27:
      onPin = 29;
      for (int i = 0; i < p; i++){
        pinMode(pins[i], INPUT);
      }
      activeNum = 0;
      break;
  }
  delay(200);
  while(onOffState == true){
    if(game == 26){//Roulette
      if(digitalRead(sPin) == HIGH){
        if(spin(onPin) == true){
          for (int i = 0; i < 30; i++){
            onPin = random(0,12);
            shift(onPin);
            delay(50);
          }
          win(0);
        }
        else lose();
        delay(100);
      }
      else if(digitalRead(lPin) == HIGH){
        onPin--;
        if(onPin < 0)
          onPin = 11;
        shift(onPin);
        delay(200);
        buttonPress = millis();
      }
      else if(digitalRead(rPin) == HIGH){
        onPin = (onPin+1)%l;
        shift(onPin);
        delay(200);
        buttonPress = millis();
      }
      if(millis() - buttonPress > sleepTime){
        lose();
      }
    }
    else if(game == 28){//Slots
      if(digitalRead(sPin) == HIGH){
        switch(wheels){
          case 0:
            for(int i = 2000; i >= 1500; i--){
              rWheel2.writeMicroseconds(i);
              delay(2);
            }
            wheels++;
            break;
          case 1:
            for(int i = 2000; i >= 1500; i--){
              rWheel1.writeMicroseconds(i);
              delay(2);
            }
            wheels++;
            break;
          case 2:
            for(int i = 2000; i >= 1550; i--){
              rWheel0.writeMicroseconds(i);
              delay(2);
            }
            win(-1);
            break;
        }
        delay(200);
        buttonPress = millis();
      }
      if(millis() - buttonPress > sleepTime){
        lose();
      }
    }
    else if(game == 27){//Betting
      if(digitalRead(sPin) == HIGH){
        if(p1 == -1){
          p1 = activeNum;
          activeNum = (activeNum+1)%10;
        }
        else if(p2 == -1 && p1 != activeNum){
          p2 = activeNum;
          delay(50);
          activeNum = 3;
          Serial.println("3");
          delay(750);
          activeNum = 2;
          Serial.println("2");
          delay(750);
          activeNum = 1;
          Serial.println("1");
          delay(750);
          win(betting(p1, p2));
        }
        delay(200);
      }
      else if(digitalRead(lPin) == HIGH){
        activeNum--;
        if(activeNum < 0)
          activeNum = 9;
        delay(200);
        buttonPress = millis();
      }
      else if(digitalRead(rPin) == HIGH){
        activeNum = (activeNum+1)%10;
        delay(200);
        buttonPress = millis();
      }
      if(millis() - buttonPress > sleepTime){
        digitalWrite(leds[onPin], LOW);
        onOffState = false;
      }
    }
    else lose();
  }
  lose();
}

int betting(int num1, int num2){
  int t = random(75,150);
  for (int i = 0; i < t; i++){
    activeNum = random(0,10);
  }
  //Because the game will wrap around from 0-9 when considering who won (to avoid 5 being the best, or the one+opponent strategy) we need to have logic to decide winner
  delay(1000);
  if((num1 < activeNum && activeNum < num2) || (num1 > activeNum && num2 < activeNum)){//Result is between the bets
    if(abs(num1 - activeNum) < abs(num2 - activeNum)) return 1;
    else if(abs(num1 - activeNum) > abs(num2 - activeNum)) return 2;
    else return -1;//Draw
  }
  else if(num1 < activeNum && num2 < activeNum){
    if(num1 < num2){//(activeNum-10)<num1<num2<activeNum - means that num2 is 100% closer to result before considering wrap, so wrap is all needing consideration
      if((num1-(activeNum-10)) < activeNum - num2) return 1;
      else if((num1-(activeNum-10)) > activeNum - num2) return 2;
      else return -1;//Draw
    }
    else if(num2 < num1){
      if((num2-(activeNum-10)) < activeNum - num1) return 2;
      else if((num2-(activeNum-10)) > activeNum - num1) return 1;
      else return -1;//Draw 
    }
  }
  else if(num1 > activeNum && num2 > activeNum){
    if(num1 > num2){//(activeNum-10)<num1<num2<activeNum - means that num2 is 100% closer to result before considering wrap, so wrap is all needing consideration
      if(((activeNum+10))-num1 < num2 - activeNum) return 1;
      else if(((activeNum-10) - num1) > num2 - activeNum) return 2;
      else return -1;//Draw
    }
    else if(num2 > num1){
      if(((activeNum+10))-num2 < num1 - activeNum) return 2;
      else if(((activeNum+10) - num2) > num1 - activeNum) return 1;
      else return -1;//Draw 
    }
  }
  else if(num1 == activeNum) return 1;
  else if (num2 == activeNum) return 2;
  return -1;//Should never happen but ya know
}

bool spin(int led){
  int s = random(1000, 5000);//Time it will run
  int remainder;
  onPin = 0;//Start at beginning
  shift(onPin);
  for(int i = 0; i < s; i++){
    if(s - i < 500){//If its in the final seconds
      if(i%100 == 0){//Slow down the rate of changing lights -- adds suspense
        onPin = (onPin+1)%l;
        shift(onPin);
        remainder = s-i;//So we can smoothly transition from loop to loop below
      }
    }else{
      if(i%50 == 0){
        onPin = (onPin+1)%l;
        shift(onPin);
      }
    }
    delay(1);
  }
  int t = random(0, 5);
  remainder = map(remainder, 0, 99, 400, 0);//The bigger the time left, the less we need to delay
  delay(remainder);
  for(int i = 400; i <= 800 + 200*t; i+=200){
    onPin = (onPin+1)%l;
    shift(onPin);
    delay(i);
  }
  for(int i = 0; i < 7; i++){
    deactivate();
    delay(200);
    shift(onPin);
    delay(200);
  }
  return (led == onPin);
}

void lose(){
  onOffState = false;
  onPin = 26;
  p1 = -1;
  p2 = -1;
  activeNum = -1;
  deactivate();
  rWheel0.writeMicroseconds(1550);
  rWheel1.writeMicroseconds(1500);
  rWheel2.writeMicroseconds(1500);
}

void win(int motor){//Motor = 0 for roulette/slots win, = 1 for p1 betting win, = 2 for p2 betting win, -1 for betting draw
  if(motor != -1){
    activeNum = -1;
    if(motor == 1){
      Serial.println("p1 win");
      betServo.writeMicroseconds(2000);
    }
    else if(motor == 2){
      Serial.println("p2 win");
      betServo.writeMicroseconds(1000);
    }
    else if(motor == 0){
      cWheel.writeMicroseconds(1000);
    }
  }
  lose();//Its to reset but I think its kinda ironic
}

//Timer overflow interrupt
ISR(TIMER2_OVF_vect){
  TCNT2 = 254;
  setBits();
}

void deactivate(){
  for (int i = 0; i < p; i++){
    pinMode(pins[i], INPUT);
  }
}

void setBits(){
  if(activeNum != -1){
    for(int i = 0; i < lengths[activeNum]; i++){
      DDRB = numbers[activeNum][i][0];
      PORTB = numbers[activeNum][i][1];
    }
  }
}

void shift(int target){//Target is the led in 2D array leds[][] to be turned on
  for (int i = 0; i < p; i++){//Goes through pin list, turns appropriate one on
    if(pins[i] == leds[target][0]){
      pinMode(leds[target][0], OUTPUT);
      digitalWrite(leds[target][0], HIGH);
    }
    else if(pins[i] == leds[target][1]){
      pinMode(leds[target][1], OUTPUT);
      digitalWrite(leds[target][1], LOW);
    }
    else pinMode(pins[i], INPUT);
  }
}
