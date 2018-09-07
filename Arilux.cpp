#include "Arilux.h"

Arilux::Arilux() : m_redPin(ARILUX_RED_PIN),
    m_greenPin(ARILUX_GREEN_PIN),
    m_bluePin(ARILUX_BLUE_PIN)
#if defined(RGBW) || defined(RGBWW)
    ,
    m_white1Pin(ARILUX_WHITE1_PIN)
#ifdef RGBWW
    ,
    m_white2Pin(ARILUX_WHITE2_PIN)
#endif
#endif
{
}

bool Arilux::init(void) const {
    analogWriteRange(ARILUX_PWM_RANGE);
    analogWriteFreq(ARILUX_PWM_FREQUENCY);
    pinMode(m_redPin, OUTPUT);
    pinMode(m_greenPin, OUTPUT);
    pinMode(m_bluePin, OUTPUT);
#if defined(RGBW) || defined(RGBWW)
    pinMode(m_white1Pin, OUTPUT);
#ifdef RGBWW
    pinMode(m_white2Pin, OUTPUT);
#endif
#endif
    return true;
}

const char* Arilux::getColorString(void) const {
    return (char*)ARILUX_COLOR_STRING;
}

bool Arilux::setAll(const float p_red, const float p_green, const float p_blue, const float p_white1, const float p_white2) const {
    auto clamp = [](uint16_t in) {
        return in > ARILUX_PWM_MAX_RANGE_VALUE ? ARILUX_PWM_RANGE : in < ARILUX_PWM_MIN_RANGE_VALUE ? 0 : in;
    };
    analogWrite(m_redPin, clamp(map(p_red, 0.f, 100.f, 0, ARILUX_PWM_RANGE)));
    analogWrite(m_greenPin, clamp(map(p_green, 0.f, 100.f, 0, ARILUX_PWM_RANGE)));
    analogWrite(m_bluePin, clamp(map(p_blue, 0.f, 100.f, 0, ARILUX_PWM_RANGE)));
#if defined(RGBW) || defined(RGBWW)
    analogWrite(m_white1Pin, clamp(map(p_white2, 0.f, 100.f, 0, ARILUX_PWM_RANGE)));
    #ifdef RGBWW
        analogWrite(m_white2Pin, clamp(map(p_white2, 0.f, 100.f, 0, ARILUX_PWM_RANGE)));
    #endif
#endif
    return true;
}
