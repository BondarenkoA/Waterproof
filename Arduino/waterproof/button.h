#ifndef Button_h
#define Button_h
#include "Arduino.h"

//#define DEBUG_SERIAL_BTN 1

typedef enum {
   SB_NONE = 0,
   SB_CLICK,
   SB_AUTO_CLICK,
   SB_LONG_CLICK,
}SBUTTON_CLICK;


class SButton {
  private :
     uint8_t  Pin;
     bool     State;
     bool     Long_press_state;
     uint32_t Ms1;
     uint32_t Ms2;
     uint32_t Ms_auto_click;
     uint16_t TM_bounce;
     uint16_t TM_long_click;

    virtual char get_btn_state();
  public :
     SButton(uint8_t pin,uint16_t tm1 = 50, uint16_t tm2 = 0);
     void begin();
     SBUTTON_CLICK Loop();
};     


#endif
