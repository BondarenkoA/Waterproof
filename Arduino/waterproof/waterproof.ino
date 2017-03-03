#include <TM1638.h>

#define POWER_DIVIDER 2.0775//коэффициент делителя напряжения для определения напряжения питания. r1/(r1+r2)

byte powerPin = A3;
//int sensorPin1 = A7;    // select the input pin 
byte sensorPowerA = A1; //Pin A of sensor power
byte sensorPowerB = A2; //Pin B of sensor power
byte beeperPin = A0; //Pin beeper
byte motorPin1 = 2; //Pin B of sensor power

byte ledPin = 13;      // select the pin for the LED
byte sensorPins[] = {A4, A5, A6, A7};


byte sensorValue = 0;  // variable to store the value coming from the sensor

int counter = 0;
int powU = 0;
byte beep = 0;

byte alarm = 0;

TM1638 dysplayModule(10, 9, 8);

void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);
  
  pinMode(sensorPowerA, OUTPUT);
  pinMode(sensorPowerB, OUTPUT);
  
  pinMode(ledPin, OUTPUT);
  
  pinMode(beeperPin, OUTPUT);
  pinMode(motorPin1, OUTPUT);

  digitalWrite(motorPin1, HIGH);
  
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // prints title with ending line break
  Serial.println("ready!");

  dysplayModule.setupDisplay(true, 0);
}

void loop() {
  // put your main code here, to run repeatedly:

  counter++;
  
  digitalWrite(ledPin, HIGH);

  Serial.println("================");
  
  sensorValue = sensorRead(4);

  alarm = sensorValue > 50;
  
  digitalWrite(motorPin1, !alarm);

  Serial.print("Sensor = "); Serial.println(sensorValue);
  
  Serial.println(alarm);

  powU = powerReadX100();
  Serial.print("Pow U = "); Serial.println( (float)powU / 100 );
  dysplayModule.setDisplayToDecNumber(powU, 0b00000100, false);

  
  if ( alarm )
  {
    dysplayModule.setLED(TM1638_COLOR_RED, 0);
    delay(200);
    dysplayModule.setLED(TM1638_COLOR_NONE, 0);
    
    tone(beeperPin, 2000, 300);
    
  }


   digitalWrite(ledPin, LOW);

   delay(800);

   digitalWrite(ledPin, LOW);
}

byte sensorRead(int sensorNum){

  int sensorPin = 0;

  sensorPin = sensorPins[sensorNum - 1];
  
  digitalWrite(sensorPowerA, HIGH);
  digitalWrite(sensorPowerB, LOW);

  sensorValue = analogRead(sensorPin);

  digitalWrite(sensorPowerA, LOW);
  digitalWrite(sensorPowerB, HIGH);

  digitalWrite(sensorPowerB, LOW);

  return sensorValue;
}

int powerReadX100(){
  #define SAMPLES_COUNT 50 //количество считываний напряжения батерии для усреднения
  int rawU = 0;

  for(int i = 0; i < SAMPLES_COUNT; i++){
    rawU += analogRead(powerPin);
  }

  rawU = rawU * ( POWER_DIVIDER / SAMPLES_COUNT );
  
  return rawU;
}

