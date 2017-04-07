#ifndef simple_out_h
#define simple_out_h
#include "Arduino.h"
#include <TM1638.h>


typedef enum {
    OUT_OFF = 0,
    OUT_ON,
    OUT_DISABLE
}OUT_STATE;

class Simple_Out {
    protected :
       uint8_t  m_pin;
    private :
       OUT_STATE  m_state = OUT_OFF;

       uint32_t m_on_time = 0;
       uint32_t m_off_time = 0;
       uint32_t m_time_to_on = 0;
       uint32_t m_time_to_off = 0;
       uint8_t m_counter = 0;
       uint16_t m_bit_patern = 0;
       
    public :
        Simple_Out(uint8_t pin);

        virtual void set_state(uint8_t state_in);
        
        void blink(uint16_t on_time, uint16_t off_time = 0, uint8_t count = 0, uint16_t bit_patern = 0);

        void off();
        
        void process();
};

class TM1638_Out : public Simple_Out{
    private :
       TM1638* mp_dysplay_module;
    public :
        
        virtual void set_state(uint8_t state_in){
          mp_dysplay_module->setLED(state_in, m_pin);
        };
     
        TM1638_Out(TM1638* p_dysplay_module_in, uint8_t pin) : Simple_Out(pin){
          mp_dysplay_module = p_dysplay_module_in;
        };
};

#endif
