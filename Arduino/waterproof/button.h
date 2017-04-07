#ifndef Button_h
#define Button_h
#include "Arduino.h"
#include <TM1638.h>

//#define DEBUG_SERIAL_BTN 1

typedef enum {
   BTN_NONE = 0,
   BTN_ON_PRESS = 0b00000001,
   BTN_ON_CLICK = 0b00000010,
   BTN_ON_HOLD  = 0b00000100,
   BTN_RELEASE  = 0b00001000 
}BUTTON_STATE;


class My_Button {
  protected :
     uint8_t  Pin;
  private :
     bool     state_is_long_pressed;
     bool     state_is_pressed;
     uint32_t time_of_last_release;
     uint32_t time_of_last_press;
     uint16_t time_bounce;
     uint16_t time_long_click;

     virtual char get_btn_state(){ return digitalRead(Pin); };
  public :
    BUTTON_STATE state = BTN_RELEASE ;
    BUTTON_STATE event = BTN_NONE ;
    
  public :
     My_Button(uint8_t pin, uint16_t time_bounce_in = 50, uint16_t time_long_click_in = 2000);
     
     void process();
        
};     

class TM1638_Button : public My_Button {
  private :
     TM1638* p_dysplay_module;
     
     virtual char get_btn_state(){  return p_dysplay_module->getButtons() != Pin; };
     
  public :
    TM1638_Button (TM1638* p_dysplay_module_in, uint8_t pin,uint16_t time_bounce_in = 50, uint16_t time_long_click_in = 2000) : My_Button(pin, time_bounce_in, time_long_click_in) {
      p_dysplay_module = p_dysplay_module_in;
    };
};

#endif
