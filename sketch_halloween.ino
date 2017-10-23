/*
  http://www.arduino.cc/en/Tutorial/BarGraph
  https://www.arduino.cc/en/Tutorial/Blink
  https://www.arduino.cc/en/Tutorial/Button
  http://bildr.org/2011/02/74hc595/
  http://les-schemas-electronique.blogspot.be/2016/06/afficheur-7-segments-4-digits-arduino.html
  http://thomas.bibby.ie/using-the-kyx-5461as-4-digit-7-segment-led-display-with-arduino/
*/


const int ledCount = 5;    // the number of LEDs in the bar graph
const int demoButton = 8;
const int recButton = 10; 
const int soundPin = 9; 
int ledPins[] = {
  2, 3, 4, 5, 6
};   // an array of pin numbers to which LEDs are attached

int notes[] = {
  523,554,587,622,659  
};

int digits[10][8] = {
  {1,1,1,1,0,0,1,1},
  {1,0,0,1,0,0,0,0},
  {1,1,1,0,0,1,0,1},
  {1,0,1,1,0,1,0,1},
  {1,0,0,1,0,1,1,0},
  {0,0,1,1,0,1,1,1},
  {1,1,1,1,0,1,0,1},
  {1,0,0,1,0,0,0,1},
  {1,1,1,1,0,1,1,1},
  {1,0,1,1,0,1,1,1} 
};


int pinAcc = 0;
int sensorAcc = 0;

int SER_Pin = 11; //pin 14 on the 75HC595
int RCLK_Pin = 12; //pin 12 on the 75HC595
int SRCLK_Pin = 13; //pin 11 on the 75HC595

//How many of the shift registers – change this
#define number_of_74hc595s 1

//do not touch
#define numOfRegisterPins number_of_74hc595s * 8

boolean registers[numOfRegisterPins];


void setup() {
  pinMode(demoButton, INPUT);
  randomSeed(analogRead(0));
  // loop over the pin array and set them all to output:
  for (int thisLed = 0; thisLed < ledCount; thisLed++) {
    pinMode(ledPins[thisLed], OUTPUT);
  }
  Serial.begin(9600);

  pinMode(SER_Pin, OUTPUT);
  pinMode(RCLK_Pin, OUTPUT);
  pinMode(SRCLK_Pin, OUTPUT);

  //reset all register pins
  clearRegisters();
  writeRegisters();
}

void loop() {
  if(digitalRead(demoButton) == HIGH){

    if((random(0,2) < 1) || pinAcc >= ledCount){
      pinAcc = pinAcc+1;
      tone(9, notes[min(pinAcc-1, (sizeof(notes)/sizeof(int)) -1)], 100);
    }
    lightLeds(pinAcc, pinAcc <= ledCount || pinAcc%2 != 0);
    delay(100);
  } else {
    pinAcc = 0;
    lightLeds(0,false);
    delay(20);
  }

  if(digitalRead(recButton) == HIGH){
    
    while(digitalRead(recButton) == HIGH){
      int sensorValue = analogRead(A0);
      if(sensorValue > sensorAcc){
        sensorAcc = sensorValue;
        lightLeds((int)(sensorValue / 1024.0*5),1);
      }
      Serial.println(sensorValue);
      delay(10);
    }

    recFeedback(sensorAcc);
    
  
  }

  if(digitalRead(recButton) == LOW && digitalRead(demoButton) == LOW){
    delay(100);
  }
}



void lightLeds(int untilPin, int blinkOverride){
  for (int thisLed = 0; thisLed < ledCount; thisLed++) {
    if (thisLed < untilPin && blinkOverride){
      digitalWrite(ledPins[thisLed], HIGH);
    }
    else {
      digitalWrite(ledPins[thisLed], LOW);
    }
  }
}



void recFeedback(int maxV){
  float sensor = 0.84;//sensorAcc/1024.0;
  int maxLed = (int) (sensor*5);
  int digit1 = (int)(sensor * 10) % 10;
  int digit2 = (int)(sensor * 100) % 10;

  for(int i = 0; i < 30; i++){
    Serial.println("digit1");
    Serial.println(digit1);
    Serial.println("digit2");
    Serial.println(digit2);
    lightLeds(min(maxLed,i), (i <= maxLed) || ((i%2) == 0));
    if((i%10) <4){
      writeDigit(digit1,false);
    } else if((i%10) < 8){
      writeDigit(digit2,true);
    } else {
      clearDigit();
    }
    delay(100);
  }
  clearDigit();
  sensorAcc=0;
}

//set all register pins to LOW
void clearRegisters(){
  for(int i = numOfRegisterPins - 1; i >= 0; i--){
    registers[i] = LOW;
  }
}

//Set and display registers
//Only call AFTER all values are set how you would like (slow otherwise)
void writeRegisters(){
  digitalWrite(RCLK_Pin, LOW);
  for(int i = numOfRegisterPins - 1; i >= 0; i--){
    digitalWrite(SRCLK_Pin, LOW);

    int val = registers[i];

    digitalWrite(SER_Pin, val);
    digitalWrite(SRCLK_Pin, HIGH);

  }
  digitalWrite(RCLK_Pin, HIGH);
}

//set an individual pin HIGH or LOW
void setRegisterPin(int index, int value){
  registers[index] = value;
}


void writeDigit(int d, bool point){
  clearRegisters();
  for(int i; i<8; i++){
    if(digits[d][i] == 1){
      setRegisterPin(i, HIGH);
    } else {
      setRegisterPin(i, LOW);
    }
  }
  if(point){
      setRegisterPin(4, HIGH);
    } else {
      setRegisterPin(4, LOW);
    }
  writeRegisters();
}

void clearDigit(){
  clearRegisters();
  writeRegisters();
}