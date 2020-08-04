#!/bin/bash

set -e 



#List mDNS devices
#platformio device list --mdns --logical | grep arduino
./generateHtmlArray.sh

# Pleades/wall 
#platformio run --target upload --upload-port RGBW_001F162E.local -e LC10_OLD

# Polaris 
# platformio run --target upload --upload-port RGBW_00AD44B1.local -e LC01_RGB

# front spotlight
#platformio run --target upload --upload-port RGBW_00E0DC9A.local -e H801

# Backyard wall
#platformio run --target upload --upload-port RGBW_00E0D28A.local -e H801


#./generateHtmlArray.sh 
#platformio run

#platformio run --target upload -e wemos
#platformio device monitor --baud 115200

platformio run --target upload -e H801
#platformio run --target upload --upload-port 192.168.178.154 -e H801
#platformio device monitor --baud 115200

# connect serial IO
#platformio device monitor

#platformio test -e native

