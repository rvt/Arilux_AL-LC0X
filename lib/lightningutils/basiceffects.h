#pragma once
#include <stdint.h>
#include <vector>
#include "effect.h"

#include <hsb.h>


class NoEffect final : public Effect {
private:
public:
    NoEffect();

    virtual HSB handleEffect(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& p_hsb) const;

    virtual HSB finalState(const uint32_t p_count,
                           const uint32_t p_time,
                           const HSB& p_hsb) const;
};

/**
 * Cycle between all colors of the rainbow
 */
class ColorsEffect final : public Effect {
private:
    const std::vector<float> m_colors;
    const uint32_t m_durationPerColor;
    const uint32_t m_startTime;

public:
    explicit ColorsEffect(const std::vector<float> p_colors, uint32_t p_durationPerColor, uint32_t p_startTime);

    virtual HSB handleEffect(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& p_hsb) const;

    virtual HSB finalState(const uint32_t p_count,
                           const uint32_t p_time,
                           const HSB& p_hsb) const;
};

/**
 * Flash/Strobe between two different colors
 */
class FlashEffect final : public Effect {
private:
    const HSB m_hsb;
    const uint32_t m_currentCount;
    const uint8_t m_period;
    const uint8_t m_pulseWidth;

public:
    FlashEffect(const HSB p_hsb,
                const uint32_t p_currentCount,
                const uint8_t p_period,
                const uint8_t p_pulseWidth);

    virtual HSB handleEffect(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& p_hsb) const;

    virtual HSB finalState(const uint32_t p_count,
                           const uint32_t p_time,
                           const HSB& p_hsb) const;
};


/**
 * Cycle between all colors of the rainbow
 */
class RainbowEffect final : public Effect {
private:
    const float m_startHue;
    const float m_secPerRotation;
    const uint32_t m_startTime;
    HSB calculateHsb(const uint32_t p_time, const HSB& hsb) const;
public:
    RainbowEffect(uint32_t p_startTime);
    explicit RainbowEffect(float p_startHue, float p_secPerRotation, uint32_t m_startTime);

    virtual HSB handleEffect(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& p_hsb) const;

    virtual HSB finalState(const uint32_t p_count,
                           const uint32_t p_time,
                           const HSB& p_hsb) const;
};

/**
 * Transitions nicely between two HSB values
 */
class TransitionEffect final : public Effect {
private:
    const HSB m_hsb;
    const uint32_t m_startMillis;
    const uint32_t m_endMillis;
    const uint32_t m_duration;

public:
    /**
     * p_hsb     : Ending HSB value
     */
    TransitionEffect(const HSB& p_hsb, const uint32_t p_startMillis, const uint32_t m_duration);

    virtual HSB handleEffect(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& hsb) const;

    virtual bool isCompleted(const uint32_t p_count,
                             const uint32_t p_time,
                             const HSB& hsb) const;

    virtual HSB finalState(const uint32_t p_count,
                           const uint32_t p_time,
                           const HSB& hsb) const;

private:
    HSB calcHSB(const uint32_t p_count,
                const uint32_t p_time,
                const HSB& hsb) const;
};

