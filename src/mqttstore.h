#pragma once
#include <stdint.h>
#include "settingsdto.h"
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.6

class MQTTStore final {
private:
    const char* m_baseTopic;
    const char* m_hsbTopic;
    const char* m_remoteBaseTopic;
    const char* m_stateTopic;
    PubSubClient m_mqttClient;
    bool m_stateInColorTopic;
public:
    MQTTStore(
        const char* p_baseTopic,
        const char* p_hsbTopic,
        const char* p_remoteBaseTopic,
        const char* p_stateTopic,
        const PubSubClient& p_mqttClient,
        const bool p_stateInColorTopic);

    void save(const SettingsDTOData& settings);

private:

    void storeHsb(const SettingsDTOData& settings);
    void storeRemoteBase(const SettingsDTOData& settings);
    void storePower(const SettingsDTOData& settings);
    void publish(const char* baseTopic, const char* topic, const char* payload);
};
