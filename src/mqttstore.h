#pragma once
#include <stdint.h>
#include "statusModel.h"
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.6

class MQTTStore final {
private:
    const char* m_baseTopic;
    const char* m_hsbTopic;
    const char* m_stateTopic;
    PubSubClient m_mqttClient;
    bool m_stateInColorTopic;
public:
    MQTTStore(
        const char* p_baseTopic,
        const char* p_hsbTopic,
        const char* p_stateTopic,
        const PubSubClient& p_mqttClient,
        const bool p_stateInColorTopic);

    void save(const StatusModel& settings);

private:

    void storeHsb(const StatusModel& settings);
    void storePower(const StatusModel& settings);
    void publish(const char* baseTopic, const char* topic, const char* payload);
};
