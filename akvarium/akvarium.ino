#include <OneWire.h>
#include <DallasTemperature.h>
#include <Arduino.h>

#define lampPin 27
#define pumpPin 35
#define servoPin 34
#define reostatPin 12
#define SON_IN 14
#define SON_OUT 33
#define flowPin 32
#define modeChangePin 15
#define pumpManagePin 17

OneWire temp1(SON_IN);
OneWire temp2(SON_OUT);
DallasTemperature sonIn(&temp1);
DallasTemperature sonOut(&temp2);

long timePumping = 1000;
long timeBetweenPumps = 5000;
bool isPumping = false;
bool isChangingWater = false;
int curTemp = 0;
int wantedTemp = 25;
int modeButtonState;
int startDay = 7;
int endDay = 19;
volatile int flowFreq;
int LToGetOut = 10;
float LPumped = 0.0, l_minute;
bool isLampsOn = false;
bool isFeeding = false;

//============================================================================================

void flow(){
  flowFreq++;
}

void Feeding(int periodFeed){
  bool doneFeeding = false;
  long timeNow = 0;
  digitalWrite(servoPin, HIGH);
  if(doneFeeding == false){
    if(millis() >= timeNow + periodFeed){
      digitalWrite(servoPin, LOW);
      doneFeeding = true;
      isFeeding = false;
    }
  }
}

void Pumping(){
  long timeNow = 0;
  bool donePumping = false;
  int period = timePumping;
  isPumping = true;
  if(donePumping == false){
    if(millis() >= timeNow + period){
       isPumping = false;
       donePumping = true;
    }
  }
}

void LightsOn(){
  digitalWrite(lampPin, HIGH);
}

void LightsOff(){
  digitalWrite(lampPin, LOW);
}

void Warming(int temp){
   if(temp < wantedTemp){
    digitalWrite(reostatPin, HIGH);
   }else{
    digitalWrite(reostatPin, LOW);
   }
}

void setup() {
  Serial.begin(9600);
  pinMode(pumpPin, OUTPUT);
  pinMode(reostatPin, OUTPUT);
  pinMode(lampPin, OUTPUT);
  pinMode(servoPin, OUTPUT);
  pinMode(modeChangePin, INPUT);
  pinMode(pumpManagePin, INPUT);
  pinMode(flowPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(flowPin), flow, RISING);
  
  
  sonIn.begin();
  sonOut.begin();
}

void loop() {
  String msg = Serial.readString();
    String key;
    String value;
    int splitInd = msg.indexOf(',');
    key = msg.substring(0, splitInd);
    value = msg.substring(splitInd+1);
    
    if(key == "feed"){
      Feeding(value.toInt());
    }

    if(key == "lights"){
        int lState = value.toInt();
        if(lState == 1){
          LightsOn();
        }else{
          LightsOff();         
        }
    }

    if(key == "Wanted"){
        wantedTemp = value.toInt();
    }

    if(key == "Liters"){
        LToGetOut = value.toInt();
    }
  sonIn.requestTemperatures();
  int waterTemp = -127;
  waterTemp = sonIn.getTempCByIndex(0);
  sonOut.requestTemperatures();
  int secondWaterTemp = -127;
  secondWaterTemp = sonOut.getTempCByIndex(0);
  
  modeButtonState = digitalRead(modeChangePin);
  if(modeButtonState == HIGH){
    isChangingWater = !isChangingWater; 
    digitalWrite(pumpPin, LOW);
  }
  if(isChangingWater == true){
    bool dirtyWaterOut = false;
    bool setCleanWater = false;
    isPumping = false;
    int pumpMangageState = digitalRead(pumpManagePin);
    if(pumpManagePin == HIGH){
      isPumping = !isPumping;
    }

    if(dirtyWaterOut == false){

      int curFlowTime = 0;
      if(millis() >= curFlowTime + 1000){
        if(flowFreq != 0){
          LPumped += l_minute/60;
          Serial.println(LPumped);
        }else{
          Serial.println(LPumped);
        }
      }
      
      if(isPumping == true){
        digitalWrite(pumpPin, HIGH);
        if(LPumped >= LToGetOut){
          digitalWrite(pumpPin, LOW);
          isPumping = false;
          dirtyWaterOut = true;
          LPumped = 0;
          Serial.println("Dirty water is out. Change the hose to put in clean water");
        }
      }
    }else{
      if(isPumping == true){
        if(secondWaterTemp < wantedTemp-3){
          Serial.println("Temperature of the new water is colder. Put in warmer water");
        }else{
          digitalWrite(pumpPin, HIGH);
          if(LPumped >= LToGetOut){
            digitalWrite(pumpPin, LOW);
            isPumping = false;
            setCleanWater = true;
            LPumped = 0;
            Serial.println("Clean water is set. Getting back to normal mode");
          }
        }
      }
    }
  }
  
  if(isChangingWater == false){
    
    
    
    isPumping = false;
    long timePumpDelay = 0;
    int periodPumpDelay = timeBetweenPumps;
    Warming(waterTemp);

    if(millis() >= timePumpDelay + periodPumpDelay){
      Pumping();
    }
    if(isPumping == true){
        digitalWrite(pumpPin, HIGH);
      }else{
        digitalWrite(pumpPin, LOW);
    }
    if(isLampsOn == true){
      LightsOn();
    }else{
      LightsOff();
    }
    if(isFeeding == true){
      Feeding(500);
    }
  }
}
