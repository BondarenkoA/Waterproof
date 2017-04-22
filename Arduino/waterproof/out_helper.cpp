#include "out_helper.h"

Out_Helper::Out_Helper(uint8_t pin){
  m_pin = pin;

  m_time_to_on = 0;
  m_time_to_off = 0;
  m_counter = 0;
}


void Out_Helper::set_state(uint8_t state_in){
    digitalWrite(m_pin, state_in);
}


void Out_Helper::blink(uint16_t on_time, uint16_t off_time = 0, uint8_t count = 0, uint16_t bit_patern = 0){
    set_state(HIGH);
    m_on_time = on_time;
    m_off_time = 0;
    m_counter = count;
    m_time_to_off = millis() + on_time;
}

void Out_Helper::off(){
    digitalWrite(m_pin, LOW);
    m_state = OUT_OFF;
}

void Out_Helper::process(){
    uint32_t is_time = millis();

    if( m_time_to_off 
        && is_time >= m_time_to_off ){

        set_state(LOW);

        m_time_to_off = 0;

        if(m_counter){
          m_counter -= 1;

           m_time_to_on = is_time + m_off_time;
           
        }
    }

    if( m_time_to_on 
        && is_time >= m_time_to_on ){

        set_state(HIGH);

        m_time_to_on = is_time + m_off_time;
        m_time_to_off = 0;

        if(m_counter) m_counter -= 1;
    }
}

