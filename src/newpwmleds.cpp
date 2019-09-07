#include "newpwmleds.h"
#include "Arduino.h"
#include "config.h"
#include <algorithm>
#include <Helpers.h>
extern "C" {
#include "pwm.h"
}

#define PWM_PERIOD 5000

// Ranges on which we maximum set each PWM channel
// This can be used as a cheap calibration (curveless)
// or to reduce power output to each LED
#ifndef ARILUX_RED_PWM_RANGE
#define ARILUX_RED_PWM_RANGE  PWM_PERIOD
#endif
#ifndef ARILUX_BLUE_PWM_RANGE
#define ARILUX_BLUE_PWM_RANGE  PWM_PERIOD
#endif
#ifndef ARILUX_GREEN_PWM_RANGE
#define ARILUX_GREEN_PWM_RANGE  PWM_PERIOD
#endif
#ifndef ARILUX_WHITE1_PWM_RANGE
#define ARILUX_WHITE1_PWM_RANGE  PWM_PERIOD
#endif
#ifndef ARILUX_WHITE2_PWM_RANGE
#define ARILUX_WHITE2_PWM_RANGE  PWM_PERIOD
#endif

NewPwmLeds::NewPwmLeds(const uint8_t red_pin,
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

NewPwmLeds::~NewPwmLeds() {
}

bool NewPwmLeds::init(void)  {
#define TOTALPINS 8
    const uint32_t all_io_info[TOTALPINS][3] = {
        {PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5,   5}, // D1
        {PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4,   4}, // D2
        {PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0,   0}, // D3
        {PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2,   2}, // D4
        {PERIPHS_IO_MUX_MTMS_U,  FUNC_GPIO14, 14}, // D5
        {PERIPHS_IO_MUX_MTDI_U,  FUNC_GPIO12, 12}, // D6
        {PERIPHS_IO_MUX_MTCK_U,  FUNC_GPIO13, 13}, // D7
        {PERIPHS_IO_MUX_MTDO_U,  FUNC_GPIO15, 15}, // D8
    };
    // Find the section to be copied for this pin configuration
    auto pushSetup = [&all_io_info](uint32_t* io_config, const uint8_t pin) {
        for (int8_t i = 0; i < TOTALPINS; i++) {
            if ((uint8_t)all_io_info[i][2] == pin) {
                memcpy(io_config, all_io_info[i], sizeof(all_io_info[i]));
                pinMode(pin, OUTPUT);
                digitalWrite(pin, LOW);
            }
        }
    };
    // Copy settings for the channels
    uint8_t totalChannels = 0;
    pushSetup(io_info[totalChannels++], m_redPin);
    pushSetup(io_info[totalChannels++], m_greenPin);
    pushSetup(io_info[totalChannels++], m_bluePin);

    // Use white channels if available
    if (m_white1Pin != 0) {
        pushSetup(io_info[totalChannels++], m_white1Pin);
    }

    if (m_white2Pin != 0) {
        pushSetup(io_info[totalChannels++], m_white2Pin);
    }

    uint32_t pwm_duty_init[] = {0, 0, 0, 0, 0};
    pwm_init(PWM_PERIOD, pwm_duty_init, totalChannels, io_info);
    pwm_start();
    return true;
}

bool NewPwmLeds::setAll(const float p_red, const float p_green, const float p_blue, const float p_white1, const float p_white2) const {
    // Don´t update the PWN if the changes are relative small
    // Mostly helpfull when there are no changes at all
    if (fabs(m_lastRed - p_red) < 0.005 &&
        fabs(m_lastGreen - p_green) < 0.005 &&
        fabs(m_lastBlue - p_blue) < 0.005 &&
        fabs(m_lastWhite1 - p_white1) < 0.005 &&
        fabs(m_lastWhite2 - p_white2) < 0.005) {
        return false;
    }

    uint8_t channels = 0;
    pwm_set_duty(Helpers::fmap(p_red, 0.f, 100.f, 0.f, ARILUX_RED_PWM_RANGE), channels++);
    pwm_set_duty(Helpers::fmap(p_green, 0.f, 100.f, 0.f, ARILUX_GREEN_PWM_RANGE), channels++);
    pwm_set_duty(Helpers::fmap(p_blue, 0.f, 100.f, 0.f, ARILUX_GREEN_PWM_RANGE), channels++);

    if (m_white1Pin != 0) {
        pwm_set_duty(Helpers::fmap(p_white1, 0.f, 100.f, 0.f, ARILUX_WHITE1_PWM_RANGE), channels++);
    }

    if (m_white2Pin != 0) {
        pwm_set_duty(Helpers::fmap(p_white2, 0.f, 100.f, 0.f, ARILUX_WHITE2_PWM_RANGE), channels++);
    }

    pwm_start(); // commit
    m_lastRed = p_red;
    m_lastGreen = p_green;
    m_lastBlue = p_blue;
    m_lastWhite1 = p_white1;
    m_lastWhite2 = p_white2;
    return true;
}
