#pragma once
#include <stdint.h>


class Leds {
    public:
        virtual bool setAll(const float p_red, const float p_green, const float p_blue, const float p_white1, const float p_white2) const;
};

