#include <stdlib.h>
#include <cstring>
#include <algorithm>
#include <ostream>

#include "cmdhandler.h"
#include "effect.h"
#include "filter.h"
#include "basicfilters.h"
#include "basiceffects.h"
#include <optparser.h>
#include <helpers.h>
#include <makeunique.h>

constexpr float FILTER_FADING_ALPHA = 0.1f;
constexpr char FILTER_TOPIC[] = "/filter";
constexpr char COLOR_TOPIC[] = "/color";
constexpr char REMOTE_TOPIC[] = "/remote";
constexpr char STATE_TOPIC[] = "/state";
constexpr char EFFECT_TOPIC[] = "/effect";
constexpr char STORE_TOPIC[] = "/store";
constexpr char RESTART_TOPIC[] = "/restart";
constexpr char FILTER[] = "filter";
constexpr char FNAME[] = "name";
constexpr char FILTER_NONE[] = "none";
constexpr char FILTER_FADING[] = "fading";
constexpr char FALPHA[] = "alpha";

constexpr char EFFECT[] = "effect";
constexpr char ENAME[] = "name";
constexpr char EFFECT_NONE[] = "none";
constexpr char EFFECT_FLASH[] = "flash";
constexpr char EFFECT_FADE[] = "fade";
constexpr char EFFECT_RAINBOW[] = "rainbow";
constexpr char TWIDTH[] = "width";
constexpr char TNAME[] = "name";
constexpr char FILTER_DURATION[] = "duration";
constexpr char FILTER_PULSE[] = "pulse";
constexpr char FILTER_PERIOD[] = "period";

constexpr char STATE[] = "state";
constexpr char STATE_ON[] = "ON";
constexpr char STATE_OFF [] = "OFF";

#ifndef UNIT_TEST
#include <Arduino.h>
#else
extern "C" uint32_t millis();
#endif

// std::ostream& operator << ( std::ostream& os, HSB const& value ) {
//constexpr char    os << "HSB(" << value.hue() << "," << value.saturation() << "," << value.brightness() << "," << value.white1() << "," << value.white2() << ")" ;
//     return os;
// }


CmdHandler::CmdHandler(
    uint8_t p_mqttSubscriberTopicStrLength,
    FPower p_fPower,
    FHsb p_fHsb,
    FFilter p_fFilter,
    FEffect p_fEffect,
    FRemoteBase p_fRemoteBase,
    FRestart p_fRestart) :
    m_mqttSubscriberTopicStrLength(p_mqttSubscriberTopicStrLength),
    m_fPower(p_fPower),
    m_fHsb(p_fHsb),
    m_fFilter(p_fFilter),
    m_fEffect(p_fEffect),
    m_fRemoteBase(p_fRemoteBase),
    m_fRestart(p_fRestart)  {
    ;
}
/*
   Function called when a MQTT message arrived
   @param p_topic   The topic of the MQTT message
   @param p_payload The payload of the MQTT message
   @param p_length  The length of the payload
*/
void CmdHandler::handle(const char* p_topic, const char* p_payload, const HSB& p_setHsb, const HSB& p_currentHsb, uint32_t transitionCounter) {
    // Mem copy into own buffer, I am not sure if p_payload is null terminated
    auto payloadlength = strlen(p_payload);

    if (payloadlength >= sizeof(m_mqttReceiveBuffer)) {
        return;
    }

    strncpy(m_mqttReceiveBuffer, p_payload, payloadlength);
    m_mqttReceiveBuffer[payloadlength] = 0;
    auto topicPos = p_topic + m_mqttSubscriberTopicStrLength;

    // Process topics
    // We use strstr if we want to both handle state and commands
    // we use strcmp if we just want to handle command topics
    // Serial.print("Reseived: ");
    // Serial.print(topicPos);
    // Serial.print(" ");
    // Serial.println(m_mqttReceiveBuffer);
    if (strstr(topicPos, COLOR_TOPIC) != nullptr) {
        bool brightnessSet;
        const HSB workingHsb = hsbFromString(p_setHsb, m_mqttReceiveBuffer, &brightnessSet);

        m_fHsb(workingHsb);

        // If ON/OFF are used within the color topic
        OptParser::get(m_mqttReceiveBuffer, [&](OptValue value) {
            if (strcmp(value, STATE_ON) == 0) {
                m_fPower(true);
            } else if (strcmp(value, STATE_OFF) == 0) {
                m_fPower(false);
            }
        });
    }

    if (strstr(topicPos, STATE_TOPIC) != nullptr) {
        if (strcmp(m_mqttReceiveBuffer, STATE_ON) == 0) {
            m_fPower(true);
        } else if (strcmp(m_mqttReceiveBuffer, STATE_OFF) == 0) {
            m_fPower(false);
        }
    }

    if (strstr(topicPos, FILTER_TOPIC) != nullptr) {
        // Get variables from payload
        char* name;
        float alpha = FILTER_FADING_ALPHA;
        OptParser::get(m_mqttReceiveBuffer, [&name, &alpha](OptValue value) {
            // Get variables from filter
            if (strcmp(value.key(), FNAME) == 0) {
                name = value;
            } else if (strcmp(value.key(), FALPHA) == 0) {
                alpha = Helpers::between((float)value, 0.001f, 1.0f);
            }
        });

        // Setup the filter
        if (strcmp(name, FILTER_NONE) == 0) {
            m_fFilter(std::move(std::make_unique<NoFilter>()));
        } else if (strcmp(name, FILTER_FADING) == 0) {
            m_fFilter(std::move(std::make_unique<FadingFilter>(p_currentHsb, alpha)));
        }
    } else if (strstr(topicPos, REMOTE_TOPIC) != nullptr) {
        const uint32_t base = atol(m_mqttReceiveBuffer);
        m_fRemoteBase(base);
    }

    // We absolutly never want to process any messages below here during bootup
    // as they might have odd and unwanted side effects
    if (strstr(topicPos, EFFECT_TOPIC) != 0) {
        const HSB workingHsb = hsbFromString(p_setHsb, m_mqttReceiveBuffer, nullptr);
        // Get variables from payload
        char* name;
        int16_t pulse = -1;
        int16_t period = -1;
        int32_t duration = -1;
        OptParser::get(m_mqttReceiveBuffer, [&name, &period, &pulse, &duration](OptValue value) {
            // Get variables from filter
            if (strcmp(value.key(), ENAME) == 0) {
                name = value;
            }

            if (strcmp(value.key(), FILTER_PERIOD) == 0) {
                period = value;
            }

            if (strcmp(value.key(), FILTER_PULSE) == 0) {
                pulse = value;
            }

            if (strcmp(value.key(), FILTER_DURATION) == 0) {
                duration = value;
            }
        });

        if (strcmp(name, EFFECT_NONE) == 0) {
            m_fEffect(std::move(std::make_unique<NoEffect>()));
        } else if (strcmp(name, EFFECT_RAINBOW) == 0) {
            if (duration > 0) {
                m_fEffect(std::move(std::make_unique<RainbowEffect>(p_currentHsb.hue(), duration, millis())));
            } else {
                m_fEffect(std::move(std::make_unique<RainbowEffect>(millis())));
            }
        } else if (strcmp(name, EFFECT_FLASH) == 0) {
            period = period < 2 ? 2 : period;
            pulse = pulse < period && pulse > 0 ? pulse : period >> 1;

            if (p_setHsb == workingHsb) {
                m_fEffect(std::move(std::make_unique<FlashEffect>(p_currentHsb.toBuilder().brightness(0).build(), transitionCounter, period, pulse)));
            } else {
                m_fEffect(std::move(std::make_unique<FlashEffect>(workingHsb, transitionCounter, period, pulse)));
            }
        } else if (strcmp(name, EFFECT_FADE) == 0) {
            if (duration > 0) {
                m_fEffect(std::move(std::make_unique<TransitionEffect>(workingHsb, millis(), duration)));
            }
        }
    } else if (strcmp(topicPos, RESTART_TOPIC) == 0) {
        if (strcmp(m_mqttReceiveBuffer, "1") == 0) {
            m_fRestart();
        }
    }
}

// brightnessSet will be true when we received a brightness within the string
HSB CmdHandler::hsbFromString(const HSB& hsb, const char* data, bool* brightnessSet) {
    float h, s, b, w1, w2;
    h = hsb.hue();
    s = hsb.saturation();
    b = hsb.brightness();
    w1 = hsb.white1();
    w2 = hsb.white2();

    if (brightnessSet != nullptr) {
        *brightnessSet = false;
    }

    char fData[64];
    strncpy(fData, data, sizeof(fData));

    OptParser::get(fData, [&h, &s, &b, &w1, &w2, &brightnessSet](OptValue f) {
        if (strcmp(f.key(), "hsb")  == 0 || strstr(f.key(), ",") != nullptr) {
            // handle hsb=X,X,X,X,X
            char ff[64];
            strncpy(ff, f, sizeof(ff));
            OptParser::get(ff, ',', [&h, &s, &b, &w1, &w2, &brightnessSet](OptValue value) {
                switch (value.pos()) {
                    case 0:
                        h = Helpers::between((float)value, 0.f, 359.9999f);
                        break;

                    case 1:
                        s = Helpers::between((float)value, 0.f, 100.f);
                        break;

                    case 2:
                        b = Helpers::between((float)value, 0.f, 100.f);

                        if (brightnessSet != nullptr) {
                            *brightnessSet = true;
                        }

                        break;

                    case 3:
                        w1 = Helpers::between((float)value, 0.f, 100.f);
                        break;

                    case 4:
                        w2 = Helpers::between((float)value, 0.f, 100.f);
                        break;
                }
            });
        } else if (strcmp(f.key(), "h") == 0) {
            h = Helpers::between((float)f, 0.f, 359.99f);
        } else if (strcmp(f.key(), "s") == 0) {
            s = Helpers::between((float)f, 0.f, 100.f);
        } else if (strcmp(f.key(), "b") == 0) {
            b = Helpers::between((float)f, 0.f, 100.f);

            if (brightnessSet != nullptr) {
                *brightnessSet = true;
            }
        } else if (strcmp(f.key(), "w1") == 0) {
            w1 = Helpers::between((float)f, 0.f, 100.f);
        } else if (strcmp(f.key(), "w2") == 0) {
            w2 = Helpers::between((float)f, 0.f, 100.f);
        }
    });
    return HSB(h, s, b, w1, w2);
};
