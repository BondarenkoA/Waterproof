#ifndef Out_Helper_h
#define Out_Helper_h
#include "Arduino.h"
#include <TM1638.h>


typedef enum {
    OUT_OFF = 0,
    OUT_ON,
    OUT_DISABLE
}OUT_STATE;

/*
 * Класс для упрощения работы с простыми индикаторами - светодиодами, биперами
 * В конструктор передается номер пина
 * Метод process() вызывается в основном цикле, и на основании счетчиков времени хранящихся в объекте класса,
 * переключаются состояния индикатора
 */
class Out_Helper {
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
        Out_Helper(uint8_t pin);

        virtual void set_state(uint8_t state_in);
        
        void blink(uint16_t on_time, uint16_t off_time = 0, uint8_t count = 0, uint16_t bit_patern = 0);

        void off();
        
        void process();
};

class TM1638_Out : public Out_Helper{
    private :
       TM1638* mp_dysplay_module;
    public :
        
        virtual void set_state(uint8_t state_in){
          mp_dysplay_module->setLED(state_in, m_pin);
        };
     
        TM1638_Out(TM1638* p_dysplay_module_in, uint8_t pin) : Out_Helper(pin){
          mp_dysplay_module = p_dysplay_module_in;
        };
};

#endif
