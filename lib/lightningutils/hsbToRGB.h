#pragma once

#include <cmath>
#include <cstdlib>

/**
 * Baseclass for HSB to RGB conversions
 */
class HSBToRGB {
public:
    /**
       * generate a new HSB color based on count or time or current HSB color
       */
    virtual void toRgb(float _h, float _s, float _b, float _rgb[]) const = 0;
    virtual ~HSBToRGB() = default;
};

/**
 * Generic converter to convert HSB to RGB
 * Will take in consideration proper dimming curve using cie1931
 * Will take into consideration the rainbow map to convert H values to RGB
 */
class HsbToRGBGeneric final : public HSBToRGB {
private:
    const float* m_red;
    const float* m_green;
    const float* m_blue;
    const size_t m_map_size;
    const float* m_cie1931;
    const size_t m_cie1931_size;
public:
    HsbToRGBGeneric(
        const float* red,
        const float* green,
        const float* blue,
        const size_t map_size,
        const float* cie1931,
        const size_t cie1931_size
    );
    static HSBToRGB* genericLedStrip();
    virtual void toRgb(float _h, float _s, float _b, float _rgb[]) const;
};
