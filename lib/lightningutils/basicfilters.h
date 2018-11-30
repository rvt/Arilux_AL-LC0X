#pragma once
#include <stdint.h>
#include "filter.h"

class HSB;

/**
 * Filter for color value
 * Main difference between a filter and effect is that that
 * a filter simply filters a color value for the purpose of color
 * corrections, smooth fadings etc.
 * A filter never ends.
 */
class NoFilter final : public Filter {
public:
    NoFilter();

    virtual HSB handleFilter(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& p_hsb);

};

/**
 * ALter the brightness of a HSB value
 * TODO: Add increse/decrease methods instead of handling this elsewere
 */
class BrightnessFilter final : public Filter {
private:
    int8_t m_increase;
    float m_increaseBy;
public:
    /**
     * p_brightness : Initial brightness
     */
    BrightnessFilter(const float p_increaseBy);

    void increase();

    void decrease();

    virtual HSB handleFilter(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& hsb);

};


class PowerFilter final : public Filter {
private:
    bool m_power;
public:
    /**
     * p_brightness : Initial brightness
     */
    PowerFilter(const bool p_power);

    /**
     * Set the brightness 0..100
     */
    void power(const bool p_brightness);

    bool power() const;

    virtual HSB handleFilter(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& hsb);

};


/**
 * Transitions nicely between two HSB values
 */
class FadingFilter final : public Filter {
private:
    const float m_alpha;
    float m_cptHue;
    float m_cptSaturation;
    float m_cptBrightness;
    float m_cptWhite1;
    float m_cptWhite2;

public:
    /**
     * hsb     : Starting HSB value
     * p_alpha : How quickly we change to the final value 0.f5 is a good start
     */
    FadingFilter(const HSB _hsb, const float p_alpha);

    virtual HSB handleFilter(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& hsb);

};
