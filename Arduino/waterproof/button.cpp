#include "button.h"

/**
 * Конструктор класса кнопки
 * Кнопка это цифровой пин подтянутый к питанию и замыкаемый на землю
 * Событие срабатывания происходит по нажатию кнопки (возвращается 1)
 * и отпусканию кнопки (возвращается время нажатия кнопки, мсек)
 * tm1 - таймаут дребезга контактов. По умолчанию 50мс
 * tm2 - время длинного нажатия клавиши. По умолчанию 2000мс
 */
SButton::SButton(uint8_t pin,uint16_t tm1, uint16_t tm2){
   Pin               = pin;
   state_is_pressed             = false;
   Long_press_state  = false;
   Ms1               = 0;
   Ms2               = 0;
   Ms_auto_click     = 0;
   TM_bounce         = tm1;
   TM_long_click     = tm2;
}

/**
 * Инициализация кнопки
 */
void SButton::begin() {
   pinMode(Pin, INPUT_PULLUP);
#ifdef DEBUG_SERIAL_BTN      
   Serial.print("Init button pin ");
   Serial.println(Pin);
#endif      
}

/**
 * Действие производимое в цикле или по таймеру
 * возвращает SB_NONE если кнопка не нажата и событие нажатие или динного нажатия кнопки
*/
SBUTTON_CLICK SButton::Loop() {
   uint16_t delta_t = 0;
   uint32_t ms = millis();
   bool pin_state = digitalRead(Pin);
   
// Фиксируем нажатие кнопки 
   if( pin_state == LOW && state_is_pressed == false && (ms-Ms1) > TM_bounce ){
       delta_t = ms - Ms1;
       Long_press_state = false;
#ifdef DEBUG_SERIAL_BTN      
       Serial.print(">>>Event button, pin=");Serial.print(Pin);Serial.print(",press ON, tm=");Serial.print(delta_t);Serial.println(" ms");
#endif      
       state_is_pressed = true;
       Ms2    = ms;
       if( TM_long_click == 0 )return SB_CLICK;
   }

// Фиксируем длинное нажатие кнопки   
   if( pin_state == LOW && !Long_press_state && TM_long_click > 0 && ( ms - Ms2 )>TM_long_click ){
      delta_t = ms - Ms2;
      Long_press_state = true;
#ifdef DEBUG_SERIAL_BTN      
      Serial.print(">>>Event button, pin=");Serial.print(Pin);Serial.print(",long press, tm=");Serial.print(delta_t);Serial.println(" ms");
#endif 
      return SB_LONG_CLICK;
   }

   
// Фиксируем отпускание кнопки 
   if( pin_state == HIGH && state_is_pressed == true  && (ms-Ms2) > TM_bounce ){
       delta_t = ms - Ms2;
      
       state_is_pressed = false;
#ifdef DEBUG_SERIAL_BTN      
       Serial.print(">>>Event button, pin=");Serial.print(Pin);Serial.print(",press OFF, tm=");Serial.print(delta_t);Serial.println(" ms");
#endif 
      Ms1    = ms;
// Возвращаем короткий клик      
      if( (TM_long_click != 0 ) && !Long_press_state )return SB_CLICK;
       
   }

   return SB_NONE;
}     

