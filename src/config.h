#ifndef RED_PIN
#define RED_PIN 0
#endif

#ifndef GREEN_PIN
#define GREEN_PIN 0
#endif

#ifndef BLUE_PIN
#define BLUE_PIN 0
#endif

#ifndef WHITE1_PIN
#define WHITE1_PIN 0
#endif

#ifndef WHITE2_PIN
#define WHITE2_PIN 0
#endif

constexpr char CONTROLLERCONFIG_FILENAME[] = "controller.conf";
constexpr char LEDSTATUS_FILENAME[] = "ledstatus.conf";

constexpr char MQTT_STATUS[]=                           "status";
constexpr char  MQTT_LASTWILL_TOPIC[]=                    "lastwill";
constexpr char  MQTT_LASTWILL_ONLINE[]=                   "online";
constexpr char  MQTT_LASTWILL_OFFLINE[]=                  "offline";

// Brights at starup when the device was in a off state
// Bright ness to be considered to be the minimum stored in EEPROM
constexpr float STARTUP_MIN_BRIGHTNESS = 5.f;

// Don't change anything below here
#define ARRAY_SIZE(array) (sizeof((array))/sizeof((array[0])))

// Number of ms per effect transistion, 20ms == 50 Hz
#define FRAMES_PER_SECOND        50
#define EFFECT_PERIOD_CALLBACK   (1000 / FRAMES_PER_SECOND)


