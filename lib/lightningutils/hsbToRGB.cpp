#include <hsbToRGB.h>
#include <stddef.h>
#include <cmath>
#include <helpers.h>
#include <cie1931.h>


// Hue degrees point for each color map
// Inspiration is from https://github.com/FastLED/FastLED/wiki/FastLED-HSV-Colors
// The below table implements the rainbow colormap
//                                  0,     45,     90 ,   135,    180,    225,    270,    315,    360
const float r_ledstripmap[] = { 1-1.0f, 1-.85f, 1-.85f, 1-.00f, 1-.00f, 1-.00f, 1-.25f, 1-.75f, 1-1.0f};
const float g_ledstripmap[] = { 1-.00f, 1-.25f, 1-.60f, 1-1.0f, 1-.75f, 1-.00f, 1-.00f, 1-.00f, 1-.00f};
const float b_ledstripmap[] = { 1-.00f, 1-.00f, 1-.00f, 1-.00f, 1-.25f, 1-1.0f, 1-.75f, 1-.25f, 1-.00f};

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
    m_cie1931_size(cie1931_size),
    m_degreeSeperation(360.f / map_size) {
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
 *
 * _h : hue 0..360
 * _s : Saturation 0..100
 * _b : Brightness 0..100
 * _rgb : RGB value as output in the range of 0..100
 */
void HsbToRGBGeneric::toRgb(float _h, float _s, float _b, float _rgb[]) const {

    // Calculate brightness value br is in the range of 0..1
    float br = Helpers::lookupTableInterp(m_cie1931, m_cie1931_size, _b * 2.f);

    // Calculate RGB values each value is in the range of 0..100
    float sepNum = _h / m_degreeSeperation;
    _rgb[0] = Helpers::lookupTableInterp(m_red, m_map_size, sepNum);
    _rgb[1] = Helpers::lookupTableInterp(m_green, m_map_size, sepNum);
    _rgb[2] = Helpers::lookupTableInterp(m_blue, m_map_size, sepNum);

    // Apply saturation RGB values are in the range of 0..100
    // TODO. Should saturation also follow cie1931?
    _rgb[0] = (100.f - _s * _rgb[0]) * br;
    _rgb[1] = (100.f - _s * _rgb[1]) * br;
    _rgb[2] = (100.f - _s * _rgb[2]) * br;
}
