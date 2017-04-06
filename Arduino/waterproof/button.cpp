#include "button.h"

/**
 * Конструктор класса кнопки
 * Кнопка это цифровой пин подтянутый к питанию и замыкаемый на землю
 * Событие срабатывания происходит по нажатию кнопки (возвращается 1)
 * и отпусканию кнопки (возвращается время нажатия кнопки, мсек)
 * time_bounce_in - таймаут дребезга контактов. По умолчанию 50мс
 * time_long_click_in - время длинного нажатия клавиши. По умолчанию 2000мс. Если 0 то отключено.
 */
My_Button::My_Button(uint8_t pin,uint16_t time_bounce_in, uint16_t time_long_click_in){
   Pin               = pin;
   state_is_pressed  = false;
   state_is_long_pressed  = false;
   time_of_last_release               = 0;
   time_of_last_press               = 0;
   time_bounce         = time_bounce_in;
   time_long_click     = time_long_click_in;
}


/**
 * Действие производимое в цикле или по таймеру
 * возвращает BTN_NONE если кнопка не нажата и событие нажатие или динного нажатия кнопки
*/
void My_Button::process() {
   uint32_t current_time = millis();
   bool pin_state = get_btn_state();

   if( pin_state == LOW  // Фиксируем нажатие кнопки ---------------------------------------
       && !state_is_pressed 
       && (current_time - time_of_last_release) > time_bounce ){
        
#ifdef DEBUG_SERIAL_BTN      
       Serial.print(">>>Event button, pin=");Serial.print(Pin);Serial.print(",press ON, tm=");Serial.print(current_time - time_of_last_release);Serial.println(" ms");
#endif    
       state_is_long_pressed = false;  
       state_is_pressed = true;
       time_of_last_press    = current_time;

       state = BTN_ON_PRESS;

       return;
   }
  
   if( pin_state == LOW // Фиксируем длинное нажатие кнопки -----------------------------------
       && !state_is_long_pressed 
       && time_long_click > 0 
       && (current_time - time_of_last_press) > time_long_click ){
      
#ifdef DEBUG_SERIAL_BTN      
      Serial.print(">>>Event button, pin=");Serial.print(Pin);Serial.print(",long press, tm=");Serial.print(current_time - time_of_last_press);Serial.println(" ms");
#endif 
      state_is_long_pressed = true;
      
      state = BTN_ON_HOLD;

      return;
   }

   if( pin_state == HIGH // Фиксируем отпускание кнопки ------------------------------------------
       && state_is_pressed == true  
       && (current_time - time_of_last_press) > time_bounce ){
        
#ifdef DEBUG_SERIAL_BTN      
       Serial.print(">>>Event button, pin=");Serial.print(Pin);Serial.print(",press OFF, tm=");Serial.print(current_time - time_of_last_press);Serial.println(" ms");
#endif    
       state_is_pressed = false;
       time_of_last_release    = current_time;

       if( time_long_click == 0 || 
           (current_time - time_of_last_press) < time_long_click ){  
          state = BTN_ON_CLICK | BTN_RELEASE;
       }else{
          state = BTN_RELEASE;
       }

       return;
       
   }

   state = BTN_NONE;
}     

