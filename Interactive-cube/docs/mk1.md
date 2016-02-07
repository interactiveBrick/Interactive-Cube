DIY Interactive Cube
====================

"Your mileage may vary..."



Prerequisites
-------------

* Get and build one of these [Adafruit RGB Matrix Hat](https://www.adafruit.com/products/2345)

* Laser cut some 6mm acrylic using the template in `hardware` folder

* Order a few 32x32 HUB75 led panels with 4mm pixel pitch from Alibaba

* Get some M3 screws and nuts from your favorite hardware shop

* Get a raspberry pi with raspbian on.

* Get a powerful power supply.



On Raspberry PI
---------------


* Set a static IP of 192.168.1.150

* Install websocket rgb panel server

    ```
    cd software/server
    make test-server
    ```

* Run server

    ```
    make runserver
    ```



On your computer
----------------

* Set a static IP to 192.168.1.100

* Turn on Riot

* Connect touch screen midi thingy

* Open a few terminals

    * Start the OSC proxy

        ```
        cd software/oscproxy
        npm install
        node osc-proxy.js
        ```

    * Start the Pd patch, make sure midi and OSC works, turn on OSC transmitter

        ```
        cd Pd-Patches
        /Applications/Pd-extended.app/Contents/MacOS/Pd-extended main2.pd
        ```

    * Start web server

        ```
        cd software/browser
        python -m SimpleHTTPServer
        open http://localhost:8000/drumcube.html
        ```


