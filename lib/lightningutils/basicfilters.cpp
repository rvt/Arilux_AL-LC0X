#include "basicfilters.h"
#include <helpers.h>
#include <hsb.h>

NoFilter::NoFilter() : Filter() {
}

HSB NoFilter::handleFilter(const uint32_t p_count,
                           const uint32_t p_time,
                           const HSB& p_hsb) {
    return p_hsb;
}


BrightnessFilter::BrightnessFilter(const float p_increaseBy) :
    Filter(),
    m_increase(0),
    m_increaseBy(p_increaseBy) {
}

HSB BrightnessFilter::handleFilter(const uint32_t p_count,
                                   const uint32_t p_time,
                                   const HSB& _hsb) {
    if (m_increase == 0) {
        return _hsb;
    }

    if (m_increase == 1 && _hsb.brightness() >= 100.f && _hsb.white1() >= 100.f && _hsb.white2() >= 100.f) {
        m_increase = 0;
        return _hsb;
    } else if (m_increase == -1 && _hsb.brightness() == 0 && _hsb.white1() == 0 && _hsb.white2() == 0) {
        m_increase = 0;
        return _hsb;
    }

    float m_brightness;

    if (m_increase == 1) {
        m_brightness = m_increaseBy;
    } else if (m_increase == -1) {
        m_brightness = - m_increaseBy;
    } else {
      m_brightness = 0;
    }

    m_increase = 0;
    float brightness = _hsb.brightness() + _hsb.brightness() / 100.f * m_brightness;
    float white1 = _hsb.white1() + _hsb.white1() / 100.f * m_brightness;
    float white2 = _hsb.white2() + _hsb.white2() / 100.f * m_brightness;

    // Don´t allow to turn off with brightness controls
    // This has the side effect that we cannot turn on or if we do we don´t
    // know anymore what leds where on.
    if (white1 < 10.f && white2 < 10.f && brightness < 10.f) {
        return _hsb;
    }

    return _hsb.toBuilder()
           .white1(Helpers::between(white1, 0.f, 100.f))
           .white2(Helpers::between(white2, 0.f, 100.f))
           .brightness(Helpers::between(brightness, 0.f, 100.f))
           .build();
}

void BrightnessFilter::increase() {
    m_increase = 1;
}

void BrightnessFilter::decrease() {
    m_increase = -1;
}


PowerFilter::PowerFilter(const bool p_power) : Filter(),
    m_power(p_power) {
}

void PowerFilter::power(const bool p_power) {
    m_power = p_power;
}

bool PowerFilter::power() const {
    return m_power;
}

HSB PowerFilter::handleFilter(const uint32_t p_count,
                              const uint32_t p_time,
                              const HSB& _hsb) {
    return _hsb.toBuilder()
           .white1(m_power ? _hsb.white1() : 0.f)
           .white2(m_power ? _hsb.white2() : 0.f)
           .brightness(m_power ? _hsb.brightness() : 0.f)
           .build();
}


FadingFilter::FadingFilter(const HSB _hsb, const float p_alpha) : Filter(),
    m_alpha(p_alpha),
    m_cptHue(_hsb.hue()),
    m_cptSaturation(_hsb.saturation()),
    m_cptBrightness(_hsb.brightness()),
    m_cptWhite1(_hsb.white1()),
    m_cptWhite2(_hsb.white2()) {
}

float newCValue(const float m_alpha, const float toV, const float fromV) {
    return  fromV + (toV - fromV) * m_alpha;
}

HSB FadingFilter::handleFilter(const uint32_t p_count,
                               const uint32_t p_time,
                               const HSB& _hsb) {
    const auto sPathHue = HSB::hueShortestPath((float)_hsb.hue(), m_cptHue);
    const auto dwHue = _hsb.hue() - sPathHue;
    const auto dwSat = _hsb.saturation() - m_cptSaturation;
    const auto dwBright = _hsb.brightness() - m_cptBrightness;
    const auto dw1 = _hsb.white1() - m_cptWhite1;
    const auto dw2 = _hsb.white2() - m_cptWhite2;
    m_cptHue = sPathHue + dwHue * m_alpha;
    m_cptSaturation = m_cptSaturation + dwSat * m_alpha;
    m_cptBrightness = m_cptBrightness + dwBright * m_alpha;
    m_cptWhite1 = m_cptWhite1 + dw1 * m_alpha;
    m_cptWhite2 = m_cptWhite2 + dw2 * m_alpha;
    return HSB(HSB::fixHue(m_cptHue), m_cptSaturation, m_cptBrightness, m_cptWhite1, m_cptWhite2);
}

//    auto sPathHue = HSB::hueShortestPath((float)_hsb.hue(), m_cptHue);
//    m_cptHue = newCValue(m_alpha, sPathHue, m_cptHue);
//    m_cptSaturation = newCValue(m_alpha, _hsb.saturation(), m_cptSaturation);
//    m_cptBrightness = newCValue(m_alpha, _hsb.brightness(), m_cptBrightness);
//    m_cptWhite1 = newCValue(m_alpha, _hsb.white1(), m_cptWhite1);
//    m_cptWhite2 = newCValue(m_alpha, _hsb.white2(), m_cptWhite2);
//    return HSB(HSB::fixHue(m_cptHue), m_cptSaturation, m_cptBrightness, m_cptWhite1, m_cptWhite2);
