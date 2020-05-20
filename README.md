| Master | Develop |
|-------|----------|
| [![Build Status](https://travis-ci.org/rvt/esp8266leds.svg?branch=master)](https://travis-ci.org/rvt/esp8266leds)  | [![Build Status](https://travis-ci.org/rvt/esp8266leds.svg?branch=develop)](https://travis-ci.org/rvt/esp8266leds) |


# LEDstrip firmware for esp8266/esp8265 devices

This is an driver for the popular LED strips using the ESP8266/EPS8265 devices.
Initially I tested them only on the Arilux devices but they can also work on any other ESP8266/ESP8265 device.

The initial code came from https://github.com/mertenats/Arilux_AL-LC0X but in the mean time it has been completely overhauled.

This firmware has been tested with OpenHAB and just a bit om Home Assistance.

## Features

- Fade from any color to any other color smoothly without apparent brightness changes without special commands
- ON/OFF states will correctly fade in/out and remember the last color (in EEPROM)
- Easy to make new effects, See effect.h and some of the including Effects
- You can send partial updates for the color, for example just can just send the hue, brightness or white values
- After startup the LED will always turn on as a safety feature (handy if the device is behind a switch, mqtt down, wifi down etc..)
- Solid reconnect to your MQTT broker.
- Uses Stefan Bruens PWM library for much finer grained controll, currently supporting 5000 levels of brightness per channel!!
- Remote control over the MQTT protocol via individual topics
- Supports transitions, flashing and other effects
- ~~Remote control with the included IR control (uncomment `#define IR_REMOTE` in `config.h`)~~ removed, I was not using it
- ~~Remote control with the included RF control (uncomment `#define RF_REMOTE` in `config.h`)~~ removed, I was not using it
- ~~TLS support (uncomment `#define TLS` in `setup.h` and change the fingerprint if not using CloudMQTT)~~ removed, I was not using it
- ArduinoOTA support for over-the-air firmware updates
- Native support for OpenHAB and should work with Home Assistant with MQTT
- Fading using cie1931 math
- Color correction using rainbow table for better color reproduction
- Setup pins from web

### Current effects

- Rainbow: Will keep fading over the rainbow of colors with a given time period
- Transition: Change from color1 to color2 over a period of time 
- Flash:  Flash between two colors or between black and the current color
- Strobe: Strobe between two colors, period can be given

### Configuration

Configuration is done using the web interface including LedPWM pins

## Updating

OTA is enabled on this firmware. Assuming the device is plugged in you should find the device using
```platformio device list --mdns --logical | grep arduino```
look for a device with a name like `RGBW_XXXXXXXX`

```bash
myMachine:Esp8266Leds rvt$ platformio device list --mdns --logical | grep arduino
RGBW_00CCAA42._arduino._tcp.local.
Type: _arduino._tcp.local.
ARILUX00879231._arduino._tcp.local.
Type: _arduino._tcp.local.
ARILUX00AD44B1._arduino._tcp.local.
Type: _arduino._tcp.local.
```

## Control

### MQTT

State and control is send and received through MQTT

## Filter vs Effect

### Effect

An effect will set the color, effects are usually based on timings like flashing leds, strobe or
slow fades. Only one effect can be active at a time.

### Filter

A filter will take the output of a effect and apply an additional transformation. THis can include
color correct and/or fading the HSB to create smooth transitions between colors. Only one filter
can be active at a time. NOTE: List of filter is on my todo..

### Boot Sequence

Bootmode controls how to start the device.
Setting for this in web interface comming soon

```
enum BootMode {
    TURNOFF = 0, // 0 turn Off at boot, ignore MQTT
    TURNON = 1, // 1 turn on at boot, ignore MQTT
    LASTKNOWNNOMQTT = 2, // 2 last known setting at boot, ignore MQTT
    FOLLOWMQTT = 3 // 3 last known setting at boot, and follow MQTT commands at boot
};
```

#### Bootorder

- Load default settings
- Get HSB values from LittleFS
- Subscribe state topic for two seconds
- Subscribe to command topic and overwrite any settings found in state over a period of two seconds

- Based on Bootmode we decide what we do with initial status

#### Control Codes and examples

1. We use individual topics to take advantage of retain and other features MQTT has to offer
   If you are have Home Assistant MQTT Discovery enabled, the `light.mqtt_json` platform will be loaded by Home Assistant instead of the `light.mqtt` platform.

   Topic: ``/color``

   | Name             | format                | Example                | Description                                                                               |
   |------------------|-----------------------|------------------------|-----------------------------------------------------------------------------------------|
   | `simple format`  | int,float,float       | 0,100,100              | Set Hue, Saturation and Brightness                                                        |
   | `hsb`            | hsb=int,float,float   | hsb=0,100,100          | Set Hue, Saturation and Brightnes with assignment                                         |
   | `hsb`            | hsb=int,float,float,float,float   | hsb=0,100,100,20,30          | Set Hue, Saturation and Brightness white1 and white 2 with assignment   |
   | `seperate`       | h=int s=float b=float w1=float w2=float | h=0 s=100 w1=25 w2=100 | Set as separate assignments  |
   | `combined`       | hsb=int,float,float b=float         | hsb=0,100,100 b=25     | Wil take brightness as 25  |

##### Example
```
mosquitto_pub -t "RGBW/001F162E/color" -m 'hsb=0,100,100,0,0' # Set Color to Red
mosquitto_pub -t "RGBW/001F162E/color" -m 'h=120' # Set Color to green with current brightness and saturation
mosquitto_pub -t "RGBW/001F162E/color" -m 'OFF' # Turn lights off (remember brightness)
mosquitto_pub -t "RGBW/001F162E/color" -m 'ON' # Turn lights on with last remembered brightness but ensures at least a minimum brightness is used(remember brightness)
mosquitto_pub -t "RGBW/001F162E/color" -m 'b=0.1 ON' # Turn lights with brightness to 0.1
```

## Available Filters

#### Disable Filtering

Topic: ``/filter`` name=``none``

Disable any filtering on the colors, color switch between current and new color.

##### Example

```
  mosquitto_pub -t "RGBW/001F162E/filter" -m 'name=none'
```

#### Fade filter

Topic: ``/filter`` name=``fading``

Will smoothly fade between colors when a new color is set.
This filter is implemented as fromValue + (toValue - fromValue) * m_alpha;

| Parameter  | type     | default  | Description          |
| ---------- | -------- | -------- | -------------------- |
| alpha      | float    | 0.f5     | Speed of fading, keep this between 0.f01 and 0.99   |

##### Example

```
mosquitto_pub -t "RGBW/001F162E/filter" -m 'name=fading alpha=0.1'
mosquitto_pub -t "RGBW/001F162E/filter" -m 'name=fading'
```

## Available effects 

All effects require the `name` parameter.
  
#### none

Topic: ``/effect`` name=``none``

Turn of any running effect

##### Example

```
mosquitto_pub -t "RGBW/001F162E/effect" -m 'name=none'
```

#### rainbow

Topic: ``/effect`` name=``rainbow``

Smoothly fades between all colors

| Parameter  | type     | default  | Description          |
| ---------- | -------- | -------- | -------------------- |
| duration   | long     | 1        | Total time in s it takes to do a full color palette   |

##### Example

```
mosquitto_pub -t "RGBW/001F162E/effect" -m 'name=rainbow' # Default 10 seconds
mosquitto_pub -t "RGBW/001F162E/effect" -m 'name=rainbow duration=300' # 5 Minute rotation
```

#### Flash

Topic: ``/effect`` name=``flash``
Flash or strobe between off/on or between two colors
Note: For some effects you might want to turn the filter off, specially for strobe style effects.

| Parameter  | type     | default  | Description          |
| ---------- | -------- | -------- | -------------------- |
| period     | int      | 50       | Total period measured in ticks. There are 50 ticks per second   |
| pulse      | int      | 25       | Width of the on/color pulse measured in ticks    |
| hsb        | hsb      |          | When a color is given we flash between this color and the current color insteadof off     |

##### Example

```
mosquitto_pub -t "RGBW/001F162E/effect" -m 'name=flash'  # 50% duty cycle, on/off
mosquitto_pub -t "RGBW/001F162E/effect" -m 'name=flash pulse=1 b=100 s=0' # 2% duty cycle strobe to white once a second

# two commands red short and blue longer 
mosquitto_pub -t "RGBW/001F162E/color" -m 'hsb=0,100,100,0,0'
mosquitto_pub -t "RGBW/001F162E/effect" -m 'name=flash period=100 pulse=10 hsb=240,100,100 period=25'
```

#### Fade

Topic: ``/effect`` name=``fade``
Gradually fade between two colors with a given duration

| Parameter  | type     | default  | Description          |
| ---------- | -------- | -------- | -------------------- |
| duration   | long     |          | Total time in ms the fade will take   |

##### Example

```
mosquitto_pub -t "RGBW/001F162E/effect" -m 'name=fade duration=15000 hsb=240,100,60' # Fade from current color to blue over 15 seconds
```

### Other things you can do

#### Restart the device from mqtt

Topic: ``/restart``

Restart the device (handy for development)

##### Example

```
mosquitto_pub -t "RGBW/001F162E/restart" -m '1'
```

#### Set base address of the remote control

Topic: ``/remote``

Set the base address of the remote control, value will be stored in EEPROM. Currently only tested with RF.
You can also pair the remote control by pressing in the first few seconds a key on the remote control.
The value will be stored in EEPROM and in mqtt.

##### Example

```
mosquitto_pub -t "RGBW/001F162E/remote" -m '10622464'
```

#### Force storage of settings

Topic: ``/store``

See also EEPROM_COMMIT_WAIT_DELAY

##### Example

```
mosquitto_pub -t "RGBW/001F162E/store" -m '1'
```

#### Last Will and Testament

The firmware will publish a [MQTT Last Will and Testament] at `rgb(w/ww)/<chipid>/status`.
When the device successfully connects it will publish `online` to that topic and when it disconnects `offline` will automatically be published.

#### Home Assistance Discovery

Removed, I don-t use HAS and as far as I know I am the only one using my firmware


#### Configuration with homebridge

Use the following plugin https://github.com/rvt/homebridge-esp8266leds

or use : ´´´npm install -g homebridge-esp8266leds´´´
Once added to homebridge you can add accessories with the following configuration

```javascript
{
  "accessory": "esp8266leds",
  "name": "Bed Light",
  "url": "http://localhost:1883",
  "username": "<USERNAME>",
  "password": "<PASSWORD>",
  "caption": "Bed Light",
  "baseTopic": "RGBW/00AD4715"
}
```

At this moment only state (ON/OFF) and HSB can be controller with homebridge

#### Configuration with OpenHAB 2

You can use this device to connect to OpenHAB 2 with MQTT configured. Make sure MQTT is configured and working
and both OpenHAB and your light are connecting to the same MQTT broker.

The below config will also allow you to say to Siri when Homekit is configured on OpenHAB: 

- ``Set Arilux to Red``
- ``Set briightness of Arilux to fifty percent``
- ``Turn off Arilux``
- etc...

File: ``items/default.items``

```
Color  Item_Arilux_Color "Arilux" <light> ["Lighting"] {mqtt="<[mosquitto:RGB(W|WW)/<chipid>/color/state:state:JS(ariluxhsbToHsb.js)],>[mosquitto:RGB(W|WW)/<chipid>/color:command:*:default]"}
```

This will receive updates and send them to the correct mqtt topic.

A transformation to turns the device created payload to something OpenHAB can understand.
Essentially take out the HSB value from the color state and return that to OpenHAB.
File: ``transform/ariluxhsbToHsb.js``

```
(function(i){
    if (i.indexOf('OFF') !== -1) return 'OFF';
    regex = /([,.\d]+)/;
    m=null;
    if ((m = regex.exec(i)) !== null) {
        return m[0].split(',').slice(0,3).join(',');
    }
    return i;
})(input);
```

Adding a color picker to the default sitemap.
File: ``sitemaps/default.sitemap``

```
    Frame label="Arilux" {
        Colorpicker item=Item_Arilux_Color label="Color" icon="colorwheel"
    }
```

Look at the directory openhab for more instructions and ready to use scripts and configurations.

## Licence

MIT License

Copyright (c) 2016 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

You Must credit the author and link to this repository if you implement any of his work.

## Contributors

- [@mertenats]: Initial creator of the project, documention and code

*If you like the content of this repo, please add a star! Thank you!*

[@StefanBruens]: https://github.com/StefanBruens/ESP8266_new_pwm
[@KmanOz]: https://github.com/KmanOz
[@DanGunvald]: https://github.com/DanGunvald
[@robbiet480]: https://github.com/robbiet480
[MQTT Last Will and Testament]: http://www.hivemq.com/blog/mqtt-essentials-part-9-last-will-and-testament
[esp8266]: https://en.wikipedia.org/wiki/ESP8266
[Home Assistant's MQTT discovery functionality]: https://home-assistant.io/docs/mqtt/discovery/
[MQTT]: http://mqtt.org/
[Alternative firmware]: https://github.com/mertenats/Arilux_AL-LC0X 
