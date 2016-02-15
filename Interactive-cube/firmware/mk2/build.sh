#!/bin/sh











cp user/* nodemcu-firmware/app/user/

rm nodemcu-firmware/bin/*bin

docker run -e INTEGER_ONLY=1 -e NODE_DEBUG=1 -e BUILD_WOFS=0 --rm -ti -v "/Users/possan/Projects/Interactive-Brick/Interactive-cube/firmware/mk2/nodemcu-firmware":"/opt/nodemcu-firmware" marcelstoer/nodemcu-build

ls -la nodemcu-firmware/bin/*bin

