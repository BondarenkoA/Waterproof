#include <avr/wdt.h>
#include <TM1638.h>
#include "button_helper.h"
#include "out_helper.h"

//#define SERIAL_LOGGING //использования логгирования в COM-порт
#define SERIAL_RATE 115200

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
  #define SERIAL_PRINTLN2(A,B)
  #define SERIAL_PRINT(A)
  #define SERIAL_PRINT2(A,B)
#endif


//Состояния
#define WS_DRY          1
#define WS_WET          2
#define WS_AFTER_WET    3
#define POW_LINE        4
#define POW_BAT         5
#define POW_LOW_BAT     6
#define INP_FORCE_OPEN  7
#define INP_FORCE_CLOSE 8
#define INP_ON_WATCH    9


typedef enum {
   DYS_BAT = 1,
   DYS_SENS,
   DYS_UP_TIME,
   DYS_BEEP_FREQ,
   DYS_LOOP_TIME,
   DYS_NONE//должно быть последним режимом, иначе кольцо не сработает
}DYSPLAY_MODE;


//============= Конфигурация ============================
#define POWER_DIVIDER     8.0311//коэффициент делителя напряжения для определения напряжения питания. r1/(r1+r2)
#define WATER_THRESOLD    200 //чуствительность датчиков воды
#define BATTERY_THRESOLD  1230 //уровень напряжения считающийся питанием от сети
#define BATTERY_LOW_THRESOLD  1190 //уровень напряжения считающийся низким, и приводящий к аварийному перекрытию кранов и т.д.
#define MAX_SENSOR_VAL    1023
#define COUNT_OF_SENSORS  4
#define DYSPLAY_REFRESH_TIME 200
#define WATER_READ_INTERVAL 1000
#define WATER_ALARM_TIME_THRESOLD 2000

byte g_beeper_PIN             = A0; //Pin beeper
byte g_power_sensor_PIN       = A1; //пин АЦП для считывания с делителя напряжения питания
byte g_water_sensor_PIN[COUNT_OF_SENSORS] = {A7, A6, A5, A4}; //Датчики воды
byte g_valve1_PIN             = 2; //Задвижка 1
byte g_valve2_PIN             = 3; //Задвижка 2
byte g_led4_PIN               = 4;
byte g_sensor_btn_PIN         = 8;
byte g_water_sensors_PhA_PIN  = 11; //Фаза A питания датчиков воды( со стороны датчика )
byte g_water_sensors_PhB_PIN  = 12; //Фаза B питания датчиков воды( со стороны резисторов )
byte g_led13_PIN              = 13;

//============= Глобальные переменные =======================
byte g_state_WS  = WS_DRY;
byte g_state_POW = POW_LINE;
byte g_state_INP = INP_ON_WATCH;

int  g_pow_U = 0;
byte g_water_sensors_val = 0;
DYSPLAY_MODE g_dysplay_mode = DYS_LOOP_TIME;

byte     g_dysplay_sensor_num = 1;
uint32_t g_dysplay_last_time = 0;
uint16_t g_beeper_freq = 1083;

uint16_t g_loop_start_time = 0;
uint16_t g_loop_time = 0;
uint16_t g_water_alarm_start_time = 0;
 
byte g_water_sensors_Phase = 0;

//============== Переферия - конопки, LED, дисплей итд ==============
TM1638 dysplayModule(7, 6, 5); //TM1638(dataPin, clockPin, strobePin, activateDisplay, intensity);

TM1638_Button btn_sel_dysplay_mode(&dysplayModule, 0b00000001);
TM1638_Button btn_sel_sensor(&dysplayModule, 0b00000010);
TM1638_Button btn_force_close(&dysplayModule, 0b01000000);
TM1638_Button btn_force_open(&dysplayModule, 0b10000000);

TM1638_Out led_force_close(&dysplayModule, 6);
TM1638_Out led_force_open(&dysplayModule, 7);
//TM1638_Out led_force_open(&dysplayModule, 7);

//**************************************************************************************************
// Инициализация
//**************************************************************************************************
void setup() {
  
  for(int i = 0; i < COUNT_OF_SENSORS; i++) pinMode(g_water_sensor_PIN[i], INPUT);
  
  pinMode(g_power_sensor_PIN, INPUT);
  pinMode(g_sensor_btn_PIN, INPUT_PULLUP);
  
  pinMode(g_water_sensors_PhA_PIN, OUTPUT);
  pinMode(g_water_sensors_PhB_PIN, OUTPUT);

  pinMode(g_led13_PIN, OUTPUT);
  pinMode(g_led4_PIN, OUTPUT);
  pinMode(g_beeper_PIN, OUTPUT);
  pinMode(g_valve1_PIN, OUTPUT);
  pinMode(g_valve2_PIN, OUTPUT);

  digitalWrite(g_valve1_PIN, HIGH);
  digitalWrite(g_valve2_PIN, HIGH);
  
  SERIAL_LOG_BEGIN

  dysplayModule.setupDisplay(true, 0);
  
  wdt_enable(WDTO_8S);
}

//**************************************************************************************************
// Основной цикл
//**************************************************************************************************
void loop() {

  g_loop_start_time = millis();

  read_input();

  determine_state();

  process();

  output();

  //digitalWrite(g_led13_PIN, digitalRead(g_sensor_btn_PIN));
  //digitalWrite(g_led4_PIN, digitalRead(g_sensor_btn_PIN));
  
  //digitalWrite(g_led13_PIN, HIGH);
  //delay(2);
  //digitalWrite(g_led13_PIN, LOW);

  g_loop_time = millis() - g_loop_start_time;

  wdt_reset();
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
    if(sensor_read_V2(i) > WATER_THRESOLD){
       g_water_sensors_val++;
    }
  }
  
  SERIAL_PRINT("Sensor = "); SERIAL_PRINTLN2(g_water_sensors_val, BIN);

  if( g_water_sensors_val && g_water_alarm_start_time == 0 ){
    g_water_alarm_start_time = millis();
  }else if (!g_water_sensors_val && g_water_alarm_start_time > 0){
    g_water_alarm_start_time = 0;
  }
  
  if( g_water_alarm_start_time > 0 && 
      (millis() - g_water_alarm_start_time > WATER_ALARM_TIME_THRESOLD )){
    g_state_WS = WS_WET;
  }else{
    if( g_state_WS == WS_WET ){
      g_state_WS = WS_AFTER_WET;
    }
  }
  
//напряжение питания

  g_pow_U = powerReadX100();
  SERIAL_PRINT("Pow U = "); SERIAL_PRINTLN( (float)g_pow_U / 100 );

  if( g_pow_U > BATTERY_THRESOLD){
    g_state_POW = POW_LINE;
  }else if(g_pow_U > BATTERY_LOW_THRESOLD){
    g_state_POW = POW_BAT;
  }else{
    g_state_POW = POW_LOW_BAT;
  }
  
//кнопки
  btn_sel_dysplay_mode.process();
  btn_sel_sensor.process();
  btn_force_close.process();
  btn_force_open.process();

  
}

//**************************************************************************************************
// determine_state()
//**************************************************************************************************
void determine_state(){
  /*SERIAL_PRINTLN("> determine_state()");

  g_old_state = g_state;

  if (g_water_sensors_val){
    g_state = g_state | WATER_ALARM;
  }else{
    g_state = g_state & ~WATER_ALARM;
  }*/
}

//**************************************************************************************************
// process()
//**************************************************************************************************
void process(){
  SERIAL_PRINTLN("> process()");

  if (g_state_WS != WS_DRY ){
    digitalWrite(g_valve1_PIN, LOW);
    digitalWrite(g_valve2_PIN, LOW);
  }else{
    digitalWrite(g_valve1_PIN, HIGH);
    digitalWrite(g_valve2_PIN, HIGH);
  };

  if (btn_sel_dysplay_mode.is_state(BTN_ON_CLICK) ){
      if (g_dysplay_mode == DYS_NONE)
        g_dysplay_mode = DYS_BAT;
      else
        g_dysplay_mode = g_dysplay_mode + 1;
              
      SERIAL_PRINT("g_dysplay_mode = ");
      SERIAL_PRINTLN(g_dysplay_mode);
  }

  if (g_dysplay_mode == DYS_SENS
      && btn_sel_sensor.is_state(BTN_ON_PRESS) ){
    g_dysplay_sensor_num++;
    if(g_dysplay_sensor_num > COUNT_OF_SENSORS ) g_dysplay_sensor_num = 1;
  }

  if (g_dysplay_mode == DYS_BEEP_FREQ
      && ( btn_sel_sensor.is_state(BTN_HOLD) || btn_sel_sensor.is_state(BTN_ON_PRESS) )){

      g_beeper_freq += 1;
      if(g_beeper_freq > 9999) g_beeper_freq = 0;      
  }
      
  if (btn_force_close.is_state(BTN_ON_PRESS) ) {
    led_force_close.blink(100, 400, 20);
    led_force_open.off();
    
    digitalWrite(g_valve1_PIN, LOW);
    digitalWrite(g_valve2_PIN, LOW);
  }
  if (btn_force_open.is_state(BTN_ON_PRESS) ){
    led_force_close.off();
    led_force_open.blink(100, 400, 20);
    
    digitalWrite(g_valve1_PIN, HIGH);
    digitalWrite(g_valve2_PIN, HIGH);
  }
  
}

//**************************************************************************************************
// output()
//**************************************************************************************************
void output(){
  char str_disp[8] = "";
  
  SERIAL_PRINTLN("> output()");

  if(allow_refresh_TM1638()){
      switch (g_dysplay_mode){
          case DYS_BAT: sprintf(str_disp, "BAT %4d", g_pow_U);
                        dysplayModule.setDisplayToString(str_disp, 0b00000100, 0);
                        break;
          case DYS_SENS: sprintf(str_disp, "S-%1d %4d", g_dysplay_sensor_num, sensor_read_V2(g_dysplay_sensor_num));
                        dysplayModule.setDisplayToString(str_disp, 0b00000000, 0);
                        break;
          case DYS_UP_TIME: sprintf(str_disp, "t %6d", millis() / 1000);
                        dysplayModule.setDisplayToString(str_disp, 0b00000100, 0);
                        break;
          case DYS_BEEP_FREQ: sprintf(str_disp, "BEEP%4d", g_beeper_freq);
                        dysplayModule.setDisplayToString(str_disp, 0b00000000, 0);
                        break;
          case DYS_LOOP_TIME: sprintf(str_disp, "LOOP%4d", g_loop_time);
                        dysplayModule.setDisplayToString(str_disp, 0b00000000, 0);
                        break;
          case DYS_NONE: dysplayModule.clearDisplay();
                        break;
      }
  }
  
  
  if ( btn_sel_dysplay_mode.is_state(BTN_ON_PRESS) ) dysplayModule.setLEDs(0b00001111);//debug string
  if ( btn_sel_dysplay_mode.is_state(BTN_ON_RELEASE) ){ dysplayModule.setLEDs(0x00);};//debug string
 
  led_force_close.process();
  led_force_open.process();
  
  if (g_state_WS != WS_DRY){
    
    tone(g_beeper_PIN, g_beeper_freq, 100);
    /*Громкие частоты:
     * 1071 1083 ~2900 ~3100 ~9500
     
     */
    
    dysplayModule.setLEDs(g_water_sensors_val);
    
  }
  else
  {
   
  }

  
}

//**************************************************************************************************
// int sensor_read(byte sensorNum) Считывание датчика воды
//**************************************************************************************************
int sensor_read(byte sensorNum){
  #define SAMPLES_COUNT 3 //количество считываний напряжения для усреднения
  int val = 0;

  for(int i = 0; i < SAMPLES_COUNT; i++){
    g_water_sensors_Phase = !g_water_sensors_Phase;
    
    digitalWrite(g_water_sensors_PhA_PIN, g_water_sensors_Phase);
    digitalWrite(g_water_sensors_PhB_PIN, !g_water_sensors_Phase);

    delay(2);
    
    if(!g_water_sensors_Phase) {//если фаза отрицательная - инвертируем значение датчика, т.к. поменялась полярность 
      val += MAX_SENSOR_VAL - analogRead(g_water_sensor_PIN[sensorNum-1]);  
    }else{
      val += analogRead(g_water_sensor_PIN[sensorNum-1]);
    }
    
    digitalWrite(g_water_sensors_PhB_PIN, g_water_sensors_Phase);
  }

  val = val / SAMPLES_COUNT;
  
  SERIAL_PRINT("Sensor");SERIAL_PRINT(sensorNum);SERIAL_PRINT(" = ");SERIAL_PRINTLN(val);
  return val;
}

//**************************************************************************************************
// int sensor_read_V2(byte sensorNum) Считывание датчика воды второй вариант
//**************************************************************************************************
int sensor_read_V2(byte sensorNum){
  #define SAMPLES_COUNT 5 //количество считываний напряжения для усреднения
  int val = 0;

  digitalWrite(g_water_sensors_PhA_PIN, LOW);
  digitalWrite(g_water_sensors_PhB_PIN, HIGH);
    
  for(int i = 0; i < SAMPLES_COUNT; i++){
    val += MAX_SENSOR_VAL - analogRead(g_water_sensor_PIN[sensorNum-1]);
    delay(1);
  }

  val = val / SAMPLES_COUNT;
    
  digitalWrite(g_water_sensors_PhB_PIN, LOW);
  
  SERIAL_PRINT("Sensor_V2");SERIAL_PRINT(sensorNum);SERIAL_PRINT(" = ");SERIAL_PRINTLN(val);
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

bool allow_refresh_TM1638(){
  if( millis() > g_dysplay_last_time + DYSPLAY_REFRESH_TIME ){
    g_dysplay_last_time = millis();

    return true;
  }

  return false;
  
}

