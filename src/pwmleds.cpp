#include "pwmleds.h"
#include "Arduino.h"
#include "config.h"
#include <helpers.h>

// PWM Range
#define ARILUX_PWM_RANGE 2047

// Ranges on which we maximum set each PWM channel
// This can be used as a cheap calibration (curveless)
// or to reduce power output to each LED
#ifndef ARILUX_RED_PWM_RANGE
#define ARILUX_RED_PWM_RANGE  2047
#endif
#ifndef ARILUX_BLUE_PWM_RANGE
#define ARILUX_BLUE_PWM_RANGE  2047
#endif
#ifndef ARILUX_GREEN_PWM_RANGE
#define ARILUX_GREEN_PWM_RANGE  2047
#endif
#ifndef ARILUX_WHITE1_PWM_RANGE
#define ARILUX_WHITE1_PWM_RANGE  2047
#endif
#ifndef ARILUX_WHITE2_PWM_RANGE
#define ARILUX_WHITE2_PWM_RANGE  2047
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
    m_white2Pin(white2_pin),
    m_lastRed(0.0f),
    m_lastGreen(0.0f),
    m_lastBlue(0.0f),
    m_lastWhite1(0.0f),
    m_lastWhite2(0.0f) {
}

bool PwmLeds::init(void) const {
    analogWriteRange(ARILUX_PWM_RANGE);
    analogWriteFreq(ARILUX_PWM_FREQUENCY);
    pinMode(m_redPin, OUTPUT);
    pinMode(m_greenPin, OUTPUT);
    pinMode(m_bluePin, OUTPUT);

    if (m_white1Pin != 0) {
        pinMode(m_white1Pin, OUTPUT);
    }

    if (m_white2Pin != 0) {
        pinMode(m_white2Pin, OUTPUT);
    }

    setAll(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    return true;
}

bool PwmLeds::setAll(const float p_red, const float p_green, const float p_blue, const float p_white1, const float p_white2) const {
    // Don´t update the PWN if the changes are relative small
    // Mostly helpfull when there are no changes at all
    /*
    if (fabs(m_lastRed - p_red) < 0.01 &&
        fabs(m_lastGreen - p_green) < 0.01 &&
        fabs(m_lastBlue - p_blue) < 0.01 &&
        fabs(m_lastWhite1 - p_white1) < 0.01 &&
        fabs(m_lastWhite2 - p_white2) < 0.01) {
        return false;
    }

    m_lastRed = p_red;
    m_lastGreen = p_green;
    m_lastBlue = p_blue;
    m_lastWhite1 = p_white1;
    m_lastWhite2 = p_white2;
    */

    analogWrite(m_redPin, Helpers::fmap(p_red, 0.f, 100.f, 0.f, ARILUX_RED_PWM_RANGE));
    analogWrite(m_greenPin, Helpers::fmap(p_green, 0.f, 100.f, 0.f, ARILUX_GREEN_PWM_RANGE));
    analogWrite(m_bluePin, Helpers::fmap(p_blue, 0.f, 100.f, 0.f, ARILUX_BLUE_PWM_RANGE));

    if (m_white1Pin != 0) {
        analogWrite(m_white1Pin, Helpers::fmap(p_white1, 0.f, 100.f, 0.f, ARILUX_WHITE1_PWM_RANGE));
    }

    if (m_white2Pin != 0) {
        analogWrite(m_white2Pin, Helpers::fmap(p_white2, 0.f, 100.f, 0.f, ARILUX_WHITE2_PWM_RANGE));
    }

    return true;
}
