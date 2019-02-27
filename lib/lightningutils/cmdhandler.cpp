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

#define FILTER_FADING_ALPHA         0.1f
#define FILTER_TOPIC                      "/filter"
#define COLOR_TOPIC                       "/color"
#define REMOTE_TOPIC                      "/remote"
#define STATE_TOPIC                       "/state"
#define EFFECT_TOPIC                      "/effect"
#define STORE_TOPIC                       "/store"
#define RESTART_TOPIC                     "/restart"
#define FILTER                      "filter"
#define FNAME                       "name"
#define FILTER_NONE                 "none"
#define FILTER_FADING               "fading"
#define FALPHA                      "alpha"

#define EFFECT                  "effect"
#define ENAME                  "name"
#define EFFECT_NONE             "none"
#define EFFECT_FLASH            "flash"
#define EFFECT_FADE             "fade"
#define EFFECT_RAINBOW             "rainbow"
#define TWIDTH                      "width"
#define TNAME                       "name"
#define FILTER_DURATION                   "duration"
#define FILTER_PULSE                   "pulse"
#define FILTER_PERIOD                  "period"

#define STATE                       "state"
#define STATE_ON                         "ON"
#define STATE_OFF                        "OFF"

#ifndef UNIT_TEST
#include <Arduino.h>
#else
extern "C" uint32_t millis();
#endif

// std::ostream& operator << ( std::ostream& os, HSB const& value ) {
//     os << "HSB(" << value.hue() << "," << value.saturation() << "," << value.brightness() << "," << value.white1() << "," << value.white2() << ")" ;
//     return os;
// }


CmdHandler::CmdHandler(
    const Properties& properties,
    FPower p_fPower,
    FHsb p_fHsb,
    FFilter p_fFilter,
    FEffect p_fEffect,
    FRemoteBase p_fRemoteBase,
    FRestart p_fRestart) :
    m_fPower(p_fPower),
    m_fHsb(p_fHsb),
    m_fFilter(p_fFilter),
    m_fEffect(p_fEffect),
    m_fRemoteBase(p_fRemoteBase),
    m_fRestart(p_fRestart)  {
    m_mqttSubscriberTopicStrLength = properties.get("mqttSubscriberTopicStrLength").getLong();
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

    if (strstr(topicPos, COLOR_TOPIC) != nullptr) {
        const float b = p_setHsb.brightness();
        const HSB workingHsb = hsbFromString(p_setHsb, m_mqttReceiveBuffer);
        m_fHsb(workingHsb);
        // If ON/OFF are used within the color topic
        OptParser::get(m_mqttReceiveBuffer, [&](OptValue v) {
            if (strcmp(v.asChar(), STATE_ON) == 0) {
                m_fPower(true, b == p_setHsb.brightness());
            } else if (strcmp(v.asChar(), STATE_OFF) == 0) {
                m_fPower(false, b == p_setHsb.brightness());
            }
        });
    }

    if (strstr(topicPos, FILTER_TOPIC) != nullptr) {
        // Get variables from payload
        const char* name;
        float alpha = FILTER_FADING_ALPHA;
        OptParser::get(m_mqttReceiveBuffer, [&name, &alpha](OptValue v) {
            // Get variables from filter
            if (strcmp(v.key(), FNAME) == 0) {
                name = v.asChar();
            } else if (strcmp(v.key(), FALPHA) == 0) {
                alpha = std::max(0.001f, std::min(v.asFloat(), 1.0f));
            }
        });

        // Setup the filter
        if (strcmp(name, FILTER_NONE) == 0) {
            m_fFilter(std::move(std::unique_ptr<Filter>(new NoFilter())));
        } else if (strcmp(name, FILTER_FADING) == 0) {
            m_fFilter(std::move(std::unique_ptr<Filter>(new FadingFilter(p_currentHsb, alpha))));
        }
    } else if (strstr(topicPos, REMOTE_TOPIC) != nullptr) {
        const uint32_t base = atol(m_mqttReceiveBuffer);
        m_fRemoteBase(base);
    }

    // We absolutly never want to process any messages below here during bootup
    // as they might have odd and unwanted side effects
    if (strstr(topicPos, EFFECT_TOPIC) != 0) {
        const HSB workingHsb = hsbFromString(p_setHsb, m_mqttReceiveBuffer);
        // Get variables from payload
        const char* name;
        int16_t pulse = -1;
        int16_t period = -1;
        int32_t duration = -1;
        const HSB hsb = hsbFromString(workingHsb, m_mqttReceiveBuffer);
        OptParser::get(m_mqttReceiveBuffer, [&name, &period, &pulse, &duration](OptValue v) {
            // Get variables from filter
            if (strcmp(v.key(), ENAME) == 0) {
                name = v.asChar();
            }

            if (strcmp(v.key(), FILTER_PERIOD) == 0) {
                period = v.asInt();
            }

            if (strcmp(v.key(), FILTER_PULSE) == 0) {
                pulse = v.asInt();
            }

            if (strcmp(v.key(), FILTER_DURATION) == 0) {
                duration = v.asLong();
            }
        });

        if (strcmp(name, EFFECT_NONE) == 0) {
            m_fEffect(std::move(std::unique_ptr<Effect>(new NoEffect())));
        } else if (strcmp(name, EFFECT_RAINBOW) == 0) {
            if (duration > 0) {
                m_fEffect(std::move(std::unique_ptr<Effect>(new RainbowEffect(p_currentHsb.hue(), duration, millis()))));
            } else {
                m_fEffect(std::move(std::unique_ptr<Effect>(new RainbowEffect(millis()))));
            }
        } else if (strcmp(name, EFFECT_FLASH) == 0) {
            period = period < 2 ? 2 : period;
            pulse = pulse < period && pulse > 0 ? pulse : period >> 1;

            if (hsb == workingHsb) {
                m_fEffect(std::move(std::unique_ptr<Effect>(new FlashEffect(p_currentHsb.toBuilder().brightness(0).build(), transitionCounter, period, pulse))));
            } else {
                m_fEffect(std::move(std::unique_ptr<Effect>(new FlashEffect(hsb, transitionCounter, period, pulse))));
            }
        } else if (strcmp(name, EFFECT_FADE) == 0) {
            if (duration > 0) {
                m_fEffect(std::move(std::unique_ptr<Effect>(new TransitionEffect(hsb, millis(), duration))));
            }
        }
    } else if (strcmp(topicPos, RESTART_TOPIC) == 0) {
        if (strcmp(m_mqttReceiveBuffer, "1") == 0) {
            m_fRestart();
        }
    }
}

HSB CmdHandler::hsbFromString(const HSB& hsb, const char* data) {
    float h, s, b, w1, w2;
    h = hsb.hue();
    s = hsb.saturation();
    b = hsb.brightness();
    w1 = hsb.white1();
    w2 = hsb.white2();
    OptParser::get(data, [&h, &s, &b, &w1, &w2](OptValue f) {
        if (strcmp(f.key(), "hsb")  == 0 || strstr(f.key(), ",") != nullptr) {
            OptParser::get(f.asChar(), ",", [&h, &s, &b, &w1, &w2](OptValue c) {
                switch (c.pos()) {
                    case 0:
                        h = Helpers::between(c.asFloat(), 0.f, 359.9999f);
                        break;

                    case 1:
                        s = Helpers::between(c.asFloat(), 0.f, 100.f);
                        break;

                    case 2:
                        b = Helpers::between(c.asFloat(), 0.f, 100.f);
                        break;

                    case 3:
                        w1 = Helpers::between(c.asFloat(), 0.f, 100.f);
                        break;

                    case 4:
                        w2 = Helpers::between(c.asFloat(), 0.f, 100.f);
                        break;
                }
            });
        } else if (strcmp(f.key(), "h") == 0) {
            h = Helpers::between(f.asFloat(), 0.f, 359.99f);
        } else if (strcmp(f.key(), "s") == 0) {
            s = Helpers::between(f.asFloat(), 0.f, 100.f);
        } else if (strcmp(f.key(), "b") == 0) {
            b = Helpers::between(f.asFloat(), 0.f, 100.f);
        } else if (strcmp(f.key(), "w1") == 0) {
            w1 = Helpers::between(f.asFloat(), 0.f, 100.f);
        } else if (strcmp(f.key(), "w2") == 0) {
            w2 = Helpers::between(f.asFloat(), 0.f, 100.f);
        }
    });
    return HSB(h, s, b, w1, w2);
};
