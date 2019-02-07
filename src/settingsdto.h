#pragma once

#include <stdint.h>
#include <cstring>
#include <hsb.h>

struct SettingsDTOData  {
    float m_hue=0;
    float m_saturation=0;
    float m_brightness=50;
    float m_white1=0;
    float m_white2=0;
     uint32_t remoteBase=0;
     uint8_t filter=0;
     bool power=true;
     float brightness=50.0f;

    bool operator==(const  SettingsDTOData& rhs) {
        return
            m_hue == rhs.m_hue &&
            m_saturation == rhs.m_saturation &&
            m_brightness == rhs.m_brightness &&
            m_white1 == rhs.m_white1 &&
            m_white2 == rhs.m_white2 &&
            remoteBase == rhs.remoteBase &&
            filter == rhs.filter &&
            power == rhs.power &&
            brightness == rhs.brightness;
    }

    bool operator!=(const SettingsDTOData& rhs) {
        return !(*this == rhs);
    }

    const HSB hsb() const {
        return HSB(m_hue, m_saturation, m_brightness, m_white1, m_white2);
    }
    void hsb(const HSB &hsb) {
        m_hue = hsb.hue();
        m_saturation = hsb.saturation();
        m_brightness = hsb.brightness();
        m_white1 = hsb.white1();
        m_white2 = hsb.white2();
    }
};


class SettingsDTO final {
private:
    SettingsDTOData m_data;
    SettingsDTOData l_data;
public:


public:
    SettingsDTO(const SettingsDTOData& data) :
        m_data(data),
        l_data(data) {
    }
    SettingsDTO() {
    }

    bool modified()  {
        return m_data != l_data;
    }

    SettingsDTOData* data() {
        return &m_data;
    }

    void data(const SettingsDTOData& data) {
        memcpy(&m_data, &data, sizeof(data));
    }

    void reset() {
        memcpy(&l_data, &m_data, sizeof(m_data));
    }

};