#include "pwmleds.h"
#include "Arduino.h"
#include "config.h"

// PWM Range
#define ARILUX_PWM_RANGE 1023

// Value at which we set to max AND MIN PWM range using
// n > ARILUX_PWM_MAX_RANGE_VALUE ? ARILUX_PWM_RANGE : in < ARILUX_PWM_MIN_RANGE_VALUE ? 0 : in;

#define ARILUX_PWM_MAX_RANGE_VALUE 1023
#define ARILUX_PWM_MIN_RANGE_VALUE 0

// Ranges on which we maximum set each PWM channel
// This can be used as a cheap calibration (curveless)
// or to reduce power output to each LED
#ifndef ARILUX_RED_PWM_RANGE
#define ARILUX_RED_PWM_RANGE  1023
#endif
#ifndef ARILUX_BLUE_PWM_RANGE
#define ARILUX_BLUE_PWM_RANGE  1023
#endif
#ifndef ARILUX_GREEN_PWM_RANGE
#define ARILUX_GREEN_PWM_RANGE  1023
#endif
#ifndef ARILUX_WHITE1_PWM_RANGE
#define ARILUX_WHITE1_PWM_RANGE  1023
#endif
#ifndef ARILUX_WHITE2_PWM_RANGE
#define ARILUX_WHITE2_PWM_RANGE  1023
#endif

#define ARILUX_PWM_FREQUENCY 225

PwmLeds::PwmLeds(const uint8_t red_pin,
        const uint8_t green_pin,
        const uint8_t blue_pin,
        const uint8_t white1_pin,
        const uint8_t white2_pin) :
    m_redPin(red_pin),
    m_greenPin(green_pin),
    m_bluePin(blue_pin),
    m_white1Pin(white1_pin),
    m_white2Pin(white2_pin)
{
}

bool PwmLeds::init(void) const {
    analogWriteRange(ARILUX_PWM_RANGE);
    analogWriteFreq(ARILUX_PWM_FREQUENCY);
    pinMode(m_redPin, OUTPUT);
    pinMode(m_greenPin, OUTPUT);
    pinMode(m_bluePin, OUTPUT);
    if (m_white1Pin!=0) {
        pinMode(m_white1Pin, OUTPUT);
    }
    if (m_white2Pin!=0) {
        pinMode(m_white2Pin, OUTPUT);
    }
    return true;
}

bool PwmLeds::setAll(const float p_red, const float p_green, const float p_blue, const float p_white1, const float p_white2) const {
    auto clamp = [](uint16_t in) {
        return in > ARILUX_PWM_MAX_RANGE_VALUE ? ARILUX_PWM_RANGE : in < ARILUX_PWM_MIN_RANGE_VALUE ? 0 : in;
    };

    auto fmap = [](float x, float in_min, float in_max, float out_min, float out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    };

    analogWrite(m_redPin, clamp(fmap(p_red, 0.f, 100.f, 0.f, ARILUX_RED_PWM_RANGE)));
    analogWrite(m_greenPin, clamp(fmap(p_green, 0.f, 100.f, 0.f, ARILUX_GREEN_PWM_RANGE)));
    analogWrite(m_bluePin, clamp(fmap(p_blue, 0.f, 100.f, 0.f, ARILUX_BLUE_PWM_RANGE)));
    if (m_white1Pin!=0) {
        analogWrite(m_white1Pin, clamp(fmap(p_white1, 0.f, 100.f, 0.f, ARILUX_WHITE1_PWM_RANGE)));
    }
    if (m_white2Pin!=0) {
        analogWrite(m_white2Pin, clamp(fmap(p_white2, 0.f, 100.f, 0.f, ARILUX_WHITE2_PWM_RANGE)));
    }
    return true;
}
