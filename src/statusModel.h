
#pragma once

#include <hsb.h>
#include  <algorithm>

class StatusModel {
    HSB _hsb;
    float _startupBrightness;
    bool  _power;
public:
    StatusModel() :
        _hsb(HSB(0.0f, 0.0f, 50.0f, 0.0f, 0.0f)),
        _startupBrightness(5.0f),
        _power(true) {
    }

    StatusModel(HSB pHsb, float pstartupBrightness, bool pPower) :
        _hsb(pHsb),
        _startupBrightness(pstartupBrightness),
        _power(pPower) {
    }

    bool operator ==(const StatusModel& rhs) const {
        return _hsb == rhs._hsb &&
               _power == rhs._power &&
               _startupBrightness == rhs._startupBrightness;
    }

    bool operator !=(const StatusModel& rhs) const {
        return !(*this == rhs);
    }

    const HSB hsb() const {
        return _hsb;
    }
    void hsb(HSB value) {
        _hsb = value;
        _startupBrightness = std::max(5.0f, _hsb.brightness());
    }
    float startupBrightness() const  {
        return _startupBrightness;
    }
    void startupBrightness(bool value) {
    }
    bool power()  const {
        return _power;
    }
    void power(bool value) {
        _power = value;
    }

};
