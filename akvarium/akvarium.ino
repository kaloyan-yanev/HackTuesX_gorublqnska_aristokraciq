#include <ESP32Servo.h>
#include <RTClib.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define lampPin 14
#define pumpPin 27
#define servoPin 26
#define reostatPin 33
#define termoAkvarium 12
#define termoVunshno 13
#define flowPin 25
#define buzzerPin 4
#define modeChangePin 15
#define pumpManagePin 2

Servo myServo;
OneWire temp1(termoAkvarium);
OneWire temp2(termoVunshno);
DallasTemperature termoSonda1(&temp1);
DallasTemperature termoSonda2(&temp2);
RTC_DS1307 rtc;

int servoPos = 90;
unsigned long timeServoOpen = 3000;
unsigned long timePumping = 1000;
unsigned long timeBetweenPumps = 5000;
bool isPumping = false;
bool isChangingWater = false;
int waterTemp = 0;
int wantedWaterTemp = 25;
int modeButtonState;
bool currentlyFeeding = false;

//============================================================================================

void Feeding(){
  bool doneFeeding = false;
  unsigned long timeNow = 0;
  int period = timeServoOpen;
  servoPos = 180;
  myServo.write(servoPos);
  if(doneFeeding == false){
    if(millis() >= timeNow + period){
      servoPos = 90;
      myServo.write(servoPos);
      doneFeeding = true;
    }
  }
}

void Pumping(){
  bool donePumping = false;
  unsigned long timeNow = 0;
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

void Zagrqvane(){
   if(waterTemp < wantedWaterTemp){
    digitalWrite(reostatPin, HIGH);
   }else{
    digitalWrite(reostatPin, LOW);
   }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Charging the capacitors. Wait 10 seconds");
  delay(10000);
  
  myServo.attach(servoPin);
  myServo.write(servoPos);
  pinMode(pumpPin, OUTPUT);
  pinMode(modeChangePin, INPUT);
  pinMode(pumpManagePin, INPUT);
  if (! rtc.begin()) {
    Serial.println("RTC module is not found");
    while (1);
  }

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  
  termoSonda1.begin();
  termoSonda2.begin();
}

void loop() {
  DateTime currentTime = rtc.now();
  waterTemp = termoSonda1.getTempCByIndex(0);
  modeButtonState = digitalRead(modeChangePin);
  if(modeButtonState == HIGH){
    isChangingWater = !isChangingWater; 
    digitalWrite(pumpPin, LOW);
  }
  if(isChangingWater == true){
    int newWaterTemp = termoSonda2.getTempCByIndex(0);
    int pumpMangageState = digitalRead(pumpManagePin);
    if(newWaterTemp < wantedWaterTemp){
      Serial.println("Temperature of the new water is colder");
    }

    
    if(pumpManagePin == HIGH){
      if(pumpPin == LOW){
        digitalWrite(pumpPin, HIGH);
      }else{
        digitalWrite(pumpPin, LOW);
      }
    }
  }
  if(isChangingWater == false){
    unsigned long timePumpDelay = 0;
    int periodPumpDelay = timeBetweenPumps;
    Zagrqvane();

    if(millis() >= timePumpDelay + periodPumpDelay){
      if(isPumping == true){
        digitalWrite(pumpPin, HIGH);
      }else{
        digitalWrite(pumpPin, LOW);
      }
    }
  }
}
