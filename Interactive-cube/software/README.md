Interactive Cube Software
=========================

"Your mileage may vary..."



On Raspberry PI
---------------

* Get and build one of these [Adafruit RGB Matrix Hat](https://www.adafruit.com/products/2345)

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


