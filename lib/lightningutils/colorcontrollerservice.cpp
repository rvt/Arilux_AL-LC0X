#include "colorcontrollerservice.h"
#include <basiceffects.h>
#include <basicfilters.h>

#ifndef UNIT_TEST
#include <Arduino.h>
#else
extern "C" uint32_t millis();
#endif

#define FILTER_FADING_ALPHA         0.1f


ColorControllerService::ColorControllerService(const HSB& hsb, UpdateFunction p_updateFunction) :
    m_updateFunction(p_updateFunction),
    m_brightnessFilter(std::unique_ptr<BrightnessFilter>(new BrightnessFilter(25))),
    m_powerFilter(std::unique_ptr<PowerFilter>(new PowerFilter(true))),
    m_setHsb(hsb),
    m_currentHsb(hsb),
    m_currentEffect(std::unique_ptr<Effect>(new NoEffect())),
    m_currentFilter(std::unique_ptr<Filter>(new FadingFilter(hsb, FILTER_FADING_ALPHA))) {
}

void ColorControllerService::handle(uint32_t transitionCounter) {
    const uint32_t currentMillis = millis();

    // check if current effect has been completed
    if (m_currentEffect->isCompleted(transitionCounter, currentMillis, m_setHsb)) {
        m_setHsb = m_currentEffect->finalState(transitionCounter, currentMillis, m_setHsb);
        m_currentEffect.reset(new NoEffect());
    }

    m_setHsb = m_brightnessFilter->handleFilter(transitionCounter, currentMillis, m_setHsb);
    m_currentHsb = m_currentEffect->handleEffect(transitionCounter, currentMillis, m_setHsb);
    m_currentHsb = m_powerFilter->handleFilter(transitionCounter, currentMillis, m_currentHsb);
    m_currentHsb = m_currentFilter->handleFilter(transitionCounter, currentMillis, m_currentHsb);
    m_updateFunction(m_currentHsb);
}

void ColorControllerService::effect(std::unique_ptr<Effect> p_effect) {
    m_currentEffect.swap(p_effect);
}

void ColorControllerService::filter(std::unique_ptr<Filter> p_filter) {
    m_currentFilter.swap(p_filter);
}

void ColorControllerService::power(bool onoff) {
    m_powerFilter->power(onoff);
}

void ColorControllerService::hsb(const HSB& hsb) {
    m_setHsb = hsb;
}

const HSB ColorControllerService::hsb() const {
    return m_setHsb;
}

const HSB ColorControllerService::currentHsb() const {
    return m_currentHsb;
}
