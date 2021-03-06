#ifndef Button_h
#define Button_h
#include "Arduino.h"
#include <TM1638.h>

//#define DEBUG_SERIAL_BTN 1

typedef enum {
   BTN_NONE = 0,
   BTN_PRESS        = 0b00000001,
   BTN_ON_PRESS     = 0b10000001,
   BTN_HOLD         = 0b00000011,
   BTN_ON_HOLD      = 0b01000011,
   BTN_RELEASE      = 0b00000100,
   BTN_ON_RELEASE   = 0b00100100,
   BTN_ON_CLICK     = 0b00110100, 
}BUTTON_STATE;

#define BTN_ON_MASK 0b11110000

/*
 * Класс для упрощения работы с кнопками
 * В конструктор передается номер пина
 * Метод process() вызывается в основном цикле, и на основании счетчиков времени хранящихся в объекте класса,
 * происходит переключение состояний обекта:
 * state - текущее состояние конопки, если при очередном вызове process(), ничего не произошло, то состояние остается прежним
 * event - новое состояние которого не было при предыдущем вызове process(). Т.е. состояние которое изменилось именно при последней обработке кнопки.
 */

class Button_Helper {
  protected :
     uint8_t  Pin;
  private :
     bool     state_is_long_pressed;
     bool     state_is_pressed;
     uint32_t time_of_last_release;
     uint32_t time_of_last_press;
     uint16_t time_bounce;
     uint16_t time_long_click;
     BUTTON_STATE state = BTN_RELEASE; 

     virtual char get_btn_state(){ return digitalRead(Pin); };
 
  public :
     Button_Helper(uint8_t pin, uint16_t time_bounce_in = 50, uint16_t time_long_click_in = 1000);

     bool is_state(BUTTON_STATE state_in);
     
     void process();
        
};     

class TM1638_Button : public Button_Helper {
  private :
     TM1638* p_dysplay_module;
     
     virtual char get_btn_state(){  return p_dysplay_module->getButtons() != Pin; };
     
  public :
    TM1638_Button (TM1638* p_dysplay_module_in, uint8_t pin,uint16_t time_bounce_in = 50, uint16_t time_long_click_in = 1000) : Button_Helper(pin, time_bounce_in, time_long_click_in) {
      p_dysplay_module = p_dysplay_module_in;
    };
};

#endif
