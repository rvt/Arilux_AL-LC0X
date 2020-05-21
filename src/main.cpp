/*
  Alternative firmware for Arilux AL-LC0X series of ESP8266 based RGB LED controllers.
  See the README at https://github.com/mertenats/Arilux_AL-LC0X for more information.
  Licensed under the MIT license.
*/
#include <memory>
#include <cstring>

#include <ESP8266WiFi.h>  // https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>
#include "config.h"
//#include <crceeprom.h>

#include <ArduinoOTA.h>
//#include <ESP_EEPROM.h>
#include "LittleFS.h"
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WifiManagerTypes.h>

#include "pwmleds.h"
#include "newpwmleds.h"
#include "colorcontrollerservice.h"
#include "statusModel.h"
#include <hsb.h>
#include "jscript.generated.h"
#include "customConfig_html.generated.h"

// Included in code so we can increase packet size
// This is something that´s not possible with Arduino IDE, currently using v2.6
// #define MQTT_MAX_PACKET_SIZE 128
// #define MQTT_MAX_TRANSFER_SIZE 128
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.6

#include <settings.h>
#include <cmdhandler.h>
#include <helpers.h>
#include "mqttstore.h"
#include <optparser.h>
#include <statemachine.h>
#include <hsbToRGB.h>
#include <propertyutils.h>

// Effects
#include <effect.h>
#include <filter.h>


typedef PropertyValue PV ;
bool controllerConfigModified;

Properties controllerConfig;
bool storeLedStatusConfigInLittleFS;
Properties ledStatusConfig;

// Counter that keeps counting up, used for filters, effects or other means to keep EFFECT_PERIOD_CALLBACK
// of transitions
uint32_t transitionCounter = 1;

// Keep track when the last time we ran the effect state changes
uint32_t effectPeriodStartMillis = 0;

// True when we first startup
boolean startupFromBoot = true;

// Indicate that a service requested an restart. Set to millies() of current time and it will restart 5000ms later
uint32_t shouldRestart = 0;

// Pwm Leds handler
std::unique_ptr<Leds> pwmLeds(nullptr);
//
std::unique_ptr<ColorControllerService> colorControllerService(nullptr);
// Command handler
std::unique_ptr<CmdHandler> cmdHandler(nullptr);

// hsb2RGB converter
std::unique_ptr<HSBToRGB> hSBToRGB(nullptr);

StatusModel ledStatusModel;
StatusModel lastStatusModel;


constexpr char JBONE_URI[] =                       "/jscript.js";
WiFiManager wm;
#define MQTT_SERVER_LENGTH 40
#define MQTT_PORT_LENGTH 5
#define MQTT_USERNAME_LENGTH 18
#define MQTT_PASSWORD_LENGTH 18
const char _default_str[] = "";
const char _customHtml_password[] = "type=\"password\" ";
const char _customHtml_check[] = "type=\"checkbox\" ";
const char _customHtml_hidden[] = "type=\"hidden\" ";
WiFiManagerParameter wm_mqtt_server("server", "mqtt server", _default_str, sizeof(_default_str)+1);
IntParameter wm_mqtt_port("port", "mqtt port", "type='number' min='0' max='65535' onkeypress='return isNumberKey(event)' ", WFM_LABEL_BEFORE);
WiFiManagerParameter wm_mqtt_user("user", "mqtt username", _default_str, sizeof(_default_str)+1);
WiFiManagerParameter wm_mqtt_password("input", "mqtt password", _default_str, sizeof(_default_str)+1, _customHtml_password, WFM_LABEL_BEFORE);

BoolParameter wm_pauseForOTA("pauseForOTA", "", _customHtml_hidden, WFM_LABEL_BEFORE);
IntParameter wm_white1Pin("white1Pin", "", _customHtml_hidden, WFM_LABEL_BEFORE);
IntParameter wm_white2Pin("white2Pin", "", _customHtml_hidden, WFM_LABEL_BEFORE);
IntParameter wm_redPin("redPin", "", _customHtml_hidden, WFM_LABEL_BEFORE);
IntParameter wm_greenPin("greenPin", "",  _customHtml_hidden, WFM_LABEL_BEFORE);
IntParameter wm_bluePin("bluePin", "",  _customHtml_hidden, WFM_LABEL_BEFORE);

WiFiManagerParameter wm_custom_html((char*)customConfig_html_nt);

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

enum BootMode {
    TURNOFF = 0, // 0 turn Off at boot, ignore MQTT
    TURNON = 1, // 1 turn on at boot, ignore MQTT
    LASTKNOWNNOMQTT = 2, // 2 last known setting at boot, ignore MQTT
    FOLLOWMQTT = 3 // 3 last known setting at boot, and follow MQTT commands at boot
};

///////////////////////////////////////////////////////////////////////////
//  LittleFS
///////////////////////////////////////////////////////////////////////////
bool loadConfigLittleFS(const char* filename, Properties& properties) {
    bool ret = false;

    if (LittleFS.begin()) {
        Serial.println("mounted file system");

        if (LittleFS.exists(filename)) {
            //file exists, reading and loading
            File configFile = LittleFS.open(filename, "r");

            if (configFile) {
                Serial.print(F("Loading config : "));
                Serial.println(filename);
                deserializeProperties<64>(configFile, properties);
                serializeProperties<64>(Serial, properties);
            }

            configFile.close();
        } else {
            Serial.print(F("File not found: "));
            Serial.println(filename);
        }

        // LittleFS.end();
    } else {
        Serial.print(F("Failed to begin LittleFS"));
    }

    return ret;
}


/**
 * Store custom oarameter configuration in LittleFS
 */
bool saveConfigLittleFS(const char* filename, Properties& properties) {
    bool ret = false;

    if (LittleFS.begin()) {
        LittleFS.remove(filename);
        File configFile = LittleFS.open(filename, "w");

        if (configFile) {
            Serial.print(F("Saving config : "));
            Serial.println(filename);
            serializeProperties<64>(configFile, properties);
                             serializeProperties<64>(Serial, properties);
            ret = true;
        } else {
            Serial.print(F("Failed to write file"));
            Serial.println(filename);
        }

        configFile.close();
        //    LittleFS.end();
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////
//  LittleFS
///////////////////////////////////////////////////////////////////////////
// Eeprom storage
// wait 500ms after last commit, then commit no more often than every 30s
Settings ledStatusSaveHandler(
    500,
    30000,
[]() {
    ledStatusConfig.put("hue", PV(ledStatusModel.hsb().hue()));
    ledStatusConfig.put("saturation", PV(ledStatusModel.hsb().saturation()));
    ledStatusConfig.put("brightness", PV(ledStatusModel.hsb().brightness()));
    ledStatusConfig.put("startupBrightness", PV(ledStatusModel.startupBrightness()));
    ledStatusConfig.put("white1", PV(ledStatusModel.hsb().white1()));
    ledStatusConfig.put("white2", PV(ledStatusModel.hsb().white2()));
    ledStatusConfig.put("power", PV(ledStatusModel.power()));
    saveConfigLittleFS(LEDSTATUS_FILENAME, ledStatusConfig);
    storeLedStatusConfigInLittleFS = false;
},
[]() {
    return storeLedStatusConfigInLittleFS;
}
);

// MQTT Storage
// mqtt updates as quickly as possible with a maximum frequence of MQTT_STATE_UPDATE_DELAY
std::unique_ptr<MQTTStore> mqttStore(nullptr);
Settings mqttSaveHandler(
    1,
    5000,
[]() {
    mqttStore->save(ledStatusModel);
},
[]() {

    if (lastStatusModel != ledStatusModel) {
        storeLedStatusConfigInLittleFS = true; // Must be done here because we also force update mqtt
        lastStatusModel = ledStatusModel;
        return true;
    }

    return false;
}
);

std::unique_ptr<StateMachine> bootSequence(nullptr);


///////////////////////////////////////////////////////////////////////////
//  Utilities
///////////////////////////////////////////////////////////////////////////


/**
 * Publish a message to mqtt
 */
void publishToMQTT(const char* topic, const char* payload) {
    if (mqttClient.publish(topic, payload, true)) {
        Serial.print("Published: ");
        Serial.print(topic);
        Serial.print(" ");
        Serial.println(payload);
    } else {
        Serial.println("Failed to publish to mqtt");
    }
}

void publishRelativeToBaseMQTT(const char* topic, const char* payload) {
    char buffer[65];
    const char* mqttBaseTopic = controllerConfig.get("mqttBaseTopic");
    strncpy(buffer, mqttBaseTopic, sizeof(buffer));
    strncat(buffer, "/", sizeof(buffer));
    strncat(buffer, topic, sizeof(buffer));
    publishToMQTT(buffer, payload);
}

void publishRelativeToBaseMQTT(const char* topic1, const char* topic2, const char* payload) {
    char buffer[65];
    const char* mqttBaseTopic = controllerConfig.get("mqttBaseTopic");
    strncpy(buffer, mqttBaseTopic, sizeof(buffer));
    strncat(buffer, "/", sizeof(buffer));
    strncat(buffer, topic1, sizeof(buffer));
    strncat(buffer, "/", sizeof(buffer));
    strncat(buffer, topic2, sizeof(buffer));
    publishToMQTT(buffer, payload);
}

/**
 * Turns off, effectifly setting brightness to 0
 */
HSB getOffState(const HSB& hsb) {
    return hsb.toBuilder().white1(0).white2(0).brightness(0).build();
}

/**
 * Get´s the ON state of a given HSB
 * When brightness is < STARTUP_MIN_BRIGHTNESS it will be turned on with the given brightness
 */
HSB getOnState(const HSB& hsb, float brightness) {
    if (hsb.brightness() < STARTUP_MIN_BRIGHTNESS) {
        return hsb.toBuilder()
               .brightness(brightness)
               .build();
    }

    return hsb;
}

/**
 * Send a command to the cmdHandler
 */
void handleCmd(const char* topic, const char* cmd) {
    cmdHandler->handle(topic, cmd, colorControllerService->hsb(), colorControllerService->currentHsb(), transitionCounter);
}

///////////////////////////////////////////////////////////////////////////
//  Webserver/WIFIManager
///////////////////////////////////////////////////////////////////////////
void serverOnlineCallback() {
    Serial.println("Server online");
    wm.server->on(JBONE_URI, []() {
        wm.server->sendHeader("Content-Encoding", "gzip");
        wm.server->setContentLength(jscript_js_gz_len);
        wm.server->send(200, "application/javascript", "");
        wm.server->sendContent_P((char*)jscript_js_gz, jscript_js_gz_len);
    });
}

void saveParamCallback() {
    Serial.println("[CALLBACK] saveParamCallback fired");

    if (std::strlen(wm_mqtt_server.getValue()) > 0) {
        controllerConfig.put("mqttServer", PV(wm_mqtt_server.getValue()));
        controllerConfig.put("mqttPort", PV((uint16_t)wm_mqtt_port.getValue()));
        controllerConfig.put("mqttUsername", PV(wm_mqtt_user.getValue()));
        controllerConfig.put("mqttPassword", PV(wm_mqtt_password.getValue()));
        controllerConfig.put("pauseForOTA", PV(wm_pauseForOTA.getValue()));

        controllerConfig.put("white1Pin", PV((int32_t)wm_white1Pin.getValue()));
        controllerConfig.put("white2Pin", PV((int32_t)wm_white2Pin.getValue()));
        controllerConfig.put("redPin", PV((int32_t)wm_redPin.getValue()));
        controllerConfig.put("greenPin", PV((int32_t)wm_greenPin.getValue()));
        controllerConfig.put("bluePin", PV((int32_t)wm_bluePin.getValue()));

        Serial.print(wm_pauseForOTA.getValue());
        controllerConfigModified = true;
        // Redirect from MQTT so on the next reconnect we pickup new values
        mqttClient.disconnect();
        // Send redirect back to param page
        wm.server->sendHeader(F("Location"), F("/param?"), true);
        wm.server->send(302, FPSTR(HTTP_HEAD_CT2), "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
        wm.server->client().stop();
    }
}

/**
 * Setup the wifimanager and configuration page
 */
void setupWifiManager() {
    wm_mqtt_port.setValue(controllerConfig.get("mqttPort"));
    wm_mqtt_password.setValue(controllerConfig.get("mqttPassword"), MQTT_PASSWORD_LENGTH);
    wm_mqtt_user.setValue(controllerConfig.get("mqttUsername"), MQTT_USERNAME_LENGTH);
    wm_mqtt_server.setValue(controllerConfig.get("mqttServer"), MQTT_SERVER_LENGTH);
    wm_pauseForOTA.setValue(controllerConfig.get("pauseForOTA"));

    wm_white1Pin.setValue(controllerConfig.get("white1Pin"));
    wm_white2Pin.setValue(controllerConfig.get("white2Pin"));
    wm_redPin.setValue(controllerConfig.get("redPin"));
    wm_greenPin.setValue(controllerConfig.get("greenPin"));
    wm_bluePin.setValue(controllerConfig.get("bluePin"));

    // Set extra setup page
    wm.setWebServerCallback(serverOnlineCallback);
    wm.addParameter(&wm_mqtt_server);
    wm.addParameter(&wm_mqtt_port);
    wm.addParameter(&wm_mqtt_user);
    wm.addParameter(&wm_mqtt_password);

    // Custom Config HTML
    wm.addParameter(&wm_custom_html);

    // Custom Config parameters
    wm.addParameter(&wm_pauseForOTA);
    wm.addParameter(&wm_redPin);
    wm.addParameter(&wm_greenPin);
    wm.addParameter(&wm_bluePin);
    wm.addParameter(&wm_white1Pin);
    wm.addParameter(&wm_white2Pin);
    
    wm.setCustomHeadElement("<script src='/jscript.js'></script>");

    // set country
    wm.setClass("invert");
    wm.setCountry("US"); // setting wifi country seems to improve OSX soft ap connectivity, may help others as well

    // Set configuration portal
    wm.setShowStaticFields(false);
    wm.setConfigPortalBlocking(false); // Must be blocking or else AP stays active
    wm.setDebugOutput(false);
    wm.setSaveParamsCallback(saveParamCallback);
    wm.setHostname(controllerConfig.get("mqttClientID"));
    std::vector<const char*> menu = {"wifi", "wifinoscan", "info", "param", "sep", "erase", "restart"};
    wm.setMenu(menu);

    wm.startWebPortal();
    wm.autoConnect(controllerConfig.get("mqttClientID"));
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    MDNS.begin(controllerConfig.get("mqttClientID"));
}

///////////////////////////////////////////////////////////////////////////
//  Startup/Defaults
///////////////////////////////////////////////////////////////////////////

void setupDefaults() {

    char chipHexBuffer[9];
    snprintf(chipHexBuffer, sizeof(chipHexBuffer), "%08X", ESP.getChipId());

    char mqttClientID[16];
    snprintf(mqttClientID, sizeof(mqttClientID), "RGBW_%s", chipHexBuffer);

    char mqttBaseTopic[16];
    snprintf(mqttBaseTopic, sizeof(mqttBaseTopic), "RGBW/%s", chipHexBuffer);

    char mqttLastWillTopic[64];
    snprintf(mqttLastWillTopic, sizeof(mqttLastWillTopic), "%s/%s", mqttBaseTopic, MQTT_LASTWILL_TOPIC);

    // Defaults for the controller
    controllerConfigModified |= controllerConfig.putNotContains("mqttClientID", PV(mqttClientID));
    controllerConfigModified |= controllerConfig.putNotContains("mqttBaseTopic", PV(mqttBaseTopic));
    controllerConfigModified |= controllerConfig.putNotContains("mqttServer", PV(""));
    controllerConfigModified |= controllerConfig.putNotContains("mqttUsername", PV(""));
    controllerConfigModified |= controllerConfig.putNotContains("mqttPassword", PV(""));
    controllerConfigModified |= controllerConfig.putNotContains("mqttPort", PV(1883));
    controllerConfigModified |= controllerConfig.putNotContains("mqttLastWillTopic", PV(mqttLastWillTopic));
    // 0 turn Off at boot, ignore MQTT
    // 1 turn on at boot, ignore MQTT
    // 2 last known setting at boot, ignore MQTT
    // 3 last known setting at boot, and follow MQTT commands at boot
    controllerConfigModified |= controllerConfig.putNotContains("bootMode", PV(2));
    controllerConfigModified |= controllerConfig.putNotContains("pauseForOTA", PV(true));

    // Adding pins
    controllerConfigModified |= controllerConfig.putNotContains("white1Pin", PV(WHITE1_PIN));
    controllerConfigModified |= controllerConfig.putNotContains("white2Pin", PV(WHITE2_PIN));
    controllerConfigModified |= controllerConfig.putNotContains("redPin", PV(RED_PIN));
    controllerConfigModified |= controllerConfig.putNotContains("greenPin", PV(GREEN_PIN));
    controllerConfigModified |= controllerConfig.putNotContains("bluePin", PV(BLUE_PIN));

    // Defaults for LED status
    ledStatusConfig.putNotContains("hue", PV(0.f));
    ledStatusConfig.putNotContains("saturation", PV(0.f));
    ledStatusConfig.putNotContains("brightness", PV(100.0f));
    // Startup Brightness
    ledStatusConfig.putNotContains("startupBrightness", PV(50.f));
    // Warm White
    ledStatusConfig.putNotContains("white1", PV(0.f));
    ledStatusConfig.putNotContains("white2", PV(0.f));
    // power status
    ledStatusConfig.putNotContains("power", PV(true));
}

void setupLedStatusModelAtBoot() {
    HSB hsb = HSB(
                  ledStatusConfig.get("hue"),
                  ledStatusConfig.get("saturation"),
                  ledStatusConfig.get("brightness"),
                  ledStatusConfig.get("white1"),
                  ledStatusConfig.get("white2")
              );

    bool power;
    float startupBrightness = ledStatusConfig.get("startupBrightness");

    const BootMode bootMode = static_cast<BootMode>((long)controllerConfig.get("bootMode"));

    switch (bootMode) {
        case TURNOFF: // Turn off at boot
            power = false;
            hsb = hsb.toBuilder().white1(0).white2(0).build();
            break;

        case TURNON: // Turn on at boot
            power = true;
            hsb = hsb.toBuilder().brightness(startupBrightness).build();
            break;

        case LASTKNOWNNOMQTT: // last known setting at boot, ignore MQTT
        case FOLLOWMQTT: // last known setting at boot, ignore MQTT
        default:
            power = ledStatusConfig.get("power");
    }

    ledStatusModel = StatusModel(hsb, startupBrightness, power);
    lastStatusModel = ledStatusModel;
}

/**
 * Start OTA
 */
void startOTA() {
    // Start OTA
    ArduinoOTA.setHostname(controllerConfig.get("mqttClientID"));
    ArduinoOTA.onStart([]() {
        Serial.println(F("OTA Beginning"));
        pwmLeds->setAll(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.print("ArduinoOTA Error[");
        Serial.print(error);
        Serial.print("]: ");

        if (error == OTA_AUTH_ERROR) {
            Serial.println(F("Auth Failed"));
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println(F("Begin Failed"));
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println(F("Connect Failed"));
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println(F("Receive Failed"));
        } else if (error == OTA_END_ERROR) {
            Serial.println(F("End Failed"));
        }
    });
    ArduinoOTA.begin();

    if (controllerConfig.get("pauseForOTA")) {
        uint16_t i = 0;

        do {
            delay(10);
            ArduinoOTA.handle();
            i++;
        } while (i < 500); // 5 seconds
    }
}

void setupMQTT() {
        // Setup mqtt
    mqttClient.setCallback([](char* p_topic, byte * p_payload, uint16_t p_length) {
        char mqttReceiveBuffer[64];

        if (p_length >= sizeof(mqttReceiveBuffer)) {
            Serial.print(F("MQTT Message to long."));
            return;
        }

        memcpy(mqttReceiveBuffer, p_payload, p_length);
        mqttReceiveBuffer[p_length] = 0;
        handleCmd(p_topic, mqttReceiveBuffer);
    });
}

void setupMQTTReconnectManager() {
    State* BOOTSEQUENCESTART;
    State* DELAYEDMQTTCONNECTION;
    State* TESTMQTTCONNECTION;
    State* CONNECTMQTT;
    State* PUBLISHONLINE;
    State* SUBSCRIBECOMMANDTOPIC;
    State* WAITFORCOMMANDCAPTURE;
    BOOTSEQUENCESTART = new State([]() {
        return 2;
    });
    DELAYEDMQTTCONNECTION = new StateTimed(1500, []() {
        return 2;
    });
    TESTMQTTCONNECTION = new State([]() {

        if (mqttClient.connected())  {
            if (WiFi.status() != WL_CONNECTED) {
                mqttClient.disconnect();
            }

            return 1;
        }

        return 3;
    });
    CONNECTMQTT = new State([]() {
        mqttClient.setServer(
            controllerConfig.get("mqttServer"),
            (int16_t)controllerConfig.get("mqttPort")
        );

        if (mqttClient.connect(
                controllerConfig.get("mqttClientID"),
                controllerConfig.get("mqttUsername"),
                controllerConfig.get("mqttPassword"),
                controllerConfig.get("mqttLastWillTopic"),
                0,
                true,
                MQTT_LASTWILL_OFFLINE)) {
            return 4;
        }

        Serial.println(F("ERROR: The connection to the MQTT broker failed"));
        return 1;
    });
    PUBLISHONLINE = new State([]() {
        publishToMQTT(
            controllerConfig.get("mqttLastWillTopic"),
            MQTT_LASTWILL_ONLINE);
        return 5;
    });
    SUBSCRIBECOMMANDTOPIC = new State([]() {
        char mqttSubscriberTopic[32];
        strncpy(mqttSubscriberTopic, controllerConfig.get("mqttBaseTopic"), sizeof(mqttSubscriberTopic));
        strncat(mqttSubscriberTopic, "/+", sizeof(mqttSubscriberTopic));

        if (mqttClient.subscribe(mqttSubscriberTopic, 0)) {
            Serial.print(F("Connected to topic : "));
            Serial.println(mqttSubscriberTopic);
            return 6;
        }

        Serial.print(F("ERROR: Failed to connect to topic : "));
        mqttClient.disconnect();
        return 1;
    });
    WAITFORCOMMANDCAPTURE = new StateTimed(3000, []() {
        startupFromBoot = false;
        // Always force save to update mqtt after re-connection
        mqttSaveHandler.save(true);
        return 2;
    });
    bootSequence.reset(new StateMachine({
        BOOTSEQUENCESTART, // 0
        DELAYEDMQTTCONNECTION,// 1
        TESTMQTTCONNECTION, // 2
        CONNECTMQTT, // 3
        PUBLISHONLINE, // 4
        SUBSCRIBECOMMANDTOPIC, // 5
        WAITFORCOMMANDCAPTURE // 6
    }));
}

void setupCommandHandler() {
    uint8_t subscriberTopicLength = strlen(controllerConfig.get("mqttBaseTopic"));
    cmdHandler.reset(new CmdHandler(
                         subscriberTopicLength,
    [](bool power) {
        const BootMode bootMode = static_cast<BootMode>((long)controllerConfig.get("bootMode"));

        if (!startupFromBoot || bootMode == FOLLOWMQTT) {
            colorControllerService->power(power);
            ledStatusModel.power(power);
        }
    },
    [](const HSB & hsb) {
        const BootMode bootMode = static_cast<BootMode>((long)controllerConfig.get("bootMode"));

        if (!startupFromBoot || bootMode == FOLLOWMQTT) {
            ledStatusModel.hsb(hsb);
            colorControllerService->hsb(hsb);
        }

    },
    [](std::unique_ptr<Filter> filter) {
        colorControllerService->filter(std::move(filter));
    },
    [](std::unique_ptr<Effect> effect) {
        colorControllerService->effect(std::move(effect));
    },
    [](const uint32_t base) {
        // Not implemented
    },
    []() {
        shouldRestart = millis();
    }
                     ));
}

///////////////////////////////////////////////////////////////////////////
//  SETUP() AND LOOP()
///////////////////////////////////////////////////////////////////////////


void setup() {

    // Enable serial port
    Serial.begin(115200);
    delay(50);

    // setup Strings
    loadConfigLittleFS(CONTROLLERCONFIG_FILENAME, controllerConfig);
    loadConfigLittleFS(LEDSTATUS_FILENAME, ledStatusConfig);
    setupDefaults();
    setupLedStatusModelAtBoot();

    setupCommandHandler();
    setupMQTT();
    setupWifiManager();
    setupMQTTReconnectManager();
    startOTA();

    // HSB to RGB conversion service
    hSBToRGB.reset(HsbToRGBGeneric::genericLedStrip());

    // Start the keds
    NewPwmLeds* leds = new NewPwmLeds(
        (long)controllerConfig.get("redPin"), 
        (long)controllerConfig.get("greenPin"), 
        (long)controllerConfig.get("bluePin"), 
        (long)controllerConfig.get("white1Pin"), 
        (long)controllerConfig.get("white2Pin")
    );
    leds->init();
    pwmLeds.reset(leds);

    // Enable colorController service (handles filters and effects)
    colorControllerService.reset(new ColorControllerService(ledStatusModel.hsb(), [](const HSB & currentHsb) {
        float colors[3];
        // currentHsb.constantRGB(colors);
        hSBToRGB->toRgb(currentHsb.hue(), currentHsb.saturation(), currentHsb.brightness(), colors);
        pwmLeds->setAll(colors[0], colors[1], colors[2], currentHsb.cwhite1(), currentHsb.cwhite2());
    }));

    // Enable the MQTT Store
    mqttStore.reset(new MQTTStore(
                        controllerConfig.get("mqttBaseTopic"),
                        "color/state",
                        "state/state",
                        mqttClient,
                        true
                    ));

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
            ledStatusSaveHandler.handle();
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            wm.process();
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            if (controllerConfigModified) {
                controllerConfigModified = false;
                saveConfigLittleFS(CONTROLLERCONFIG_FILENAME, controllerConfig);
            }
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            mqttSaveHandler.handle();
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            bootSequence->handle();
        } else if (shouldRestart != 0 && (currentMillis - shouldRestart >= 5000)) {
            shouldRestart = 0;
            ESP.restart();
        }

#if defined(ARILUX_DEBUG_TELNET)
        else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            // Handle Telnet connection for debugging
            handleTelnet();
        }

#endif
    }
}
