/*
  Alternative firmware for Arilux AL-LC0X series of ESP8266 based RGB LED controllers.
  See the README at https://github.com/mertenats/Arilux_AL-LC0X for more information.
  Licensed under the MIT license.
*/
#include <memory>
#include <cstring>
#include "debug.h"

#include <ESP8266WiFi.h>  // https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>
#include "config.h"

#if defined(IR_REMOTE)
#include <IRremoteESP8266.h> // https://github.com/markszabo/IRremoteESP8266
#endif
#if defined(RF_REMOTE)
#include <RCSwitch.h> // https://github.com/sui77/rc-switch
#endif


#include <ArduinoOTA.h>
#include <ESP_EEPROM.h>

#include "pwmleds.h"
#include "colorcontrollerservice.h"
#include <hsb.h>

// Included in code so we can increase packet size
// This is something that´s not possible with Arduino IDE, currently using v2.6
// #define MQTT_MAX_PACKET_SIZE 128
// #define MQTT_MAX_TRANSFER_SIZE 128
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.6

#include <settings.h>
#include <cmdhandler.h>
#include <helpers.h>
#include "eepromstore.h"
#include "mqttstore.h"
#include <optparser.h>
#include <statemachine.h>

// Effects
#include <effect.h>
#include <filter.h>

// Number of ms per effect transistion, 20ms == 50 Hz
#define FRAMES_PER_SECOND        50
#define EFFECT_PERIOD_CALLBACK   (1000 / FRAMES_PER_SECOND)

#include <propertyutils.h>

typedef PropertyValue PV ;
Properties properties;

// Counter that keeps counting up, used for filters, effects or other means to keep EFFECT_PERIOD_CALLBACK
// of transitions
volatile uint32_t transitionCounter = 1;

// Keep track when the last time we ran the effect state changes
volatile uint32_t effectPeriodStartMillis = 0;

volatile boolean receiveRFCodeFromRemote = true;

// Set to true during cold startup
boolean coldStartupActive = true;

// Pwm Leds handler
std::unique_ptr<PwmLeds> pwmLeds(nullptr);
// 
std::unique_ptr<ColorControllerService> colorControllerService(nullptr);
// Command handler
std::unique_ptr<CmdHandler> cmdHandler(nullptr);

#if defined(RF_REMOTE)
RCSwitch rcSwitch = RCSwitch();
#endif
#if defined(TLS)
WiFiClientSecure wifiClient;
#else
WiFiClient wifiClient;
#endif
PubSubClient mqttClient(wifiClient);

// Settings
SettingsDTO settingsDTO;

// Eeprom storage
// wait 500ms after last commit, then commit no more often than every 30s
std::unique_ptr<EEPromStore> eepromStore(nullptr);
Settings eepromSaveHandler(
    500,
    30000,
    []() {eepromStore->save(settingsDTO);EEPROM.commit();},
    []() {return settingsDTO.modified();}
);

// MQTT Storage
// mqtt updates as quickly as possible with a maximum frequence of MQTT_STATE_UPDATE_DELAY
std::unique_ptr<MQTTStore> mqttStore(nullptr);
Settings mqttSaveHandler(
    1,
    MQTT_STATE_UPDATE_DELAY,
    []() {mqttStore->save(settingsDTO);},
    []() {return settingsDTO.modified();}
);

State* BOOTSEQUENCESTART;
//State* SETUPSERIAL;
State* DELAYEDMQTTCONNECTION;
State* TESTMQTTCONNECTION;
State* CONNECTMQTT;
State* PUBLISHONLINE;
State* SUBSCRIBECOMMANDTOPIC;
State* WAITFORCOMMANDCAPTURE;
// Boot sequence setup
State* bootSequenceStates[7];

std::unique_ptr<StateMachine> bootSequence(nullptr);


///////////////////////////////////////////////////////////////////////////
//  SSL/TLS
///////////////////////////////////////////////////////////////////////////
/*
  Function called to verify the fingerprint of the MQTT server certificate
*/
#if defined(TLS)
void verifyFingerprint() {
    DEBUG_PRINT(F("INFO: Connecting to "));
    DEBUG_PRINTLN(MQTT_SERVER);

    if (!wifiClient.connect(MQTT_SERVER, MQTT_PORT)) {
        DEBUG_PRINTLN(F("ERROR: Connection failed. Halting execution"));
        delay(1000);
        ESP.reset();
    }

    if (wifiClient.verify(TLS_FINGERPRINT, MQTT_SERVER)) {
        DEBUG_PRINTLN(F("INFO: Connection secure"));
    } else {
        DEBUG_PRINTLN(F("ERROR: Connection insecure! Halting execution"));
        delay(1000);
        ESP.reset();
    }
}
#endif

///////////////////////////////////////////////////////////////////////////
//  Utilities
///////////////////////////////////////////////////////////////////////////


/**
 * Publish a message to mqtt
 */
void publishToMQTT(const char* topic, const char* payload) {
    if (mqttClient.publish(topic, payload, true)) {
        DEBUG_PRINT(F("INFO: MQTT message publish succeeded. Topic: "));
        DEBUG_PRINT(topic);
        DEBUG_PRINT(F(". Payload: "));
        DEBUG_PRINTLN(payload);
    } else {
        DEBUG_PRINTLN(F("ERROR: MQTT message publish failed, either connection lost, or message too large"));
    }
}

/**
 * Turns off, effectifly setting brightness to 0
 */
HSB getOffState(const HSB& hsb) {
    return hsb.toBuilder().white1(0).white2(0).brightness(0).build();
}

/**
 * Get´s the ON state of a given HSB
 * When hsn is off it will be turned on with the given brightness
 */
HSB getOnState(const HSB& hsb, float brightness) {
    // If the light is already on, we ignore EEPROM settings
    if (hsb.brightness() > 0) {
        return hsb;
    } else {
        return hsb.toBuilder()
               .brightness(brightness)
               .build();
    }
}

///////////////////////////////////////////////////////////////////////////
//  WiFi
///////////////////////////////////////////////////////////////////////////

void setupWiFi(const Properties &props) {
    auto mqttClientID = props.get("mqttClientID").getCharPtr();
    auto wifi_ssid = props.get("wifi_ssid").getCharPtr();
    auto wifi_password = props.get("wifi_password").getCharPtr();
    WiFi.hostname(mqttClientID);

    delay(10);
    Serial.print(F("INFO: Connecting to: "));
    Serial.println(wifi_ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid, wifi_password);

    randomSeed(micros());
    MDNS.begin(mqttClientID);
}

///////////////////////////////////////////////////////////////////////////
//  Function called to handle received RF codes from the remote
///////////////////////////////////////////////////////////////////////////
#if defined(RF_REMOTE)
void handleRFRemote(void) {
    if (rcSwitch.available()) {
        // Before boot is finnished and the remote
        // is pressed we will store the remote controle base code
        if (receiveRFCodeFromRemote) {
            settingsDTO.remoteBase(rcSwitch.getReceivedValue() & 0xFFFF00);
        }

        const uint32_t value = rcSwitch.getReceivedValue() - settingsDTO.remoteBase();
        DEBUG_PRINT(F("Key Received : "));
        DEBUG_PRINT(value & 0xFFFF00);
        DEBUG_PRINT(F(" / key:"));
        DEBUG_PRINTLN(value);

        switch (value) {
            case ARILUX_REMOTE_KEY_BRIGHT_PLUS:
                brightnessFilter.increase();
                break;

            case ARILUX_REMOTE_KEY_BRIGHT_MINUS:
                brightnessFilter.decrease();
                break;

            case ARILUX_REMOTE_KEY_OFF:
                settingsDTO.power(false);
                colorControllerService->power(false);
                break;

            case ARILUX_REMOTE_KEY_ON:
                settingsDTO.power(true);
                colorControllerService->power(true);
                break;

            case ARILUX_REMOTE_KEY_RED:
                workingHsb = HSB(0, 255 * 4, workingHsb.brightness(), 0, 0);
                break;

            case ARILUX_REMOTE_KEY_GREEN:
                workingHsb = HSB(120, 255 * 4, workingHsb.brightness(), 0, 0);
                break;

            case ARILUX_REMOTE_KEY_BLUE:
                workingHsb = HSB(240, 255 * 4, workingHsb.brightness(), 0, 0);
                break;

            case ARILUX_REMOTE_KEY_WHITE:
                workingHsb = HSB(0, 0, workingHsb.brightness(), 0, 0);
                break;

            case ARILUX_REMOTE_KEY_ORANGE:
                workingHsb = HSB(25, 255 * 4, workingHsb.brightness(), 0, 0);
                break;

            case ARILUX_REMOTE_KEY_LTGRN:
                workingHsb = HSB(120, 100 * 4, workingHsb.brightness(), 0, 0);
                break;

            case ARILUX_REMOTE_KEY_LTBLUE:
                workingHsb = HSB(240, 100 * 4, workingHsb.brightness(), 0, 0);
                break;

            case ARILUX_REMOTE_KEY_AMBER:
                workingHsb = HSB(49, 255 * 4, workingHsb.brightness(), 0, 0);
                break;

            case ARILUX_REMOTE_KEY_CYAN:
                workingHsb = HSB(180, 255 * 4, workingHsb.brightness(), 0, 0);
                break;

            case ARILUX_REMOTE_KEY_PURPLE:
                workingHsb = HSB(300, 255 * 4, workingHsb.brightness(), 0, 0);
                break;

            case ARILUX_REMOTE_KEY_YELLOW:
                workingHsb = HSB(60, 255 * 4, workingHsb.brightness(), 0, 0);
                break;

            case ARILUX_REMOTE_KEY_PINK:
                workingHsb = HSB(350, 64 * 4, workingHsb.brightness(), 0, 0);
                break;

            case ARILUX_REMOTE_KEY_TOGGLE:
                workingHsb = currentEffect->finalState(transitionCounter, millis(), workingHsb);
                currentEffect.reset(new NoEffect());
                break;

            case ARILUX_REMOTE_KEY_SPEED_PLUS:
                // TODO: Implement some incremantal speedup filter
                workingHsb = workingHsb.toBuilder().hue(fmod(workingHsb.hue() + 5, 360.f)).build();
                break;

            case ARILUX_REMOTE_KEY_SPEED_MINUS:
                // TODO: Implement some incremantal speedup filter
                workingHsb = workingHsb.toBuilder().hue(fmod(workingHsb.hue() - 5, 360.f)).build();
                break;

            case ARILUX_REMOTE_KEY_MODE_PLUS:
                workingHsb = workingHsb.toBuilder().saturation(Helpers::between(workingHsb.saturation() + 5, 0.f, 100.f)).build();
                break;

            case ARILUX_REMOTE_KEY_MODE_MINUS:
                workingHsb = workingHsb.toBuilder().saturation(Helpers::between(workingHsb.saturation() - 5, 0.f, 100.f)).build();
                break;

            default:
                DEBUG_PRINTLN(F("ERROR: RF code not defined"));
                break;
        }

        rcSwitch.resetAvailable();
    }
}
#endif

/**
 * Create a new char* from input
 */
char* makeString(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 255, format, args);
    va_end(args);
    return strdup(buffer);
}

void loadConfiguration(Properties &props) {
    // 00FF1234
    const char* chipId = makeString("%08X", ESP.getChipId());
    // ARILUX00FF1234
    props.put("mqttClientID", PV(makeString(HOSTNAME_TEMPLATE, chipId)));
    // RGBW/00FF1234
    const char* mqttTopicPrefix = makeString(MQTT_TOPIC_PREFIX_TEMPLATE, MQTT_PREFIX, chipId);
    props.put("mqttTopicPrefix", PV(mqttTopicPrefix));
    // RGBW/00FF1234/lastwill
    props.put("mqttLastWillTopic", PV(makeString(MQTT_LASTWILL_TOPIC_TEMPLATE, mqttTopicPrefix)));
    //  RGBW/00FF1234/+
    const char* mqttSubscriberTopic = makeString(MQTT_SUBSCRIBER_TOPIC_TEMPLATE, mqttTopicPrefix);
    props.put("mqttSubscriberTopic", PV(mqttSubscriberTopic));
    // Calculate length of the subcriber topic
    props.put("mqttSubscriberTopicStrLength", PV((int32_t)std::strlen(mqttSubscriberTopic) - 2));
    // friendlyName : Arilux LC11 RGBW LED Controller 00FF1234
    props.put("friendlyName", PV(makeString("Arilux %s %s LED Controller %s", DEVICE_MODEL, MQTT_PREFIX, chipId)));

    // Wigi username and password
    props.put("wifi_ssid", PV(WIFI_SSID));
    props.put("wifi_password", PV(WIFI_PASSWORD));

    // mqtt server and port
    props.put("mqtt_server", PV(MQTT_SERVER));
    props.put("mqtt_port", PV(MQTT_PORT));

    delete chipId;
}


/**
 * Trun on the lights after reboot
 * requires eeprom store to be initialised
 */
HSB initialiseAfterStartup() {
    // set color from EEPROM and enable PWM on lights
    // to ensure we turn on light as quickly as possible
    settingsDTO = eepromStore->get();
    if (settingsDTO.brightness() < STARTUP_MIN_BRIGHTNESS) {
        settingsDTO.brightness(STARTUP_MIN_BRIGHTNESS);
    }
    // Enforce on in power filter and settings
    settingsDTO.power(true);
    settingsDTO.reset();

    return getOnState(settingsDTO.hsb(), settingsDTO.brightness());
}


/**
 * Start OTA
 */
void startOTA() {
    // Start OTA
    ArduinoOTA.setHostname(properties.get("mqttClientID").getCharPtr());

    ArduinoOTA.onStart([]() {
        DEBUG_PRINTLN(F("OTA Beginning"));
        pwmLeds->setAll(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    });

    ArduinoOTA.onError([](ota_error_t error) {
        DEBUG_PRINT("ArduinoOTA Error[");
        DEBUG_PRINT(error);
        DEBUG_PRINT("]: ");

        if (error == OTA_AUTH_ERROR) {
            DEBUG_PRINTLN(F("Auth Failed"));
        } else if (error == OTA_BEGIN_ERROR) {
            DEBUG_PRINTLN(F("Begin Failed"));
        } else if (error == OTA_CONNECT_ERROR) {
            DEBUG_PRINTLN(F("Connect Failed"));
        } else if (error == OTA_RECEIVE_ERROR) {
            DEBUG_PRINTLN(F("Receive Failed"));
        } else if (error == OTA_END_ERROR) {
            DEBUG_PRINTLN(F("End Failed"));
        }
    });
    ArduinoOTA.begin();
    ArduinoOTA.handle();
}

///////////////////////////////////////////////////////////////////////////
//  SETUP() AND LOOP()
///////////////////////////////////////////////////////////////////////////

void setup() {
    // setup Strings
    loadConfiguration(properties);

    BOOTSEQUENCESTART = new State([](){
        return TESTMQTTCONNECTION;
    });

    DELAYEDMQTTCONNECTION = new StateTimed(1500, []() {
        return TESTMQTTCONNECTION;
    });

    TESTMQTTCONNECTION = new State([]() {
        receiveRFCodeFromRemote = false;
        if (mqttClient.connected())  {
            if (WiFi.status() != WL_CONNECTED) {
                mqttClient.disconnect();
            }
            return DELAYEDMQTTCONNECTION;
        }
        return CONNECTMQTT;
    });

    CONNECTMQTT = new State([](){
        if (mqttClient.connect(
                properties.get("mqttClientID").getCharPtr(), 
                MQTT_USER, 
                MQTT_PASS, 
                properties.get("mqttLastWillTopic").getCharPtr(), 
                0, 1, MQTT_LASTWILL_OFFLINE)) {
            return PUBLISHONLINE;
        }
        DEBUG_PRINTLN(F("ERROR: The connection to the MQTT broker failed"));
        DEBUG_PRINT(F("Username: "));
        DEBUG_PRINTLN(MQTT_USER);
        DEBUG_PRINT(F("Broker: "));
        DEBUG_PRINTLN(MQTT_SERVER);
        return DELAYEDMQTTCONNECTION;
    });

    PUBLISHONLINE = new State([](){
        publishToMQTT(
            properties.get("mqttLastWillTopic").getCharPtr(), 
            MQTT_LASTWILL_ONLINE);
        return SUBSCRIBECOMMANDTOPIC;
    });

    SUBSCRIBECOMMANDTOPIC = new State([](){
        if (mqttClient.subscribe(properties.get("mqttSubscriberTopic").getCharPtr(), 0)) {
            DEBUG_PRINT(F("INFO: Connected to topic : "));
            return WAITFORCOMMANDCAPTURE;
        }
        DEBUG_PRINT(F("ERROR: Failed to connect to topic : "));
        mqttClient.disconnect();
        return DELAYEDMQTTCONNECTION;
    });

    WAITFORCOMMANDCAPTURE = new StateTimed(3000, [](){
        coldStartupActive = false;
        return TESTMQTTCONNECTION;
    });

    bootSequenceStates[0] = BOOTSEQUENCESTART;
    bootSequenceStates[1] = DELAYEDMQTTCONNECTION;
    bootSequenceStates[2] = TESTMQTTCONNECTION;
    bootSequenceStates[3] = CONNECTMQTT;
    bootSequenceStates[4] = PUBLISHONLINE;
    bootSequenceStates[5] = SUBSCRIBECOMMANDTOPIC;
    bootSequenceStates[6] = WAITFORCOMMANDCAPTURE;
    bootSequence.reset(new StateMachine(7, bootSequenceStates));


    // Enable serial port
    Serial.begin(115200);
    delay(50);
    Serial.println(F("Starting"));
    Serial.println(F("Hostname: "));
    Serial.println(properties.get("mqttClientID").getCharPtr());

    cmdHandler.reset(new CmdHandler (
        properties,
        [](bool p) {
            colorControllerService->power(p);
            settingsDTO.power(p);
        },
        [](const HSB& hsb) {
            colorControllerService->hsb(hsb);
        },
        [](std::unique_ptr<Filter> filter) {
            colorControllerService->filter(std::move(filter));
        },
        [](std::unique_ptr<Effect> effect) {
            colorControllerService->effect(std::move(effect));
        },
        [](const uint32_t base) {
            settingsDTO.remoteBase(base);
        },
        []() {
            ESP.restart();
        }
    ));

    // Setup Wi-Fi    
    setupWiFi(properties);
    startOTA();

   #if defined(PAUSE_FOR_OTA)
        uint16_t i = 0;
        do {
            yield();
            ArduinoOTA.handle();
            yield();
            delay(10);
            i++;
        } while (i < 750);
    #endif

    pwmLeds.reset(new PwmLeds(RED_PIN, GREEN_PIN, BLUE_PIN, WHITE1_PIN, WHITE2_PIN));

    eepromStore.reset(new EEPromStore(0));
    EEPROM.begin(EEPromStore::requestedSize());

    const HSB startHsb = initialiseAfterStartup();

    colorControllerService.reset(new ColorControllerService(startHsb, [](const HSB& currentHsb) {
        // Last filters are for modification of the HSB values before we send it to the DEVICE_MODEL
        // and we don´t want to store these
        const HSB setHsb = colorControllerService->hsb();
        settingsDTO.hsb(setHsb);

        // Store brightness only if it´s >= STARTUP_MIN_BRIGHTNESS
        if (setHsb.brightness() >= STARTUP_MIN_BRIGHTNESS) {
            settingsDTO.brightness(setHsb.brightness());
        }

        float colors[3];
        currentHsb.constantRGB(colors);
        pwmLeds->setAll(colors[0], colors[1], colors[2], currentHsb.cwhite1(), currentHsb.cwhite2());
    }));

    mqttStore.reset(new MQTTStore(
                        properties.get("mqttTopicPrefix").getCharPtr(),
                        MQTT_COLOR_STATE_TOPIC,
                        MQTT_REMOTE_STATE_TOPIC,
                        MQTT_STATE_STATE_TOPIC,
                        mqttClient,
                        STATE_IN_COLOR_TOPIC
                    ));

    #if defined(TLS)
        // Check the fingerprint of CloudMQTT's SSL cert
        verifyFingerprint();
    #endif

    #if defined(ARILUX_DEBUG_TELNET)
        // Start the Telnet server
        startTelnet();
        handleTelnet();
    #endif

    #if defined(IR_REMOTE)
        // Start the IR receiver
        irRecv.enableIRIn();
    #endif

    #if defined(RF_REMOTE)
           // Start the RF receiver
        rcSwitch.enableReceive(RF_PIN);
    #endif

    // Setup mqtt
    mqttClient.setServer(properties.get("mqtt_server").getCharPtr(), properties.get("mqtt_port").getLong());
    mqttClient.setCallback([](char* p_topic, byte* p_payload, uint16_t p_length) {
        char mqttReceiveBuffer[64];
        if (p_length >= sizeof(mqttReceiveBuffer)) {
            DEBUG_PRINT(F("MQTT Message to long."));
            return;
        }

        memcpy(mqttReceiveBuffer, p_payload, p_length);
        mqttReceiveBuffer[p_length] = 0;
        
        cmdHandler->handle(p_topic, mqttReceiveBuffer, colorControllerService->hsb(), colorControllerService->currentHsb(), transitionCounter);
    });

    // Start boot sequence
    bootSequence->start();

    Serial.println(F("Setup done:"));

    // Avoid running towards millis() when loop starts
    effectPeriodStartMillis = millis();
}

#define NUMBER_OF_SLOTS 15
void loop() {

    const uint32_t currentMillis = millis();

    if (currentMillis - effectPeriodStartMillis >= EFFECT_PERIOD_CALLBACK) {

        effectPeriodStartMillis += EFFECT_PERIOD_CALLBACK;
        transitionCounter++;
        colorControllerService->handle(transitionCounter);

        uint8_t slot = 0;

        if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            ArduinoOTA.handle();
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            mqttClient.loop();
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            eepromSaveHandler.handle();
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            mqttSaveHandler.handle();
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            settingsDTO.reset();
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            bootSequence->handle();
        }
#if defined(ARILUX_DEBUG_TELNET)
        else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            // Handle Telnet connection for debugging
            handleTelnet();
        }
#endif
#if defined(IR_REMOTE)
        else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            // Handle received IR codes from the remote
            handleIRRemote();
        }

#endif
#if defined(RF_REMOTE)
        else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            // Handle received RF codes from the remote
            handleRFRemote();
        }
#endif

        const uint32_t thisDuration = (millis() - currentMillis);

        if (thisDuration > EFFECT_PERIOD_CALLBACK) {
            DEBUG_PRINT(F("Spiked : "));
            DEBUG_PRINT(slot);
            DEBUG_PRINT(F(" "));
            DEBUG_PRINTLN(thisDuration);
        }

    }
}
