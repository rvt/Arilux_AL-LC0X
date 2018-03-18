#include "HSB.h"

HSB::HSB(int p_hue, int p_saturation, int p_brightness, int p_white1, int p_white2)
    : m_hue(p_hue % 360),
      m_saturation(p_saturation),
      m_brightness(p_brightness),
      m_white1(p_white1),
      m_white2(p_white1) {
}

HSB::HSB(const HSB& p_hsb)
    : m_hue(p_hsb.m_hue),
      m_saturation(p_hsb.m_saturation),
      m_brightness(p_hsb.m_brightness),
      m_white1(p_hsb.m_white1),
      m_white2(p_hsb.m_white2) {
}

HSBBuilder HSB::toBuilder() {
    return HSBBuilder(m_hue, m_saturation, m_brightness, m_white1, m_white2);
}

void HSB::getHSB(int colors[]) const {
    colors[0] = m_hue;
    colors[1] = m_saturation;
    colors[2] = m_brightness;
}

int HSB::getHue() const {
    return m_hue;
}

int HSB::getSaturation() const {
    return m_saturation;
}

int HSB::getBrightness() const {
    return m_brightness;
}

int HSB::getWhite1() const {
    return m_white1;
}

int HSB::getWhite2() const {
    return m_white1;
}

int HSB::getCWhite1() const {
    return m_white1;
}

int HSB::getCWhite2() const {
    return m_white2;
}

void HSB::getConstantRGB(int colors[]) const {
    unsigned long r_temp, g_temp, b_temp;
    int inverse_sat = 1020 - m_saturation;
    int index_mod = m_hue % 120;

    if (m_hue < 120) {
        r_temp = 120 - index_mod;
        g_temp = index_mod;
        b_temp = 0;
    } else if (m_hue < 240) {
        r_temp = 0;
        g_temp = 120 - index_mod;
        b_temp = index_mod;
    } else if (m_hue < 360) {
        r_temp = index_mod;
        g_temp = 0;
        b_temp = 120 - index_mod;
    } else {
        r_temp = 0;
        g_temp = 0;
        b_temp = 0;
    }

    r_temp = ((r_temp * m_saturation) / 120) + inverse_sat;
    g_temp = ((g_temp * m_saturation) / 120) + inverse_sat;
    b_temp = ((b_temp * m_saturation) / 120) + inverse_sat;
    r_temp = (r_temp * m_brightness) / 1020;
    g_temp = (g_temp * m_brightness) / 1020;
    b_temp = (b_temp * m_brightness) / 1020;
    colors[0]   = r_temp;
    colors[1]   = g_temp;
    colors[2]   = b_temp;
}

bool HSB::operator ==(const HSB& rhs) const {
    return m_hue == rhs.m_hue &&
           m_brightness == rhs.m_brightness &&
           m_saturation == rhs.m_saturation &&
           m_white1 == rhs.m_white1 &&
           m_white2 == rhs.m_white2;
}

bool HSB::operator !=(const HSB& rhs) const {
    return m_hue != rhs.m_hue ||
           m_brightness != rhs.m_brightness ||
           m_saturation != rhs.m_saturation ||
           m_white1 != rhs.m_white1 ||
           m_white2 != rhs.m_white2;
}

HSB& HSB::operator = (const HSB& rhs) {
    if (&rhs == this) {
        return *this;
    }

    m_hue = rhs.m_hue;
    m_brightness = rhs.m_brightness;
    m_saturation = rhs.m_saturation;
    m_white1 = rhs.m_white1;
    m_white2 = rhs.m_white2;
    return *this;
}
