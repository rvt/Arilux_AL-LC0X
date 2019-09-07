#include <hsbToRGB.h>
#include <stddef.h>
#include <cmath>
#include <Helpers.h>
#include <cie1931.h>


// Hue degrees point for each color map
// Inspiration is from https://github.com/FastLED/FastLED/wiki/FastLED-HSV-Colors
// The below table implements the rainbow colormap
//                                 0,    45,    90,    135,   180,  225,   270,   315,   360
const float r_ledstripmap[] = {  1.f,  0.75f,  0.75f,  0.f,  0.f,   0.f,  .25f,  0.75f,  1.f};
const float g_ledstripmap[] = {  0.f,  0.25f,  0.75f,  1.f,  0.75f, 0.f, 0.f,    0.f,    0.f};
const float b_ledstripmap[] = {  0.f,  0.f,    0.f,    0.f,  0.25f, 1.f,  .75f,  0.25f,  0.f};

HsbToRGBGeneric::HsbToRGBGeneric(
    const float* red,
    const float* green,
    const float* blue,
    const size_t map_size,
    const float* cie1931,
    const size_t cie1931_size
) : m_red(red),
    m_green(green),
    m_blue(blue),
    m_map_size(map_size),
    m_cie1931(cie1931),
    m_cie1931_size(cie1931_size) {
}

HSBToRGB* HsbToRGBGeneric::genericLedStrip() {
    return new HsbToRGBGeneric(
               r_ledstripmap,
               g_ledstripmap,
               b_ledstripmap,
               (sizeof(r_ledstripmap) / sizeof(*r_ledstripmap)),
               cie1931,
               cie1931_size
           );
}


/**
 * Convert an HSV value to an RGB value
 */
void HsbToRGBGeneric::toRgb(float _h, float _s, float _b, float _rgb[]) const {

    auto satMap = [](float x, float out_min, float out_max) {
        return x * (out_max - out_min) + out_min;
    };

    // Calculate brightness value br is in the range of 0..1
    float br = Helpers::lookupTableInterp(m_cie1931, m_cie1931_size, _b * 2.f);

    // Calculate RGB values each value is in the range of 0..1
    float degreeSeperation = 360.f / m_map_size;
    _rgb[0] = Helpers::lookupTableInterp(m_red, m_map_size, _h / degreeSeperation);
    _rgb[1] = Helpers::lookupTableInterp(m_green, m_map_size, _h / degreeSeperation);
    _rgb[2] = Helpers::lookupTableInterp(m_blue, m_map_size, _h / degreeSeperation);

    // Calculate weight of RGB values weight is in the range of 0..1
    // float weight = (_rgb[0]*0.299f + _rgb[1]*0.587f + _rgb[2]*0.114f);

    // Calculate inverse s range is 0..1
    float s = (100.f - _s) / 100.f;

    // Apply saturation RGB values are in the range of 0..100
    _rgb[0] = satMap(s, _rgb[0], 1.f) * br;
    _rgb[1] = satMap(s, _rgb[1], 1.f) * br;
    _rgb[2] = satMap(s, _rgb[2], 1.f) * br;
}
