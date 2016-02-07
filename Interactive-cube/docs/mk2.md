# Mini interactive cube

The initial cube was made using led panels on each sides, but it lacks one important piece, input, and only having orientation/acceleration is not optimal, this cube addresses that, and is slightly different, it has sixteen buttons on each of the six sides and they are all usable through OSC.






## Enclosure



### Cube sides and pixel ordering

Looking at the cube from the natural angle, from top (side #4) left (side #3) front (side #0):

<img src="https://github.com/interactiveBrick/Interactive-Cube/raw/master/Interactive-cube/docs/cube1.png" width="300" height="300" alt="Top/Left/Front" />

Looking at the cube from the back (side #2) right (side #1):

<img src="https://github.com/interactiveBrick/Interactive-Cube/raw/master/Interactive-cube/docs/cube2.png" width="300" height="300" alt="Back/Right" />

Looking at the cube from the bottom (side #5) left (side #3):

<img src="https://github.com/interactiveBrick/Interactive-Cube/raw/master/Interactive-cube/docs/cube3.png" width="300" height="300" alt="Bottom/Left" />



### Example

So if you want to turn on pixel #11 on the left side (side #3) you would adress the pixel by calculating `side * 16 + pixel`, so 3 * 16 + 11, so we want to enable pixel 59

In the OSC message `/cubeNNNN/leds` that would be the fourth bit (value 8) of the 7th byte: `/cubeNNNN/leds 0 0 0 0 0 0 8 0 0 0 0 0`

Front, pixel 0 to 15.
Right, pixel 16 to 31.
Back, pixel 32 to 47.
Left, pixel 48 to 63.
Top, pixel 64 to 79.
Bottom, pixel 80 to 96.





## OSC Messages

The device sends and receives OSC messages, these are the different ones.

All OSC messages share the prefix `/cubename/` which is a 8 character string that can be configured, by default it's `cubeNNNN` where `NNNN` is a part of the wlan mac address.



### Key press

`/cubeNNNN/btn btn down`

Namespace: `/cubeNNNN/btn`

Arguments:

* `btn` int32 - Which button
* `down` int32 - 1 = Button was pressed, 0 = Button was released


### Gyro data

`/cubeNNNN/rot rx ry rz`

Namespace: `/cubeNNNN/rot`

Arguments:

* `rx` int32 - Relative rotation around X.
* `ry` int32 - Relative rotation around Y.
* `rz` int32 - Relative rotation around Z.


### Accelerometer data

`/cubeNNNN/acc ax ay az`

Namespace: `/cubeNNNN/acc`

Arguments:

* `ax` int32 - Relative acceleration X.
* `ay` int32 - Relative acceleration Y.
* `az` int32 - Relative acceleration Z.



### Update a single pixel

`/cubeNNNN/leds led on`

Namespace: `/cubeNNNN/leds`

Arguments:

* `led` int32 - Pixel to change
* `on` int32 - 1 = Lit, 0 = Not lit 



### Update one side

`/cubeNNNN/leds side n0 n1`

Namespace: `/cubeNNNN/leds`

Arguments:

* `side` int32 - Which side to update. (0 - 5)
* `n0`, `n1` int32 - Two 8bit values, one bit for each led.



### Update entire display

`/cubeNNNN/leds ft0 ft1 rt0 rt1 bk0 bk1 lt0 lt1 tp0 tp1 bt0 bt1`

Namespace: `/cubeNNNN/leds`

Arguments:

* `ft0`, `ft1` int32 - Front side leds, 8 bits each.
* `rt0`, `rt1` int32 - Right side leds, 8 bits each.
* `bk0`, `bk1` int32 - Back side leds, 8 bits each.
* `lt0`, `lt1` int32 - Left side leds, 8 bits each.
* `tp0`, `tp1` int32 - Top side leds, 8 bits each.
* `bt0`, `bt1` int32 - Bottom side leds, 8 bits each.



## Configuration

When the device boots up, it will blink the first pixel a couple of times, if you hold it down at that time, the firmware will go into configuration mode, from there it will set up a wireless network named `Cube Setup - NNNN`, connect to that network and open `http://192.168.0.1` to configure your device.




## Components

* 1x NodeMCU ESP8266 Devkit
* 6x Adafruit Trellis + Elastomer Pads for buttons on each side
* 1x ADXL345 I2C Accelerometer (Sparkfun)
* 1x L3GD20H I2C Gyroscope (Adafruit)
* 1x USB Powerpack




### Laser template

If you want to cut a simple enclosure, there's one included.

<img src="https://github.com/interactiveBrick/Interactive-Cube/raw/master/Interactive-cube/docs/enclosure.png" width="600" alt="Enclosure" />




