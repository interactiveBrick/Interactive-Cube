#!/bin/sh

# FN=`ls nodemcu-firmware/bin/nodemcu*bin`
# FN=`ls nodemcu-firmware/bin/0x00000*bin`
# /Users/possan/Library/Arduino15/packages/esp8266/tools/esptool/0.4.5/esptool -vv -cd ck -cb 115200 -cp /dev/cu.SLAB_USBtoUART -ca 0x00000 -cf $FN
# FN=`ls nodemcu-firmware/bin/0x10000*bin`
# /Users/possan/Library/Arduino15/packages/esp8266/tools/esptool/0.4.5/esptool -vv -cd ck -cb 115200 -cp /dev/cu.SLAB_USBtoUART -ca 0x10000 -cf $FN

# nodemcu-firmware/tools/esptool.py --port /dev/cu.SLAB_USBtoUART write_flash 0x00000 nodemcu-firmware/bin/0x00000.bin 0x10000 nodemcu-firmware/bin/0x10000.bin

nodemcu-firmware/tools/esptool.py --port /dev/cu.SLAB_USBtoUART write_flash --flash_freq 80m --flash_size 4m --flash_mode dio 0x00000 nodemcu-firmware/bin/0x00000.bin 0x10000 nodemcu-firmware/bin/0x10000.bin

