#include <TM1638.h>
#include "button.h"

#define POWER_DIVIDER 2.0775//коэффициент делителя напряжения для определения напряжения питания. r1/(r1+r2)
#define WATER_THRESOLD 50 //чуствительность датчиков воды
#define BATTERY_THRESOLD 1100 //уровень напряжения считающийся низким, и приводящий к аварийному перекрытию кранов и т.д.
#define MAX_SENSOR_VAL 1023

#define SERIAL_RATE 115200

#define SERIAL_LOGGING //использования логгирования в COM-порт
#ifdef SERIAL_LOGGING

  #define SERIAL_LOG_BEGIN Serial.begin(SERIAL_RATE);\
                            while (!Serial) {;}\
                            Serial.println("Serial LOG started!");
  #define SERIAL_PRINTLN(A) Serial.println(A);
  #define SERIAL_PRINTLN2(A,B) Serial.println(A,B);
  #define SERIAL_PRINT(A) Serial.print(A);
  #define SERIAL_PRINT2(A,B) Serial.print(A,B);
#else
  #define SERIAL_LOG_BEGIN
  #define SERIAL_PRINTLN(A)
  #define SERIAL_PRINTLN(A,B)
  #define SERIAL_PRINT(A)
  #define SERIAL_PRINT(A,B)
#endif


//Состояния
#define STAND_BY          0b00000011
#define BEGIN_WATER_ALARM 0b00000100
#define WATER_ALARM       0b00001100
#define END_WATER_ALARM   0b00001000
#define LOW_BATTERY       0b00110000
#define MANUAL_CONTROL    0b11000000

#define COUNT_OF_SENSORS 4

byte g_power_sensor_PIN       = A1;//пин АЦП для считывания делителя вх. напряжения
byte g_water_sensors_PhA_PIN  = 11; //Фаза A питания датчиков воды
byte g_water_sensors_PhB_PIN  = 12; //Фаза B питания датчиков воды
byte g_water_sensor_PIN[COUNT_OF_SENSORS] = {A7, A6, A5, A4}; //Датчики воды
byte g_beeper_PIN             = A0; //Pin beeper
byte g_valve1_PIN             = 2; //Задвижка 1
byte g_valve2_PIN             = 3; //Задвижка 2
byte g_led13_PIN              = 13;      // select the pin for the LED


byte g_state = STAND_BY;
byte g_old_state = STAND_BY;
int g_pow_U = 0;
byte g_water_sensors_val = 0;

int counter = 0;
byte beep = 0;
byte alarm = 0;

byte g_water_sensors_Phase = 0;

TM1638 dysplayModule(7, 6, 5); //TM1638(dataPin, clockPin, strobePin, activateDisplay, intensity);

//**************************************************************************************************
// Инициализация
//**************************************************************************************************
void setup() {

  for(int i = 0; i < COUNT_OF_SENSORS; i++) pinMode(g_water_sensor_PIN[i], INPUT);
  
  pinMode(g_power_sensor_PIN, INPUT);
  
  pinMode(g_water_sensors_PhA_PIN, OUTPUT);
  pinMode(g_water_sensors_PhB_PIN, OUTPUT);

  pinMode(g_led13_PIN, OUTPUT);
  pinMode(g_beeper_PIN, OUTPUT);
  pinMode(g_valve1_PIN, OUTPUT);
  pinMode(g_valve2_PIN, OUTPUT);

  digitalWrite(g_valve1_PIN, HIGH);
  digitalWrite(g_valve2_PIN, HIGH);
  
  SERIAL_LOG_BEGIN

  dysplayModule.setupDisplay(true, 0);
}

//**************************************************************************************************
// Основной цикл
//**************************************************************************************************
void loop() {
  
  read_input();

  determine_state();

  process();

  output();

  digitalWrite(g_led13_PIN, HIGH); delay(1);
  digitalWrite(g_led13_PIN, LOW);
  
  delay(100);
}



//**************************************************************************************************
// read_input() обработка входящей информации(датчики, кнопки, команды и.т.д.) 
//**************************************************************************************************
void read_input(){
  
  SERIAL_PRINTLN("> read_input()");

// датчики воды

  g_water_sensors_val = 0;
  
  for(int i = COUNT_OF_SENSORS; i > 0 ; i--){
    g_water_sensors_val = g_water_sensors_val << 1;
    if(sensor_read(i) > WATER_THRESOLD){
       g_water_sensors_val++;
    }
  }
  
  SERIAL_PRINT("Sensor = "); SERIAL_PRINTLN2(g_water_sensors_val, BIN);
  
//напряжение питания

  g_pow_U = powerReadX100();
  SERIAL_PRINT("Pow U = "); SERIAL_PRINTLN( (float)g_pow_U / 100 );
  
//кнопки
}

//**************************************************************************************************
// determine_state()
//**************************************************************************************************
void determine_state(){
  SERIAL_PRINTLN("> determine_state()");

  g_old_state = g_state;

  if (g_water_sensors_val){
    g_state = g_state | WATER_ALARM;
  }else{
    g_state = g_state & ~WATER_ALARM;
  }
}

//**************************************************************************************************
// process()
//**************************************************************************************************
void process(){
  SERIAL_PRINTLN("> process()");

  if (g_state & WATER_ALARM){
    digitalWrite(g_valve1_PIN, LOW);
    digitalWrite(g_valve2_PIN, LOW);
  }else{
    digitalWrite(g_valve1_PIN, HIGH);
    digitalWrite(g_valve2_PIN, HIGH);
  }
}

//**************************************************************************************************
// output()
//**************************************************************************************************
void output(){
  char str_disp[8];
  
  SERIAL_PRINTLN("> output()");
  
  sprintf(str_disp, "BAT %4d", g_pow_U);
  //dysplayModule.setDisplayToDecNumber(g_pow_U, 0b00000100, false);
  dysplayModule.setDisplayToString(str_disp, 0b00000100, 0);

  if (g_state & WATER_ALARM){
    
    tone(g_beeper_PIN, 800, 50);
    
    dysplayModule.setLEDs(g_water_sensors_val);
    delay(100);
    dysplayModule.setLEDs(0x00);
    
  }
  
}

//**************************************************************************************************
// int sensor_read(byte sensorNum) Считывание датчика воды
//**************************************************************************************************
int sensor_read(byte sensorNum){

  int val = 0;
  
  g_water_sensors_Phase = !g_water_sensors_Phase;
  
  digitalWrite(g_water_sensors_PhA_PIN, g_water_sensors_Phase);
  digitalWrite(g_water_sensors_PhB_PIN, !g_water_sensors_Phase);

  val = analogRead(g_water_sensor_PIN[sensorNum-1]);

  digitalWrite(g_water_sensors_PhB_PIN, g_water_sensors_Phase);

  if(!g_water_sensors_Phase) {//если фаза отрицательная - инвертируем значение датчика, т.к. поменялась полярность 
    val = MAX_SENSOR_VAL - val;  
  }
  
  SERIAL_PRINT("Sensor");SERIAL_PRINT(sensorNum);SERIAL_PRINT(" = ");SERIAL_PRINTLN(val);
  return val;
}

//**************************************************************************************************
// int powerReadX100() Определение напряжения питания х100(умножаем на 100 чтобы целым числом хранить сотые доли)
//**************************************************************************************************
int powerReadX100(){
  #define SAMPLES_COUNT 50 //количество считываний напряжения для усреднения
  int rawU = 0;

  for(int i = 0; i < SAMPLES_COUNT; i++){
    rawU += analogRead(g_power_sensor_PIN);
  }

  rawU = rawU * ( POWER_DIVIDER / SAMPLES_COUNT );
  
  return rawU;
}

