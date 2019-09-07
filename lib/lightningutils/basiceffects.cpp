#include "basiceffects.h"
#include <hsb.h>
#include <Helpers.h>

#ifndef UNIT_TEST
#include <Arduino.h>
#else
extern "C" uint32_t millis();
#endif

NoEffect::NoEffect() : Effect() {
}

HSB NoEffect::handleEffect(const uint32_t p_count,
                           const uint32_t p_time,
                           const HSB& p_hsb) const {
    return p_hsb;
}

HSB NoEffect::finalState(const uint32_t p_count,
                         const uint32_t p_time,
                         const HSB& p_hsb) const {
    return p_hsb;
}


ColorsEffect::ColorsEffect(const std::vector<float> p_colors, uint32_t p_durationPerColor, uint32_t p_startTime) :
    Effect(),
    m_colors(p_colors),
    m_durationPerColor(p_durationPerColor),
    m_startTime(p_startTime) {
}

HSB ColorsEffect::handleEffect(const uint32_t p_count,
                               const uint32_t p_time,
                               const HSB& p_hsb) const {
    if (m_colors.size() == 0) {
        return p_hsb;
    }

    const uint8_t item = ((p_time - m_startTime) / m_durationPerColor) % m_colors.size();
    return p_hsb.toBuilder().hue(m_colors.at(item)).build();
}

HSB ColorsEffect::finalState(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& p_hsb) const {
    return p_hsb;
}


FlashEffect::FlashEffect(const HSB p_hsb,
                         const uint32_t p_currentCount,
                         const uint8_t p_period,
                         const uint8_t p_pulseWidth) :
    Effect(),
    m_hsb(p_hsb),
    m_currentCount(p_currentCount),
    m_period(p_period),
    m_pulseWidth(p_pulseWidth) {
}

HSB FlashEffect::handleEffect(const uint32_t p_count,
                              const uint32_t p_time,
                              const HSB& p_hsb) const {
    const uint32_t cc = p_count % m_period;

    if (cc < m_pulseWidth) {
        return m_hsb;
    } else {
        return p_hsb;
    }
}

HSB FlashEffect::finalState(const uint32_t p_count,
                            const uint32_t p_time,
                            const HSB& p_hsb) const {
    return p_hsb;
}


RainbowEffect::RainbowEffect(uint32_t p_startTime) :
    Effect(),
    m_startHue(0.0),
    m_secPerRotation(10.f),
    m_startTime(p_startTime) {
}

RainbowEffect::RainbowEffect(float p_startHue, float p_secPerRotation, uint32_t p_startTime) :
    Effect(),
    m_startHue(p_startHue),
    m_secPerRotation(p_secPerRotation),
    m_startTime(p_startTime) {
}

HSB RainbowEffect::handleEffect(const uint32_t p_count,
                                const uint32_t p_time,
                                const HSB& p_hsb) const {
    return calculateHsb(p_time, p_hsb);
}

HSB RainbowEffect::finalState(const uint32_t p_count,
                              const uint32_t p_time,
                              const HSB& p_hsb) const {
    return calculateHsb(p_time, p_hsb);
}

HSB RainbowEffect::calculateHsb(const uint32_t p_time, const HSB& hsb) const {
    // rotationSec will count up to 360 in one second
    float rotationSec = ((float)(p_time - m_startTime)) * (360.f / 1000.f);
    float hue = fmod(rotationSec / m_secPerRotation + m_startHue, 360.f);
    return HSB(hue, hsb.saturation(), hsb.brightness(), hsb.white1(), hsb.white2());
}



TransitionEffect::TransitionEffect(const HSB& p_hsb,
                                   const uint32_t p_startMillis,
                                   const uint32_t m_duration) : Effect(),
    m_hsb(p_hsb),
    m_startMillis(p_startMillis),
    m_endMillis(p_startMillis + m_duration),
    m_duration(m_duration) {
}

HSB TransitionEffect::handleEffect(const uint32_t p_count,
                                   const uint32_t p_time,
                                   const HSB& p_hsb) const {
    return calcHSB(p_count, p_time, p_hsb);
}

HSB TransitionEffect::finalState(const uint32_t p_count,
                                 const uint32_t p_time,
                                 const HSB& p_hsb) const {
    return calcHSB(p_count, m_endMillis, p_hsb);
}

bool TransitionEffect::isCompleted(const uint32_t p_count,
                                   const uint32_t p_time,
                                   const HSB& p_hsb) const {
    return p_time > m_endMillis;
}

HSB TransitionEffect::calcHSB(const uint32_t p_count,
                              const uint32_t p_time,
                              const HSB& p_hsb) const {
    const float percent = ((p_time - m_startMillis) * 100) / m_duration;
    const float m_hsbsPath = HSB::hueShortestPath(p_hsb.hue(), m_hsb.hue());

    const float newHue = Helpers::fmap(percent, 0.f, 100.f, p_hsb.hue(), m_hsbsPath);
    return HSB(
               HSB::fixHue(newHue),
               Helpers::fmap(percent, 0.f, 100.f, p_hsb.saturation(), m_hsb.saturation()),
               Helpers::fmap(percent, 0.f, 100.f, p_hsb.brightness(), m_hsb.brightness()),
               Helpers::fmap(percent, 0.f, 100.f, p_hsb.white1(), m_hsb.white1()),
               Helpers::fmap(percent, 0.f, 100.f, p_hsb.white2(), m_hsb.white2()));
}
