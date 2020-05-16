#include "mqttstore.h"

#include <Arduino.h>

constexpr char STATE_ON[] =                         "ON";
constexpr char STATE_OFF[] =                         "OFF";

MQTTStore::MQTTStore(
    const char* p_baseTopic,
    const char* p_hsbTopic,
    const char* p_stateTopic,
    const PubSubClient& p_mqttClient,
    const bool p_stateInColorTopic) :
    m_baseTopic(p_baseTopic),
    m_hsbTopic(p_hsbTopic),
    m_stateTopic(p_stateTopic),
    m_mqttClient(p_mqttClient),
    m_stateInColorTopic(p_stateInColorTopic) {
}

void MQTTStore::save(const StatusModel& settings) {
    storeHsb(settings);
    storePower(settings);
}

void MQTTStore::storeHsb(const StatusModel& settings) {
    char payloadBuffer[64];
    const HSB hsb = settings.hsb();
    snprintf(payloadBuffer, sizeof(payloadBuffer), "hsb=%.2f,%.2f,%.2f,%.2f,%.2f",
             hsb.hue(),
             (hsb.saturation()),
             (hsb.brightness()),
             (hsb.white1()),
             (hsb.white2())
            );

    if (m_stateInColorTopic) {
        char buffer[16] = " state=";
        strncat(buffer, settings.power() ? STATE_ON : STATE_OFF, sizeof(buffer));
        strncat(payloadBuffer, buffer, sizeof(payloadBuffer));
    }

    publish(m_baseTopic, m_hsbTopic, payloadBuffer);
}

void MQTTStore::storePower(const StatusModel& settings) {
    publish(m_baseTopic, m_stateTopic, settings.power() ? STATE_ON : STATE_OFF);
}

void MQTTStore::publish(const char* baseTopic, const char* topic, const char* payload) {
    char topicBuffer[32];
    strncpy(topicBuffer, baseTopic, sizeof(topicBuffer));
    strncat(topicBuffer, "/", sizeof(topicBuffer));
    strncat(topicBuffer, topic, sizeof(topicBuffer));

    if (m_mqttClient.publish(topicBuffer, payload, true)) {
        Serial.print("Published: ");
        Serial.print(topicBuffer);
        Serial.print(" ");
        Serial.println(payload);
    } else {
        Serial.println("Failed to publish to mqtt");
    }
}
