#pragma once
#include <algorithm>
#include <cmath>

class Helpers {

public:

    /**
     * Ensure value n is always lower <= n <= upper
     */
    template <typename T>
    static T between(const T& n, const T& lower, const T& upper) {
        return std::max(lower, std::min(n, upper));
    }

    /**
     * Re-maps a number from one range to another.
     * That is, a value of fromLow would get mapped to toLow, a value of fromHigh to toHigh,
     * values in-between to values in-between, etc.
     * value  : the number to map.
     * in_min : the lower bound of the value’s current range.
     * in_max : the upper bound of the value’s current range.
     * out_min: the lower bound of the value’s target range.
     * out_max: the upper bound of the value’s target range.
     */
    static float fmap(float value, float in_min, float in_max, float out_min, float out_max) {
        return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    // same as fmap execpt the in_min and out_min is always 0.f
    static float fmap(float value, float in_max, float out_max) {
        return value * out_max / in_max;
    }

    /**
     * Maps a input value that is from 0..100.f to a output value out_max
     * for example of value is 75 and out_max is 150 then percentMap returns 112.5
     */
    static float percentmap(float value, float out_max) {
        return value * out_max / 100.f;
    }

    /**
     * Lookup a value from the table and apply interpolation
     * for in between values
     * When value is outside of the range we return the upper or lower value of the table
     * @Param  : table to find the value for
     * @Param  : size of the lookup table
     * @Param  : value to lookup
     * @return : calculated value
     */
    static float lookupTableInterp(const float table[], size_t tSize, float value) {
        if (value < 0) {
            return table[0];
        }

        size_t lowIndex = (size_t)value;
        size_t highIndex = lowIndex + 1;

        if (highIndex >= tSize) {
            return table[tSize - 1];
        }

        float val0 = table[lowIndex];
        float val1 = table[highIndex];

        // Avoid calculations on areas that are not sloped
        if (val0 == val1) {
            return val0;
        }

        float scale = value - floor(value);
        float res = (val1 - val0) * scale + val0;

        return res;
    }
};
