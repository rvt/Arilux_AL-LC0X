#pragma once
#include <stdint.h>
#include "leds.h"

/**
 * PWM LEDS using https://github.com/StefanBruens/ESP8266_new_pwm
 */
class NewPwmLeds : public Leds {
private:
    const uint8_t m_redPin;
    const uint8_t m_greenPin;
    const uint8_t m_bluePin;
    const uint8_t m_white1Pin;
    const uint8_t m_white2Pin;

    mutable float m_lastRed;
    mutable float m_lastGreen;
    mutable float m_lastBlue;
    mutable float m_lastWhite1;
    mutable float m_lastWhite2;
    mutable uint32_t io_info[5][3];
public:
    NewPwmLeds(const uint8_t red_pin, const uint8_t green_pin, const uint8_t blue_pin, const uint8_t white1_pin, const uint8_t white2_pin);
    virtual ~NewPwmLeds();
    bool init(void) ;
    virtual bool setAll(const float p_red, const float p_green, const float p_blue, const float p_white1, const float p_white2) const;

};
