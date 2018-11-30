
#include <functional>
#include <memory>
#include <hsb.h>
#include <effect.h>
#include <filter.h>
#include <propertyutils.h>


class CmdHandler {
    public:
        typedef std::function<void(bool power)> FPower;
        typedef std::function<void(const HSB&)> FHsb;
        typedef std::function<void(std::unique_ptr<Filter>)> FFilter;
        typedef std::function<void(std::unique_ptr<Effect>)> FEffect;
        typedef std::function<void(const uint32_t)> FRemoteBase;
        typedef std::function<void(void)> FRestart;

    public:
        // MQTT buffer
        char m_mqttReceiveBuffer[512];
        uint16_t m_mqttSubscriberTopicStrLength;

        const FPower m_fPower; 
        const FHsb m_fHsb; 
        const FFilter m_fFilter;
        const FEffect m_fEffect;
        const FRemoteBase m_fRemoteBase;
        const FRestart m_fRestart;
        
    public:
        CmdHandler(const Properties& properties,
            FPower p_fPower,
            FHsb p_fHsb,
            FFilter p_fFilter,
            FEffect p_fEffect,
            FRemoteBase p_fRemoteBase,
            FRestart p_fRestart
        );

        void handle(const char* p_topic, const char* p_payload, const HSB& p_setHsb, const HSB& p_currentHsb, uint32_t transitionCounter);
        static HSB hsbFromString(const HSB& hsb, const char* data);
};
