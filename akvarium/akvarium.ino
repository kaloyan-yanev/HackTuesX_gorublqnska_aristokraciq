#include <OneWire.h>
#include <DallasTemperature.h>

#define SON_IN 32
#define SON_OUT 22
#define lampPin 2
#define pumpPin 3
#define servoPin 4
#define reostatPin 5

OneWire son_in(SON_IN);
OneWire son_out(SON_OUT);

DallasTemperature sonIn(&son_in);
DallasTemperature sonOut(&son_out);

void setup(void)
{
    Serial.begin(115200);

    pinMode(lampPin, OUTPUT);
    pinMode(pumpPin, OUTPUT);
    pinMode(servoPin, OUTPUT);
    pinMode(reostatPin, OUTPUT);

    sonIn.begin();
    sonOut.begin();
    
}

void loop(void)
{
    sonIn.requestTemperatures();
    sonOut.requestTemperatures();
    Serial.println(sonIn.getTempCByIndex(0));
    Serial.println(sonOut.getTempCByIndex(0));

    if(sonIn.getTempCByIndex(0) > 27){
      digitalWrite(lampPin, HIGH);
      digitalWrite(pumpPin, HIGH);
      digitalWrite(servoPin, HIGH);
      digitalWrite(reostatPin, HIGH);
    }else{
      digitalWrite(lampPin, LOW);
      digitalWrite(pumpPin, LOW);
      digitalWrite(servoPin, LOW);
      digitalWrite(reostatPin, LOW);
    }
}
